#!/usr/bin/env python3
"""
AMKN8639 OpenBLT XCP Host Tool v3
Transport: TX=[len][XCP_payload], RX=raw XCP (no length prefix)
Flow: CONNECT → SET_MTA → PROGRAM_CLEAR → PROGRAM(data)×N → PROGRAM(size=0) → PROGRAM_RESET
"""

import serial, time, sys, os

BL_START = 0x08020000
PROG_MAX  = 62  # CTO=64, minus PID+size

PID_RES = 0xFF
PID_ERR = 0xFE

CMD_CONNECT       = 0xFF
CMD_SET_MTA       = 0xF6
CMD_PROGRAM       = 0xD0
CMD_PROGRAM_CLEAR = 0xD1
CMD_PROGRAM_RESET = 0xCF

class XcpHost:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud, timeout=3.0)
        print(f"[INFO] {port} @ {baud}")

    def close(self): self.ser.close()

    def send(self, payload):
        frame = bytes([len(payload)]) + payload
        print(f"  TX: {frame.hex(' ')}")
        self.ser.write(frame)
        self.ser.flush()

    def recv(self, timeout=3.0):
        """Receive raw XCP response (no length prefix)"""
        t0 = time.time()
        buf = b''
        while (time.time() - t0) < timeout:
            b = self.ser.read(1)
            if not b:
                if buf: break  # Got something, stop
                continue
            buf += b
            # If we got PID_RES or PID_ERR, read remaining based on pattern
            if buf[0] == PID_RES:
                # CONNECT response is 8 bytes; others are shorter
                # Read more bytes with short timeout
                more = self.ser.read(32)
                buf += more
                break
            elif buf[0] == PID_ERR:
                # Error response is 2 bytes
                more = self.ser.read(1)
                buf += more
                break
        if buf:
            print(f"  RX: {buf.hex(' ')}")
        return buf

    def check_ok(self, resp):
        return len(resp) > 0 and resp[0] == PID_RES

    def xcp_connect(self):
        print("\n=== CONNECT ===")
        self.send(bytes([CMD_CONNECT]))
        r = self.recv()
        if not self.check_ok(r):
            print("[FAIL]")
            return False
        res = r[1] if len(r) > 1 else 0
        cto = r[3] if len(r) > 3 else 0
        print(f"  Resources: 0x{res:02X} PGM={'YES' if res&0x10 else 'NO'} CTO={cto}")
        print("[OK]")
        return True

    def xcp_set_mta(self, addr):
        payload = bytes([CMD_SET_MTA, 0,0,0,0,
                        addr&0xFF, (addr>>8)&0xFF, (addr>>16)&0xFF, (addr>>24)&0xFF])
        self.send(payload)
        return self.check_ok(self.recv())

    def xcp_program_clear(self, addr, length):
        print(f"\n=== ERASE 0x{addr:08X}+0x{length:X} ===")
        payload = bytes([CMD_PROGRAM_CLEAR, 0,0,
                        length&0xFF, (length>>8)&0xFF,
                        (length>>16)&0xFF, (length>>24)&0xFF])
        self.send(payload)
        r = self.recv(timeout=15.0)
        ok = self.check_ok(r)
        print("[OK]" if ok else "[FAIL]")
        return ok

    def xcp_program(self, data_chunk):
        size = len(data_chunk)
        payload = bytes([CMD_PROGRAM, size]) + data_chunk
        self.send(payload)
        return self.check_ok(self.recv(timeout=5.0))

    def xcp_program_finalize(self):
        print("\n=== FINALIZE (PROGRAM size=0) ===")
        self.send(bytes([CMD_PROGRAM, 0]))
        r = self.recv(timeout=10.0)
        ok = self.check_ok(r)
        print("[OK]" if ok else "[FAIL]")
        return ok

    def xcp_program_reset(self):
        print("\n=== RESET ===")
        self.send(bytes([CMD_PROGRAM_RESET]))
        time.sleep(0.3)
        r = self.recv(timeout=2.0)
        print("[OK] Rebooting...")
        return True

    def flash(self, bin_path, base=BL_START):
        print(f"\n{'='*60}")
        print(f"  XCP Flasher  |  {os.path.basename(bin_path)}")
        print(f"  Base: 0x{base:08X}")
        print(f"{'='*60}")

        with open(bin_path, 'rb') as f: fw = f.read()
        print(f"  Size: {len(fw)}B ({len(fw)/1024:.1f}KB)")
        while len(fw) % 4: fw += b'\xFF'

        if not self.xcp_connect(): return False
        if not self.xcp_set_mta(base): return False

        SECTOR = 0x20000
        elen = ((len(fw) + SECTOR - 1) // SECTOR) * SECTOR
        if not self.xcp_program_clear(base, elen): return False

        total = len(fw)
        written = 0
        cs = PROG_MAX
        last_pct = -1

        print(f"\n=== WRITE {total}B ({cs}B chunks) ===")
        for off in range(0, total, cs):
            chunk = fw[off:off+cs]
            if not self.xcp_set_mta(base + off): return False
            if not self.xcp_program(chunk):
                print(f"[FAIL] @ 0x{base+off:08X}")
                return False
            written += len(chunk)
            pct = written * 100 // total
            if pct != last_pct:
                print(f"  {written}/{total} ({pct}%)")
                last_pct = pct

        if not self.xcp_program_finalize(): return False
        print(f"\n[OK] {written}B programmed")
        self.xcp_program_reset()
        return True

def main():
    if len(sys.argv) < 3:
        print("Usage: xcp_flash.py <COM> <firmware.bin> [addr_hex]")
        sys.exit(1)
    h = XcpHost(sys.argv[1])
    try:
        addr = int(sys.argv[3], 0) if len(sys.argv) > 3 else BL_START
        if h.flash(sys.argv[2], addr):
            print("\n[DONE] Success!")
        else:
            print("\n[FAIL]")
            sys.exit(1)
    finally:
        h.close()

if __name__ == '__main__':
    main()
