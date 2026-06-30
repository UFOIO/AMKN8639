#!/usr/bin/env python3
"""
RK3568 OTA Agent - Remote firmware upgrade for AMKN8639 STM32H743
=================================================================
Runs on RK3568 (Linux), communicates with STM32 via UART.

Flow:
  1. Download firmware from server (HTTP/HTTPS) or use local file
  2. Send AT+OTA to STM32 APP → STM32 reboots into BL XCP mode
  3. Send XCP-lite commands: Erase → Write → Boot
  4. Verify new firmware is running

Usage:
  python3 rk_ota_agent.py --uart /dev/ttyS1 --fw http://server/firmware.bin
  python3 rk_ota_agent.py --uart /dev/ttyS1 --fw local.bin
"""

import serial
import struct
import time
import sys
import os
import argparse
import hashlib
import urllib.request
import tempfile

# ============================================================
# XCP-lite Protocol (same as BL V6.0)
# ============================================================
FLASH_BASE  = 0x08000000
APP_BASE    = 0x08020000
SECTOR_SIZE = 128 * 1024

def xsum(data):
    c = 0
    for b in data: c ^= b
    return c & 0xFF

def make_packet(cmd, data=b''):
    payload = bytes([cmd]) + data
    return bytes([0xAA, len(payload)+1]) + payload + bytes([xsum(payload)])

def parse_ack(raw):
    if not raw or len(raw) < 4 or raw[0] != 0xAA:
        return None
    if raw[3] != xsum(raw[1:3]):
        return None
    return raw[2]  # status byte

# ============================================================
# RK3568 OTA Agent
# ============================================================
class OTA_Agent:
    def __init__(self, uart_dev, baud=115200):
        self.uart = uart_dev

    def _open_uart(self):
        self.ser = serial.Serial(self.uart, 115200, timeout=0.2)
        self.ser.reset_input_buffer()

    def _close_uart(self):
        if hasattr(self, 'ser') and self.ser.is_open:
            self.ser.close()

    def _send_cmd(self, cmd, data=b'', wait_ack=True, timeout=5):
        pkt = make_packet(cmd, data)
        self.ser.write(pkt)
        self.ser.flush()
        if not wait_ack:
            return 0
        return self._wait_ack(timeout)

    def _wait_ack(self, timeout=5):
        deadline = time.time() + timeout
        while time.time() < deadline:
            b = self.ser.read(1)
            if not b:
                time.sleep(0.02)
                continue
            if b[0] == 0xAA:
                rest = self.ser.read(3)
                if len(rest) >= 3:
                    s = parse_ack(b + rest)
                    if s is not None:
                        return s
        return None

    def _wait_for(self, keyword, timeout=15):
        """Wait for keyword in serial output"""
        buf = b''
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.ser.in_waiting:
                c = self.ser.read(1)
                buf += c
                if keyword.encode() in buf:
                    return True
            else:
                time.sleep(0.05)
        return False

    # ============================================================
    # Step 1: Trigger STM32 into BL mode
    # ============================================================
    def enter_bl_mode(self):
        """Send AT+OTA to APP, APP triggers BKP reset into BL XCP mode"""
        print("[1/4] Triggering OTA mode...")
        self._open_uart()

        # Flush any pending data
        self.ser.read(self.ser.in_waiting or 1)
        time.sleep(0.1)

        # Send AT+OTA command (APP's AT handler should call ota_request_update())
        self.ser.write(b'AT+OTA\r\n')
        self.ser.flush()
        time.sleep(0.5)

        # Read response (APP should ack before reset)
        resp = self.ser.read(self.ser.in_waiting or 1)
        print(f"  APP response: {resp.decode('ascii', errors='replace').strip()[:100]}")

        # Close and wait for STM32 to reset
        self._close_uart()
        print("  Waiting for STM32 reset (3s)...")
        time.sleep(3)

        # Reopen, wait for BL
        self._open_uart()
        self.ser.reset_input_buffer()

        if not self._wait_for('BL V6', timeout=12):
            print("  BL not detected! Trying XCP connect anyway...")
            # Maybe BL is already waiting
            self.ser.write(b'\xAA')
            self.ser.flush()
            time.sleep(0.5)

        # Read BL init
        extra = self.ser.read(self.ser.in_waiting or 1)
        print(f"  BL: {extra.decode('ascii', errors='replace').strip()[:150]}")
        return True

    # ============================================================
    # Step 2 & 3: Erase + Write firmware
    # ============================================================
    def flash_firmware(self, fw_data):
        """Flash firmware to APP area"""
        original_size = len(fw_data)
        while len(fw_data) % 4:
            fw_data += b'\xFF'

        words = struct.unpack(f'<{len(fw_data)//4}I', fw_data)
        total = len(words)

        # Calculate sectors
        start_sec = (APP_BASE - FLASH_BASE) // SECTOR_SIZE
        end_sec = ((APP_BASE + len(fw_data) - 1 - FLASH_BASE) // SECTOR_SIZE)

        # --- Erase ---
        print(f'[2/4] Erasing sectors {start_sec}..{end_sec}...')
        for s in range(start_sec, end_sec + 1):
            addr = FLASH_BASE + s * SECTOR_SIZE
            status = self._send_cmd(0x01, struct.pack('<I', addr), timeout=10)
            if status != 0:
                print(f'  Erase sector {s} @ 0x{addr:08X} FAIL (status={status})')
                return False
            print(f'  Sector {s} erased')

        # --- Write ---
        print(f'[3/4] Writing {total} words ({original_size} bytes)...')
        errors = 0
        for i, w in enumerate(words):
            addr = APP_BASE + i * 4
            status = self._send_cmd(0x02, struct.pack('<II', addr, w))
            if status != 0:
                errors += 1
                if errors > 10:
                    print(f'\n  Too many errors, aborting!')
                    return False

            if i % 1024 == 0 or i == total - 1:
                pct = (i + 1) * 100 // total
                print(f'\r  [{pct:3d}%] {i+1}/{total}', end='', flush=True)

        print(f'\n  Write complete. Errors: {errors}')
        return errors == 0

    # ============================================================
    # Step 4: Boot new firmware
    # ============================================================
    def boot_app(self):
        print('[4/4] Booting new firmware...')
        status = self._send_cmd(0x03, timeout=3)
        if status == 0:
            print('  Boot command sent!')
            time.sleep(2)
            resp = self.ser.read(min(self.ser.in_waiting, 500) or 1)
            txt = resp.decode('ascii', errors='replace')
            # Check for APP startup signature
            if 'AT+INFO' in txt or 'UART1 Init' in txt:
                print(f'  APP started OK!')
                return True
            print(f'  APP output: {txt[:200]}')
            return 'AT+INFO' in txt
        print(f'  Boot command FAIL (status={status})')
        return False

    # ============================================================
    # Top-level: do full OTA update
    # ============================================================
    def update(self, fw_data):
        if not self.enter_bl_mode():
            return False
        if not self.flash_firmware(fw_data):
            return False
        if not self.boot_app():
            return False
        return True


# ============================================================
# Firmware source helpers
# ============================================================
def load_firmware(source):
    """Load firmware from URL or local file. Returns (data, sha256)."""
    if source.startswith('http://') or source.startswith('https://'):
        print(f'Downloading from {source}...')
        req = urllib.request.urlopen(source)
        data = req.read()
        print(f'  Downloaded {len(data)} bytes')
    else:
        print(f'Loading from {source}...')
        with open(source, 'rb') as f:
            data = f.read()
        print(f'  Loaded {len(data)} bytes')

    sha = hashlib.sha256(data).hexdigest()
    print(f'  SHA256: {sha[:16]}...')
    return data


# ============================================================
# CLI
# ============================================================
def main():
    parser = argparse.ArgumentParser(description='RK3568 OTA Agent for AMKN8639')
    parser.add_argument('--uart', required=True, help='UART device (e.g., /dev/ttyS1, COM46)')
    parser.add_argument('--fw', required=True, help='Firmware source (URL or .bin file)')
    parser.add_argument('--baud', type=int, default=115200, help='UART baud rate')
    args = parser.parse_args()

    # Load firmware
    fw = load_firmware(args.fw)

    if len(fw) > (2 * 1024 * 1024 - 128 * 1024):
        print(f'ERROR: Firmware too large ({len(fw)} bytes)')
        sys.exit(1)

    # Run OTA
    agent = OTA_Agent(args.uart, args.baud)
    try:
        success = agent.update(fw)
        if success:
            print('\n========== OTA UPDATE SUCCESS ==========')
        else:
            print('\n========== OTA UPDATE FAILED ==========')
            sys.exit(1)
    finally:
        agent._close_uart()


if __name__ == '__main__':
    main()
