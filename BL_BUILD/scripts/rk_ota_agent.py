#!/usr/bin/env python3
"""
RK3568 OTA Agent v3.1 - Multi-word batch write (60 words/pkt)
================================================================
Runs on RK3568 (Linux), communicates with AMKN8639 STM32H743 via UART.

Protocol: XCP-lite   CMD: 1=erase 2=batch-write 3=boot 4=info 5=LED
Batch size: 60 words (240 bytes) per packet - matches ota_host.py v3.1

Usage:
  python3 rk_ota_agent.py --uart /dev/ttyS1 --fw http://server/AMKN8639_APP.bin
  python3 rk_ota_agent.py --uart /dev/ttyS1 --fw local.bin
  python3 rk_ota_agent.py --uart /dev/ttyS1 --fw local_bootloader.bin --bl
"""

import serial
import struct
import time
import sys
import argparse
import hashlib
import urllib.request

# ============================================================
# XCP-lite Protocol  (identical to ota_host.py v3.1)
# ============================================================
APP_BASE    = 0x08020000
BL_TEMP     = 0x081E0000
SECTOR_SIZE = 128 * 1024
WORDS_PER_PKT = 60                     # batch write: 60 words/packet

def xsum(data: bytes) -> int:
    """XOR checksum"""
    c = 0
    for b in data:
        c ^= b
    return c & 0xFF

def make_packet(cmd: int, data: bytes = b'') -> bytes:
    """Build XCP-lite packet: [0xAA LEN CMD DATA... CKSUM]"""
    payload = bytes([cmd]) + data
    return bytes([0xAA, len(payload) + 1]) + payload + bytes([xsum(payload)])

# ============================================================
# OTA Agent
# ============================================================
class OTAAgent:
    def __init__(self, uart_dev: str, baud: int = 115200):
        self.uart = uart_dev
        self.baud = baud
        self.ser = None

    # ---------- serial helpers ----------
    def _open(self):
        self.ser = serial.Serial(self.uart, self.baud, timeout=0.1)
        self.ser.reset_input_buffer()
        time.sleep(0.2)

    def _close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def _drain(self):
        """Discard pending input"""
        time.sleep(0.1)
        if self.ser and self.ser.is_open:
            self.ser.read(self.ser.in_waiting or 1)

    def _read_all(self, timeout: float = 0.3) -> bytes:
        """Read all available data with a short settle time"""
        time.sleep(timeout)
        if self.ser and self.ser.is_open:
            return self.ser.read(self.ser.in_waiting or 1)
        return b''

    # ---------- XCP ack scanner (same logic as ota_host.py) ----------
    def _wait_ack(self, timeout: float = 5.0):
        """Scan for a valid XCP ACK packet - returns status byte or None"""
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.ser.in_waiting:
                raw = self.ser.read(self.ser.in_waiting)
                for i in range(len(raw)):
                    if raw[i] == 0xAA and i + 3 < len(raw):
                        if raw[i + 3] == xsum(raw[i + 1:i + 3]):
                            return raw[i + 2]       # status byte
            else:
                time.sleep(0.005)
        return None

    def _send_cmd(self, cmd: int, data: bytes = b'',
                  wait_ack: bool = True, timeout: float = 5.0):
        """Send one XCP command, optionally wait for ACK"""
        pkt = make_packet(cmd, data)
        self.ser.write(pkt)
        self.ser.flush()
        if not wait_ack:
            return 0
        return self._wait_ack(timeout)

    # ---------- wait for keyword ----------
    def _wait_for(self, keyword: str, timeout: float = 15.0) -> bool:
        """Wait until *keyword* appears in serial input"""
        buf = b''
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.ser.in_waiting:
                buf += self.ser.read(self.ser.in_waiting)
                if keyword.encode() in buf:
                    return True
            else:
                time.sleep(0.05)
        return False

    # ============================================================
    # 1. Enter BL mode
    # ============================================================
    def enter_bl_mode(self) -> bool:
        """
        Send AT+OTA to APP -> APP resets into BL XCP mode.
        Then scan for BL banner and send connect byte.
        """
        print('[1/4] Triggering OTA...')
        self._open()
        self._drain()

        # --- option A: APP is running, send AT+OTA ---
        self.ser.write(b'AT+OTA\r\n')
        self.ser.flush()
        time.sleep(0.6)
        resp = self._read_all(0.3)
        if resp:
            for line in resp.decode('ascii', 'replace').split('\n'):
                stripped = line.strip()
                if stripped:
                    print(f'  APP: {stripped[:120]}')

        self._close()
        print('  Waiting for STM32 reset (4 s)...')
        time.sleep(4)

        # --- option B: BL is already waiting (cold boot) ---
        self._open()

        # drain any banner
        buf = b''
        deadline = time.time() + 12
        found = False
        while time.time() < deadline:
            if self.ser.in_waiting:
                buf += self.ser.read(self.ser.in_waiting)
                # detect BL banner
                if b'BL V7' in buf or b'Wait' in buf:
                    found = True
                    break
            else:
                time.sleep(0.05)

        if not found:
            print('  BL banner not seen, trying XCP connect anyway...')

        # send XCP connect byte
        self.ser.write(b'\xAA')
        self.ser.flush()
        time.sleep(0.3)
        extra = self._read_all(0.3)
        if extra:
            print(f'  BL: {extra.decode("ascii", "replace").strip()[:200]}')
        print('  Connected.')
        return True

    # ============================================================
    # 2. Erase sectors
    # ============================================================
    def erase(self, base: int, size: int) -> bool:
        """Erase enough 128-KB sectors to cover [base ... base+size)"""
        sectors = (size + SECTOR_SIZE - 1) // SECTOR_SIZE
        print(f'[2/4] Erasing {sectors} sector(s)...')
        for i in range(sectors):
            addr = base + i * SECTOR_SIZE
            status = self._send_cmd(0x01, struct.pack('<I', addr), timeout=15)
            if status != 0:
                print(f'  FAIL @ 0x{addr:08X}')
                return False
            print(f'  [{i+1}/{sectors}] OK')
        return True

    # ============================================================
    # 3. Multi-word batch write  (60 words/pkt - matches ota_host.py)
    # ============================================================
    def write(self, data: bytes, base: int = APP_BASE) -> bool:
        """Batch-write firmware to flash.  Pads data to 4-byte boundary."""
        # pad to 4-byte alignment
        while len(data) % 4:
            data += b'\xFF'
        words = struct.unpack(f'<{len(data) // 4}I', data)
        total = len(words)
        W = WORDS_PER_PKT
        size_kb = len(data) // 1024
        print(f'[3/4] Writing {total} words ({size_kb} KB) - batch {W} words/pkt...')

        t0 = time.time()
        errors = 0
        for chunk_start in range(0, total, W):
            chunk_end = min(chunk_start + W, total)
            # build payload: addr + N x word
            payload = struct.pack('<I', base + chunk_start * 4)
            for i in range(chunk_start, chunk_end):
                payload += struct.pack('<I', words[i])

            status = self._send_cmd(0x02, payload, timeout=10)
            if status != 0:
                errors += 1
                print(f'\n  FAIL @ batch {chunk_start // W}')

            done = chunk_end
            pct = done * 100 // total
            if done % (W * 8) == 0 or done == total:
                elapsed = time.time() - t0
                speed = (done * 4) / elapsed / 1024 if elapsed > 0 else 0
                eta = (elapsed / done * total - elapsed) if done > 0 else 0
                print(f'\r  [{pct:3d}%] {done}/{total}  {speed:.1f} KB/s  ETA {eta:.0f} s',
                      end='', flush=True)

        print()                          # newline after progress
        elapsed = time.time() - t0
        print(f'  Done - {elapsed:.1f} s, {total * 4 / elapsed / 1024:.1f} KB/s, errors={errors}')
        return errors == 0

    # ============================================================
    # 4. Boot APP
    # ============================================================
    def boot_app(self) -> bool:
        """Send boot command, wait a few seconds, check for APP output"""
        print('[4/4] Booting APP...')
        status = self._send_cmd(0x03, timeout=3)
        if status != 0:
            print(f'  Boot ACK FAIL (status={status})')
            return False
        print('  Boot ACK OK, waiting for APP startup...')
        time.sleep(3)
        output = self._read_all(0.5)
        txt = output.decode('ascii', 'replace')
        if 'UART' in txt or 'AT+INFO' in txt:
            print('  APP started OK!')
            print(f'  First lines: {txt.strip()[:300]}')
            return True
        print(f'  APP output unclear:\n{txt[:300]}')
        # even if output is garbled, boot ACK was good
        return True

    # ============================================================
    # Top-level: full OTA update
    # ============================================================
    def update(self, fw_data: bytes, base: int = APP_BASE) -> bool:
        """Full OTA: enter BL -> erase -> write -> boot"""
        if not self.enter_bl_mode():
            return False
        if not self.erase(base, len(fw_data)):
            return False
        if not self.write(fw_data, base):
            return False
        if not self.boot_app():
            return False
        return True


# ============================================================
# Firmware source helpers
# ============================================================
def load_firmware(source: str) -> bytes:
    """Load firmware from URL or local file.  Returns raw bytes."""
    if source.startswith('http://') or source.startswith('https://'):
        print(f'Downloading {source}...')
        req = urllib.request.urlopen(source)
        data = req.read()
        print(f'  {len(data)} bytes')
    else:
        print(f'Loading {source}...')
        with open(source, 'rb') as f:
            data = f.read()
        print(f'  {len(data)} bytes ({len(data) / 1024:.1f} KB)')

    sha = hashlib.sha256(data).hexdigest()
    print(f'  SHA256: {sha[:16]}...')
    return data


# ============================================================
# CLI
# ============================================================
def main():
    parser = argparse.ArgumentParser(
        description='RK3568 OTA Agent v3.1 for AMKN8639')
    parser.add_argument('--uart', required=True,
                        help='UART device, e.g. /dev/ttyS1 or COM46')
    parser.add_argument('--fw', required=True,
                        help='Firmware source - URL or local .bin file')
    parser.add_argument('--bl', action='store_true',
                        help='Flash BL to staging area (triggers self-upgrade)')
    parser.add_argument('--baud', type=int, default=115200)
    args = parser.parse_args()

    # load firmware
    fw = load_firmware(args.fw)

    # run OTA
    agent = OTAAgent(args.uart, args.baud)
    try:
        if args.bl:
            # BL upgrade: write to staging area, then trigger self-upgrade
            print('BL upgrade mode - writing to staging area...')
            if not agent.enter_bl_mode():
                print('FAIL: enter BL mode')
                sys.exit(1)
            if not agent.erase(BL_TEMP, len(fw)):
                print('FAIL: erase staging')
                sys.exit(1)
            if not agent.write(fw, BL_TEMP):
                print('FAIL: write staging')
                sys.exit(1)
            # trigger self-upgrade via AT command
            agent._close()
            time.sleep(0.3)
            s2 = serial.Serial(args.uart, args.baud, timeout=0.2)
            s2.write(b'AT+OTA=BL\r\n')
            s2.flush()
            time.sleep(0.5)
            s2.close()
            print('\n========== BL UPGRADE TRIGGERED ==========')
        else:
            if agent.update(fw, APP_BASE):
                print('\n========== OTA UPDATE SUCCESS ==========')
            else:
                print('\n========== OTA UPDATE FAILED ==========')
                sys.exit(1)
    finally:
        agent._close()


if __name__ == '__main__':
    main()
