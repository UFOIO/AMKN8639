#!/usr/bin/env python3
"""
ota_server.py — AMKN8639 OTA Firmware Upload & Management Tool

Features:
  1. HTTP file server for firmware distribution
  2. Firmware signing (via imgtool)
  3. RK3568 OTA trigger (via TCP socket to RK3568 custom protocol)
  4. Progress monitoring
  5. Batch firmware management

Architecture:
  PC (this tool) ──HTTP──> MCU (LWIP client)    ← firmware download
  PC (this tool) ──TCP──> RK3568                ← OTA trigger commands
  RK3568         ──UART──> MCU                  ← AT+OTA commands

Usage:
  # Start HTTP server only
  python ota_server.py serve --dir ./firmware --port 8080

  # Sign + upload firmware to RK3568
  python ota_server.py upload \
      --key amkn8639_root.pem \
      --bin AMKN8639_APP.bin \
      --version 1.5.0 \
      --host 192.168.1.100 \
      --rk3568 192.168.1.99:9000

  # Full pipeline: sign → serve → trigger
  python ota_server.py deploy \
      --key amkn8639_root.pem \
      --bin AMKN8639_APP.bin \
      --version 1.5.0 \
      --rk3568 192.168.1.99:9000
"""
import argparse
import http.server
import socketserver
import subprocess
import threading
import json
import socket
import time
import sys
import os
from pathlib import Path

# ─── Configuration ──────────────────────────────────────────
HTTP_PORT = 8080
RK3568_PORT = 9000

# ─── Global State ────────────────────────────────────────────
g_server = None
g_server_thread = None
g_ota_active = False


# ══════════════════════════════════════════════════════════════
# HTTP Firmware Server
# ══════════════════════════════════════════════════════════════

class FirmwareHandler(http.server.SimpleHTTPRequestHandler):
    """Serves firmware files with CORS headers for MCU HTTP client."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=kwargs.pop("directory"), **kwargs)

    def log_message(self, format, *args):
        print(f"  [HTTP] {self.client_address[0]} - {format % args}")

    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        super().end_headers()


def start_http_server(directory: str, port: int):
    """Start HTTP server in background thread."""
    global g_server, g_server_thread

    directory = os.path.abspath(directory)
    os.chdir(directory)

    handler = lambda *args: FirmwareHandler(*args, directory=directory)
    g_server = socketserver.TCPServer(("0.0.0.0", port), handler)
    g_server_thread = threading.Thread(target=g_server.serve_forever, daemon=True)
    g_server_thread.start()

    print(f"\n  HTTP Server: http://0.0.0.0:{port}")
    print(f"  Serving:      {directory}")
    print(f"  Files:")
    for f in sorted(os.listdir(directory)):
        if f.endswith(".bin"):
            sz = os.path.getsize(os.path.join(directory, f))
            print(f"    http://<ip>:{port}/{f}  ({sz:,} bytes)")


def stop_http_server():
    global g_server
    if g_server:
        g_server.shutdown()
        g_server.server_close()
        g_server = None


# ══════════════════════════════════════════════════════════════
# RK3568 Communication (custom TCP protocol)
# ══════════════════════════════════════════════════════════════

class RK3568Client:
    """TCP client for communicating with RK3568 custom protocol."""

    def __init__(self, host: str, port: int, timeout: float = 30.0):
        self.host = host
        self.port = port
        self.timeout = timeout
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(self.timeout)
        self.sock.connect((self.host, self.port))
        print(f"  Connected to RK3568: {self.host}:{self.port}")

    def disconnect(self):
        if self.sock:
            self.sock.close()
            self.sock = None

    def send_cmd(self, cmd: str) -> str:
        """Send command, return response."""
        print(f"  -> {cmd}")
        self.sock.sendall((cmd + "\r\n").encode())
        resp = b""
        while True:
            try:
                chunk = self.sock.recv(1024)
                if not chunk:
                    break
                resp += chunk
                if b"\n" in resp:
                    break
            except socket.timeout:
                break
        result = resp.decode("utf-8", errors="replace").strip()
        print(f"  <- {result}")
        return result

    def trigger_ota(self, url: str, size: int, sha256: str = ""):
        """Send OTA start command to RK3568."""
        cmd = f"AT+OTA=START,{url},{size},{sha256}"
        return self.send_cmd(cmd)

    def query_status(self) -> str:
        return self.send_cmd("AT+OTA=STATUS?")

    def commit(self) -> str:
        return self.send_cmd("AT+OTA=COMMIT")

    def abort_ota(self) -> str:
        return self.send_cmd("AT+OTA=ABORT")


# ══════════════════════════════════════════════════════════════
# Commands
# ══════════════════════════════════════════════════════════════

def cmd_serve(args):
    """Start HTTP firmware server."""
    print("═" * 50)
    print("  AMKN8639 OTA Firmware Server")
    print("═" * 50)

    start_http_server(args.dir, args.port)

    try:
        print("\n  Press Ctrl+C to stop...\n")
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n  Shutting down...")
        stop_http_server()
        print("  Done.")


def cmd_upload(args):
    """Sign firmware (if needed) and upload via RK3568."""
    print("═" * 50)
    print("  AMKN8639 OTA Upload")
    print("═" * 50)

    # Step 1: Sign firmware if not already signed
    input_bin = args.bin
    signed_bin = input_bin

    if not args.skip_sign:
        print(f"\n[1/4] Signing firmware...")
        signed_bin = input_bin.replace(".bin", "_SIGNED.bin")
        cmd = [
            sys.executable, "-m", "imgtool.main", "sign",
            "--key", args.key,
            "--header-size", "0x200",
            "--align", "8",
            "--version", args.version,
            "--slot-size", str(0x180000),
            "--load-addr", "0x08020000",
            "--pad",
            input_bin, signed_bin
        ]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"  ERROR: {result.stderr}")
            sys.exit(1)
        print(f"  Signed: {signed_bin} ({os.path.getsize(signed_bin):,} bytes)")
    else:
        print(f"\n[1/4] Using pre-signed firmware: {signed_bin}")

    # Step 2: Start HTTP server
    print(f"\n[2/4] Starting HTTP server...")
    fw_dir = os.path.dirname(os.path.abspath(signed_bin)) or "."
    fw_name = os.path.basename(signed_bin)
    start_http_server(fw_dir, args.http_port)

    # Give server time to start
    time.sleep(1)

    # Step 3: Build URL and trigger OTA via RK3568
    print(f"\n[3/4] Triggering OTA via RK3568...")
    fw_size = os.path.getsize(signed_bin)
    fw_url = f"http://{args.host}:{args.http_port}/{fw_name}"

    print(f"  Firmware URL:  {fw_url}")
    print(f"  Firmware size: {fw_size:,} bytes")

    rk = RK3568Client(args.rk3568_ip, args.rk3568_port)
    try:
        rk.connect()

        resp = rk.trigger_ota(fw_url, fw_size)
        if "ERR" in resp:
            print(f"  OTA start failed: {resp}")
            rk.disconnect()
            stop_http_server()
            sys.exit(1)

        # Step 4: Monitor progress
        print(f"\n[4/4] Monitoring progress...")
        print(f"  {'─' * 40}")
        last_pct = -1
        while True:
            time.sleep(2)
            status = rk.query_status()
            print(f"  {status}")

            # Parse progress
            if "READY" in status:
                print(f"\n  Firmware verified, ready to commit!")
                if not args.no_commit:
                    print(f"  Sending COMMIT...")
                    rk.commit()
                    print(f"  MCU will reboot into new firmware.")
                else:
                    print(f"  Auto-commit disabled. Send AT+OTA=COMMIT manually.")
                break

            if "DOWNLOADING" in status:
                try:
                    pct = int(status.split(",")[-1])
                    if pct != last_pct:
                        bar = "█" * (pct // 5) + "░" * (20 - pct // 5)
                        print(f"  [{bar}] {pct}%", end="\r")
                        last_pct = pct
                except:
                    pass

            if "ERROR" in status:
                print(f"\n  OTA failed: {status}")
                break

        print(f"  {'─' * 40}")
    finally:
        rk.disconnect()

    print(f"\n  Keep HTTP server running? Press Ctrl+C to stop...")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        stop_http_server()
        print("  Done.")


def cmd_deploy(args):
    """Full pipeline: sign → serve → trigger in one command."""
    # Reuse upload logic
    args.skip_sign = False
    args.no_commit = args.no_commit
    cmd_upload(args)


def cmd_status(args):
    """Query OTA status from RK3568."""
    rk = RK3568Client(args.rk3568_ip, args.rk3568_port)
    try:
        rk.connect()
        status = rk.query_status()
        print(f"  OTA Status: {status}")

        # Also query version
        version = rk.send_cmd("AT+OTA=VERSION?")
        print(f"  Version:    {version}")
    finally:
        rk.disconnect()


def cmd_rollback(args):
    """Trigger rollback via RK3568."""
    if not args.force:
        confirm = input("  This will rollback to factory firmware. Continue? [y/N] ")
        if confirm.lower() != "y":
            print("  Cancelled.")
            return

    rk = RK3568Client(args.rk3568_ip, args.rk3568_port)
    try:
        rk.connect()
        resp = rk.send_cmd("AT+OTA=ROLLBACK")
        print(f"  {resp}")
    finally:
        rk.disconnect()


# ══════════════════════════════════════════════════════════════
# Main
# ══════════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(
        description="AMKN8639 OTA Firmware Management Tool"
    )
    sub = parser.add_subparsers(dest="command", help="Commands")

    # serve
    p = sub.add_parser("serve", help="Start HTTP firmware server")
    p.add_argument("--dir", default="./firmware", help="Firmware directory")
    p.add_argument("--port", type=int, default=8080, help="HTTP port")

    # upload
    p = sub.add_parser("upload", help="Sign + upload firmware")
    p.add_argument("--key", default="amkn8639_root.pem", help="Signing key")
    p.add_argument("--bin", required=True, help="Firmware .bin file")
    p.add_argument("--version", required=True, help="Firmware version (e.g. 1.5.0)")
    p.add_argument("--host", default="192.168.1.100", help="This PC's IP for HTTP")
    p.add_argument("--http-port", type=int, default=8080, help="HTTP server port")
    p.add_argument("--rk3568-ip", default="192.168.1.99", help="RK3568 IP")
    p.add_argument("--rk3568-port", type=int, default=9000, help="RK3568 TCP port")
    p.add_argument("--skip-sign", action="store_true", help="Skip signing step")
    p.add_argument("--no-commit", action="store_true", help="Don't auto-commit after verify")

    # deploy (alias for upload)
    p = sub.add_parser("deploy", help="Full deploy pipeline (same as upload)")
    p.add_argument("--key", default="amkn8639_root.pem", help="Signing key")
    p.add_argument("--bin", required=True, help="Firmware .bin file")
    p.add_argument("--version", required=True, help="Firmware version (e.g. 1.5.0)")
    p.add_argument("--host", default="192.168.1.100", help="This PC's IP")
    p.add_argument("--http-port", type=int, default=8080, help="HTTP server port")
    p.add_argument("--rk3568-ip", default="192.168.1.99", help="RK3568 IP")
    p.add_argument("--rk3568-port", type=int, default=9000, help="RK3568 TCP port")
    p.add_argument("--no-commit", action="store_true", help="Don't auto-commit")

    # status
    p = sub.add_parser("status", help="Query OTA status")
    p.add_argument("--rk3568-ip", default="192.168.1.99", help="RK3568 IP")
    p.add_argument("--rk3568-port", type=int, default=9000, help="RK3568 TCP port")

    # rollback
    p = sub.add_parser("rollback", help="Rollback to factory firmware")
    p.add_argument("--rk3568-ip", default="192.168.1.99", help="RK3568 IP")
    p.add_argument("--rk3568-port", type=int, default=9000, help="RK3568 TCP port")
    p.add_argument("--force", "-f", action="store_true", help="Skip confirmation")

    args = parser.parse_args()

    cmds = {
        "serve": cmd_serve,
        "upload": cmd_upload,
        "deploy": cmd_deploy,
        "status": cmd_status,
        "rollback": cmd_rollback,
    }

    if args.command in cmds:
        cmds[args.command](args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
