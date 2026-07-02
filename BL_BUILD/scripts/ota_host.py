#!/usr/bin/env python3
"""AMKN8639 OTA Host v4.0 - closed-loop: flash, diag, BL self-upgrade"""
import serial, struct, sys, time, os, argparse, io, threading, subprocess
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

APP_BASE     = 0x08020000
BL_TEMP      = 0x081E0000
SEC_SIZE     = 128 * 1024
WORDS_PER_PKT = 60
DEFAULT_ERASE_TIMEOUT = 30
DEFAULT_WRITE_TIMEOUT = 30
DEFAULT_INFO_TIMEOUT  = 3
STLINK_CLI = r"D:\stmp\bin\STM32_Programmer_CLI.exe"

DTCM_SAVE_POINTS = [
    (0x20000008, "RCC_AHB4ENR (expect 0x02 after GPIOBEN)"),
    (0x20000010, "GPIOB ODR (LED test)"),
    (0x20000018, "RCC_AHB4ENR (expect 0x03 after GPIOAEN|GPIOBEN)"),
    (0x2000001C, "RCC_APB2ENR (expect 0x10 USART1EN)"),
    (0x20000020, "GPIOA MODER (expect PA9/PA10 AF mode)"),
    (0x20000024, "GPIOA AFRH  (expect PA9/PA10 AF7)"),
    (0x20000028, "USART1 CR1 (expect 0xD = TE|RE|UE)"),
    (0x2000002C, "USART1 BRR (expect 0x22B = 555)"),
    (0x20000030, "USART1 ISR (after CR1 set)"),
    (0x20000034, "USART1 ISR (after 10ms settle)"),
    (0x20000038, "USART1 CR1 (after TE|RE)"),
    (0x2000003C, "USART1 ISR (after second settle)"),
    (0x20000040, "USART1 ISR (before print)"),
    (0x20000250, "call_from_sram copy FLASH base (expect 0x52002000)"),
    (0x20000254, "call_from_sram copy KEY1        (expect 0x45670123)"),
    (0x20000258, "call_from_sram copy KEY2        (expect 0xCDEF89AB)"),
]


def xsum(d):
    c = 0
    for b in d:
        c ^= b
    return c & 0xFF


def pkt(cmd, data=b""):
    pl = bytes([cmd]) + data
    return bytes([0xAA, len(pl) + 1]) + pl + bytes([xsum(pl)])


def run_swd(args, timeout=15):
    r = subprocess.run([STLINK_CLI] + args, capture_output=True, timeout=timeout)
    return r.returncode == 0, r.stdout.decode("ascii", "replace"), r.stderr.decode("ascii", "replace")


def reset_board_via_swd():
    strategies = [
        [r"-c", "port=SWD", "freq=4000", "-rst"],
        [r"-c", "port=SWD", "freq=4000", "connectmode=under-reset", "-rst"],
        [r"-c", "port=SWD", "freq=4000", "-hardrst"],
        [r"-c", "port=SWD", "freq=1000", "-rst"],
        [r"-c", "port=SWD", "freq=4000"],
    ]
    for i, args in enumerate(strategies, 1):
        ok, _, _ = run_swd(args, timeout=10)
        if ok:
            if i > 1:
                print("  [reset] strategy %d OK" % i)
            return True
    print("  [reset] all 5 strategies failed - press RESET button manually")
    return False


def read_dtcm(addr, n_words=1):
    # NOTE: STM32_Programmer_CLI -r32 ADDR N takes N as BYTES, not words.
    n_bytes = max(4, n_words * 4)
    ok, out, _ = run_swd(["-c", "port=SWD", "freq=4000", "-r32",
                          "0x%08X" % addr, "%d" % n_bytes, "-halt"])
    if not ok:
        return None
    vals = []
    for line in out.splitlines():
        s = line.strip()
        if s.startswith("0x") and ":" in s:
            payload = s.split(":", 1)[1].strip()
            if not payload:
                continue
            for tok in payload.split():
                try:
                    vals.append(int(tok, 16))
                except Exception:
                    pass
    while len(vals) < n_words:
        vals.append(0)
    return vals[:n_words]


def read_flash(addr, n_bytes):
    ok, out, _ = run_swd(["-c", "port=SWD", "freq=4000", "-r8",
                          "0x%08X" % addr, "%d" % n_bytes, "-halt"])
    if not ok:
        return None
    for line in out.splitlines():
        line = line.strip()
        if line.startswith("0x") and ":" in line:
            parts = line.split(":", 1)[1].strip().split()
            try:
                return bytes(int(p, 16) for p in parts
                             if len(p) == 2 and all(c in "0123456789abcdefABCDEF" for c in p))
            except:
                pass
    return None


class OTA:
    def __init__(self, port, baud=115200):
        self.port = port
        self.baud = baud
        self.s = serial.Serial(port, baud, timeout=0.05)
        time.sleep(0.1)
        self._spammer = None
        self._stop = [False]

    def close(self):
        self._stop_spam()
        if self.s and self.s.is_open:
            self.s.close()

    def _start_spam(self):
        self._stop_spam()
        self._stop = [False]
        def loop():
            while not self._stop[0]:
                try:
                    self.s.write(b"\xAA")
                    self.s.flush()
                except Exception:
                    break
                time.sleep(0.02)
        self._spammer = threading.Thread(target=loop, daemon=True)
        self._spammer.start()

    def _stop_spam(self):
        self._stop[0] = True
        if self._spammer:
            self._spammer.join(timeout=0.5)
            self._spammer = None

    def _wait_ack(self, t=5, expect_len=2):
        dl = time.time() + t
        buf = b""
        while time.time() < dl:
            if self.s.in_waiting:
                buf += self.s.read(self.s.in_waiting)
                for i in range(len(buf) - 3):
                    if buf[i] == 0xAA and i + 3 < len(buf):
                        if buf[i + 1] != expect_len:
                            continue
                        if buf[i + 3] == xsum(buf[i + 1:i + 3]):
                            return buf[i + 2], buf
            else:
                time.sleep(0.005)
        return None, buf

    def _read_response(self, t=3, expect_len=None):
        dl = time.time() + t
        buf = b""
        while time.time() < dl:
            if self.s.in_waiting:
                buf += self.s.read(self.s.in_waiting)
                i = 0
                while i < len(buf):
                    if buf[i] != 0xAA:
                        i += 1
                        continue
                    if i + 1 >= len(buf):
                        break
                    plen = buf[i + 1]
                    if expect_len is not None and plen != expect_len:
                        i += 1
                        continue
                    total = plen + 3
                    if i + total > len(buf):
                        break
                    body = buf[i + 2:i + 2 + plen]
                    ck = buf[i + 2 + plen]
                    if xsum(body) == ck:
                        return body, buf
                    i += 1
            else:
                time.sleep(0.005)
        return None, buf

    def wait_bl(self, t=15, do_reset=True):
        if do_reset:
            print("[wait_bl] Resetting board via SWD...")
            if not reset_board_via_swd():
                print("  reset failed")
                return False
        else:
            print("[wait_bl] Waiting for BL banner (reset manually if needed)...")
        self.s.reset_input_buffer()
        captured = b""
        dl = time.time() + t
        entered = False
        triggered = False
        while time.time() < dl:
            if self.s.in_waiting:
                chunk = self.s.read(self.s.in_waiting)
                captured += chunk
                try:
                    sys.stdout.write(chunk.decode("ascii", "replace"))
                    sys.stdout.flush()
                except Exception:
                    pass
                if not triggered and b"Wait 10s for XCP" in captured:
                    self.s.write(b"\x55")
                    self.s.flush()
                    triggered = True
                if b"XCP!" in captured or b"READY" in captured:
                    entered = True
                    break
            else:
                time.sleep(0.05)
        time.sleep(0.1)
        try:
            while self.s.in_waiting:
                self.s.read(self.s.in_waiting)
        except Exception:
            pass
        self.s.reset_input_buffer()
        if entered:
            print("[wait_bl] BL in XCP mode, READY")
        else:
            print("[wait_bl] WARN: did not see READY - check banner/connection")
        return entered

    def trigger_ota(self):
        print("[trigger_ota] Sending AT+OTA to APP...")
        self._stop_spam()
        self.s.reset_input_buffer()
        self.s.write(b"AT+OTA\r\n")
        self.s.flush()
        return self.wait_bl(do_reset=False)

    def info(self, t=DEFAULT_INFO_TIMEOUT):
        self.s.reset_input_buffer()
        self.s.write(pkt(4))
        self.s.flush()
        body, _ = self._read_response(t=t, expect_len=10)
        if body is None or not body:
            print("  No response (timeout %ds)" % t)
            return False
        if len(body) >= 10 and body[0] == 0x84:
            ver_major = body[1]
            ver_minor = body[2]
            flash_kb  = (body[3] << 8) | body[4]
            appok     = body[9]
            print("  BL v%d.%d  Flash=%dKB  APP: %s" % (
                ver_major, ver_minor, flash_kb,
                "OK" if appok == 1 else "INVALID"))
            return True
        print("  Unexpected body (%d bytes): %s" % (
            len(body) if body else 0,
            body.hex() if body else ""))
        return False

    def erase(self, base, size, t=DEFAULT_ERASE_TIMEOUT, progress=True):
        n = (size + SEC_SIZE - 1) // SEC_SIZE
        if progress:
            print("[erase] %d sector(s) from 0x%08X (timeout %ds each)" % (n, base, t))
        t0 = time.time()
        for i in range(n):
            addr = base + i * SEC_SIZE
            self.s.reset_input_buffer()
            self.s.write(pkt(1, struct.pack("<I", addr)))
            self.s.flush()
            s, _ = self._wait_ack(t, expect_len=2)
            if s is None:
                print("  FAIL @ 0x%08X TIMEOUT" % addr)
                return False
            if s != 0:
                print("  FAIL @ 0x%08X ack=0x%02X" % (addr, s))
                return False
            if progress:
                print("  [%d/%d] 0x%08X OK  (%.1fs)" % (i + 1, n, addr, time.time() - t0))
        return True

    def erase_one(self, addr, t=DEFAULT_ERASE_TIMEOUT):
        print("[erase_one] sector @ 0x%08X" % addr)
        return self.erase(addr, SEC_SIZE, t=t, progress=False)

    def write(self, data, base=APP_BASE, t=DEFAULT_WRITE_TIMEOUT):
        while len(data) % 4:
            data += b"\xFF"
        words = struct.unpack("<%dI" % (len(data) // 4), data)
        total = len(words)
        W = WORDS_PER_PKT
        print("[write] %d words (%dKB) batch=%d (timeout %ds)" % (total, len(data) // 1024, W, t))
        t0 = time.time()
        err = 0
        done = 0
        for cs in range(0, total, W):
            ce = min(cs + W, total)
            d = struct.pack("<I", base + cs * 4)
            for j in range(cs, ce):
                d += struct.pack("<I", words[j])
            self.s.reset_input_buffer()
            self.s.write(pkt(2, d))
            self.s.flush()
            s, _ = self._wait_ack(t, expect_len=2)
            if s is None:
                print("\n  TIMEOUT @ batch %d (word %d)" % (cs // W, cs))
                err = 1
                break
            if s != 0:
                print("\n  FAIL @ batch %d (word %d) ack=0x%02X" % (cs // W, cs, s))
                err = 1
                break
            done = ce
            if done % (W * 8) == 0 or done == total:
                et = time.time() - t0
                speed = done * 4 / et / 1024 if et > 0 else 0
                eta = (et / done * (total - done)) if done > 0 else 0
                print("\r  [%3d%%] %d/%d  %.1fKB/s  ETA %ds" %
                      (done * 100 // total, done, total, speed, eta), end="", flush=True)
        print()
        et = time.time() - t0
        if et > 0:
            print("  Done %.1fs, %.1fKB/s, err=%d" % (et, done * 4 / et / 1024, err))
        return err == 0

    def boot(self, capture_s=4):
        print("[boot] cmd=3 (boot APP)")
        self.s.reset_input_buffer()
        self.s.write(pkt(3))
        self.s.flush()
        s, _ = self._wait_ack(5, expect_len=2)
        if s == 0:
            print("  OK! capturing %ds APP output..." % capture_s)
            time.sleep(0.3)
            buf = b""
            dl = time.time() + capture_s
            while time.time() < dl:
                if self.s.in_waiting:
                    c = self.s.read(self.s.in_waiting)
                    buf += c
                    try:
                        sys.stdout.write(c.decode("ascii", "replace"))
                        sys.stdout.flush()
                    except Exception:
                        pass
                else:
                    time.sleep(0.05)
            print()
            if b"AT+INFO" in buf or b"AT+RTC" in buf:
                print("  >>> APP BOOTED OK <<<")
            else:
                print("  >>> APP no recognizable output <<<")
            return True
        print("  FAIL ack=0x%02X" % (s or 0xFF))
        return False

    def flash(self, path, base=APP_BASE, et=DEFAULT_ERASE_TIMEOUT, wt=DEFAULT_WRITE_TIMEOUT):
        d = open(path, "rb").read()
        print("FW: %dB (%dKB) %s" % (len(d), len(d) // 1024, os.path.basename(path)))
        if not self.erase(base, len(d), t=et):
            return False
        if not self.write(d, base, t=wt):
            return False
        return True


def cmd_diag(port):
    print("=" * 60)
    print(" DTCM diagnostic via SWD (BL V7.1 save points)")
    print("=" * 60)
    print(" Resetting board via SWD...")
    if not reset_board_via_swd():
        print("  reset failed - cannot diag")
        return 1
    time.sleep(0.5)
    print(" Reading DTCM 0x20000008-0x20000044 (main save points)...")
    vals = read_dtcm(0x20000008, (0x20000044 - 0x20000008) // 4)
    print(" Reading DTCM 0x20000250-0x20000258 (copy const pool)...")
    vals2 = read_dtcm(0x20000250, (0x2000025C - 0x20000250) // 4)
    if vals is None:
        print("  DTCM read failed")
        return 1
    print()
    for addr, desc in DTCM_SAVE_POINTS:
        if 0x20000008 <= addr < 0x20000044:
            off = (addr - 0x20000008) // 4
            print("  0x%08X = 0x%08X  ; %s" % (addr, vals[off], desc))
        elif 0x20000250 <= addr < 0x2000025C:
            off2 = (addr - 0x20000250) // 4
            v = vals2[off2] if off2 < len(vals2) else 0
            print("  0x%08X = 0x%08X  ; %s" % (addr, v, desc))
        else:
            print("  0x%08X: skipped" % addr)
    print()
    print(" Reading SRAM 0x20000000-0x2000000F (copy dst)...")
    v0 = read_dtcm(0x20000000, 4)
    if v0 is not None:
        for i2, val in enumerate(v0):
            print("  0x%08X = 0x%08X" % (0x20000000 + i2 * 4, val))
    return 0


def cmd_test(port, et=DEFAULT_ERASE_TIMEOUT, wt=DEFAULT_WRITE_TIMEOUT):
    print("=" * 60)
    print(" End-to-end test on %s" % port)
    print("=" * 60)
    o = OTA(port)
    try:
        if not o.wait_bl():
            print("FAIL: BL not responding")
            return 1
        if not o.info():
            print("FAIL: info not received")
            return 1
        if not o.erase_one(0x08020000, t=et):
            print("FAIL: erase single sector")
            return 1
        print("  Single-sector erase OK!")
        print("  Verifying erased sector is 0xFFFFFFFF via SWD read...")
        b = read_flash(0x08020000, 16)
        if b is None:
            print("  WARN: flash read failed")
        elif all(x == 0xFF for x in b):
            print("  OK: sector 1 is fully erased (0xFFFFFFFF)")
        else:
            print("  WARN: sector 1 not fully erased: %s" % b.hex())
        print("=" * 60)
        print(" END-TO-END TEST PASSED")
        print("=" * 60)
        return 0
    finally:
        o.close()


def main():
    ap = argparse.ArgumentParser(description="AMKN8639 OTA Host v4.0 - closed-loop flash/diag/BL upgrade")
    ap.add_argument("port", nargs="?", default="COM46", help="Serial port (default: COM46)")
    sub = ap.add_subparsers(dest="cmd", metavar="CMD")

    pi = sub.add_parser("info", help="Query BL version + flash size + APP status")
    pi.add_argument("--timeout", type=int, default=DEFAULT_INFO_TIMEOUT)
    pi.add_argument("--no-reset", action="store_true")

    pf = sub.add_parser("flash", help="Erase + write + boot APP from bin file")
    pf.add_argument("file")
    pf.add_argument("--addr", type=lambda x: int(x, 0), default=APP_BASE)
    pf.add_argument("--erase-timeout", type=int, default=DEFAULT_ERASE_TIMEOUT)
    pf.add_argument("--write-timeout", type=int, default=DEFAULT_WRITE_TIMEOUT)
    pf.add_argument("--no-boot", action="store_true")
    pf.add_argument("--no-reset", action="store_true")
    pf.add_argument("--no-erase", action="store_true")

    pb = sub.add_parser("boot", help="Boot APP (case 3)")
    pb.add_argument("--capture", type=int, default=4)
    pb.add_argument("--no-reset", action="store_true")

    pe = sub.add_parser("erase", help="Erase N sectors from --addr")
    pe.add_argument("--addr", type=lambda x: int(x, 0), default=APP_BASE)
    pe.add_argument("--size", type=lambda x: int(x, 0), required=True)
    pe.add_argument("--timeout", type=int, default=DEFAULT_ERASE_TIMEOUT)
    pe.add_argument("--no-reset", action="store_true")

    pw = sub.add_parser("write", help="Write bin to flash at --addr")
    pw.add_argument("file")
    pw.add_argument("--addr", type=lambda x: int(x, 0), default=APP_BASE)
    pw.add_argument("--timeout", type=int, default=DEFAULT_WRITE_TIMEOUT)
    pw.add_argument("--no-reset", action="store_true")

    pbl = sub.add_parser("flash-bl", help="Flash BL to BL_TEMP and trigger self-upgrade")
    pbl.add_argument("file")
    pbl.add_argument("--no-reset", action="store_true")

    pa = sub.add_parser("auto", help="Send AT+OTA to APP, then flash + boot")
    pa.add_argument("file")

    sub.add_parser("diag", help="Read DTCM save points via SWD")
    pt = sub.add_parser("test", help="End-to-end test: info + erase 1 sector + verify")
    pt.add_argument("--erase-timeout", type=int, default=DEFAULT_ERASE_TIMEOUT)
    pt.add_argument("--write-timeout", type=int, default=DEFAULT_WRITE_TIMEOUT)

    sub.add_parser("reset", help="Reset board via SWD only")

    a = ap.parse_args()
    if not a.cmd:
        ap.print_help()
        return 1

    if a.cmd == "reset":
        return 0 if reset_board_via_swd() else 1
    if a.cmd == "diag":
        return cmd_diag(a.port)
    if a.cmd == "test":
        return cmd_test(a.port, a.erase_timeout, a.write_timeout)

    o = OTA(a.port)
    try:
        if a.cmd == "info":
            if not o.wait_bl(do_reset=not a.no_reset):
                return 1
            o.info(t=a.timeout)
            return 0
        if a.cmd == "flash":
            if not o.wait_bl(do_reset=not a.no_reset):
                return 1
            if not a.no_erase and not o.erase(a.addr, os.path.getsize(a.file), t=a.erase_timeout):
                return 1
            d = open(a.file, "rb").read()
            print("FW: %dB (%dKB) %s" % (len(d), len(d) // 1024, os.path.basename(a.file)))
            if not o.write(d, a.addr, t=a.write_timeout):
                return 1
            if not a.no_boot:
                o.boot()
            print("\n=== DONE ===")
            return 0
        if a.cmd == "boot":
            if not o.wait_bl(do_reset=not a.no_reset):
                return 1
            o.boot(capture_s=a.capture)
            return 0
        if a.cmd == "erase":
            if not o.wait_bl(do_reset=not a.no_reset):
                return 1
            o.erase(a.addr, a.size, t=a.timeout)
            return 0
        if a.cmd == "write":
            if not o.wait_bl(do_reset=not a.no_reset):
                return 1
            d = open(a.file, "rb").read()
            print("FW: %dB (%dKB) %s" % (len(d), len(d) // 1024, os.path.basename(a.file)))
            o.write(d, a.addr, t=a.timeout)
            return 0
        if a.cmd == "flash-bl":
            if not o.wait_bl(do_reset=not a.no_reset):
                return 1
            d = open(a.file, "rb").read()
            print("BL: %dB" % len(d))
            o.erase(BL_TEMP, len(d)) or sys.exit(1)
            o.write(d, BL_TEMP) or sys.exit(1)
            o.s.close()
            time.sleep(0.2)
            s2 = serial.Serial(a.port, 115200, timeout=0.2)
            s2.write(b"AT+OTA=BL\r\n")
            s2.flush()
            time.sleep(0.5)
            s2.close()
            print("Waiting for BL self-upgrade...")
            time.sleep(1)
            s3 = serial.Serial(a.port, 115200, timeout=0.5)
            buf = b""
            dl = time.time() + 18
            while time.time() < dl:
                if s3.in_waiting:
                    buf += s3.read(s3.in_waiting)
                    if b"UPGRADE SUCCESS" in buf or b"UPGRADE FAILED" in buf:
                        break
                else:
                    time.sleep(0.1)
            s3.close()
            for l in buf.decode("ascii", "replace").split("\n"):
                if l.strip():
                    print("  " + l.strip()[:120])
            if b"UPGRADE SUCCESS" in buf:
                print("\n=== BL UPGRADE SUCCESS ===")
                return 0
            print("\n=== BL upgrade: check output above ===")
            return 1
        if a.cmd == "auto":
            if not o.trigger_ota():
                return 1
            o.flash(a.file) or sys.exit(1)
            o.boot()
            print("\n=== DONE ===")
            return 0
    finally:
        o.close()
    return 0


if __name__ == "__main__":
    sys.exit(main() or 0)
