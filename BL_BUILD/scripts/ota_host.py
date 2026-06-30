#!/usr/bin/env python3
"""AMKN8639 OTA Host v3.1 - Working multi-word write (60 words/pkt)"""
import serial, struct, sys, time, os, argparse, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

APP_BASE=0x08020000; BL_TEMP=0x081E0000; SEC_SIZE=128*1024
WORDS_PER_PKT=60

def xsum(d): c=0; [c:=c^b for b in d]; return c&0xFF
def pkt(cmd,data=b""):
    pl=bytes([cmd])+data; return bytes([0xAA,len(pl)+1])+pl+bytes([xsum(pl)])

class OTA:
    def __init__(self,port,baud=115200):
        self.s=serial.Serial(port,baud,timeout=0.1); time.sleep(0.2)
    def close(self):
        if self.s and self.s.is_open: self.s.close()
    def _drain(self):
        time.sleep(0.1); self.s.read(self.s.in_waiting or 1)
    def _wait_ack(self,t=5):
        dl=time.time()+t
        while time.time()<dl:
            if self.s.in_waiting:
                r=self.s.read(self.s.in_waiting)
                for i in range(len(r)):
                    if r[i]==0xAA and i+3<len(r):
                        if r[i+3]==xsum(r[i+1:i+3]): return r[i+2]
            else: time.sleep(0.005)
        return None

    def trigger_ota(self):
        print("[1/5] Trigger OTA...")
        self.s.reset_input_buffer()
        self.s.write(b"AT+OTA\r\n"); self.s.flush()
        time.sleep(0.5)
        r=self.s.read(min(self.s.in_waiting,500)or 1)
        if r:
            for l in r.decode("ascii","replace").strip().split("\n"):
                if l.strip(): print("  "+l.strip()[:80])
        # Wait for BL
        buf=b""; dl=time.time()+10
        while time.time()<dl:
            if self.s.in_waiting:
                buf+=self.s.read(self.s.in_waiting)
                if b"READY" in buf: break
            else: time.sleep(0.05)
        time.sleep(0.2)
        # Send connect
        self.s.write(b"\xAA"); self.s.flush()
        time.sleep(0.3)
        r=self.s.read(min(self.s.in_waiting,200)or 1)
        print("  Connected!")
        return True

    def wait_bl(self,t=12):
        print("Reset board now...")
        self.s.reset_input_buffer()
        buf=b""; dl=time.time()+t
        while time.time()<dl:
            if self.s.in_waiting:
                buf+=self.s.read(self.s.in_waiting)
                if b"Wait" in buf:
                    self.s.write(b"\xAA"); self.s.flush()
                    time.sleep(0.3); self._drain(); return True
            else: time.sleep(0.1)
        return False

    def info(self):
        self.s.write(pkt(4)); self.s.flush(); time.sleep(0.3)
        r=self.s.read(30)
        if len(r)>=4 and r[0]==0xAA:
            print("  BL v%d.0  APP: %s" % (r[3],"OK" if len(r)>11 and r[11]==1 else "INVALID"))
            return True
        print("  No response"); return False

    def erase(self,base,size):
        n=(size+SEC_SIZE-1)//SEC_SIZE
        print("[2/5] Erase %d sector(s)..." % n)
        for i in range(n):
            addr=base+i*SEC_SIZE
            self.s.write(pkt(1,struct.pack("<I",addr))); self.s.flush()
            s=self._wait_ack(15)
            if s!=0: print("  FAIL @ 0x%08X" % addr); return False
            print("  [%d/%d] OK" % (i+1,n))
        return True

    def write(self,data,base=APP_BASE):
        while len(data)%4: data+=b"\xFF"
        words=struct.unpack("<%dI"%(len(data)//4),data)
        total=len(words); W=WORDS_PER_PKT
        print("[3/5] Write %d words (%dKB) batch=%d..." % (total,len(data)//1024,W))
        t0=time.time(); err=0
        for cs in range(0,total,W):
            ce=min(cs+W,total)
            d=struct.pack("<I",base+cs*4)
            for i in range(cs,ce): d+=struct.pack("<I",words[i])
            self.s.write(pkt(2,d)); self.s.flush()
            s=self._wait_ack(10)
            if s!=0: err+=1; print("\n  FAIL @ batch %d" % (cs//W))
            done=ce; pct=done*100//total
            if done%(W*8)==0 or done==total:
                et=time.time()-t0; eta=(et/done*total-et) if done>0 else 0
                print("\r  [%3d%%] %d/%d  %.1fKB/s  ETA %ds" % (pct,done,total,done*4/et/1024 if et>0 else 0,eta),end="",flush=True)
        print()
        et=time.time()-t0
        print("  Done %.1fs, %.1fKB/s, errors=%d" % (et,done*4/et/1024 if et>0 else 0,err))
        return err==0

    def boot(self):
        print("[4/5] Boot APP...")
        self.s.write(pkt(3)); self.s.flush()
        s=self._wait_ack(3)
        if s==0:
            print("  OK!"); time.sleep(2)
            r=self.s.read(min(self.s.in_waiting,500)or 1)
            for l in r.decode("ascii","replace").strip().split("\n"):
                if l.strip() and any(k in l for k in ["UART","Init"]): print("  "+l.strip()[:80])
            return True
        print("  FAIL"); return False

    def flash(self,path,base=APP_BASE):
        d=open(path,"rb").read()
        print("FW: %db (%dKB)  %s" % (len(d),len(d)//1024,os.path.basename(path)))
        return self.erase(base,len(d)) and self.write(d,base)

def main():
    ap=argparse.ArgumentParser(description="AMKN8639 OTA Host v3.1")
    ap.add_argument("port"); ap.add_argument("cmd",choices=["auto","flash","flash-bl","info","boot"])
    ap.add_argument("file",nargs="?"); ap.add_argument("--addr",type=lambda x:int(x,0))
    a=ap.parse_args(); o=OTA(a.port)
    try:
        if a.cmd=="auto":
            if not a.file: print("Need file"); sys.exit(1)
            o.trigger_ota() or sys.exit(1)
            o.flash(a.file) or sys.exit(1)
            o.boot(); print("\n=== DONE ===")
        elif a.cmd=="flash":
            if not a.file: print("Need file"); sys.exit(1)
            o.wait_bl() or sys.exit(1)
            o.flash(a.file,a.addr or APP_BASE) or sys.exit(1)
            o.boot(); print("\n=== DONE ===")
        elif a.cmd=="flash-bl":
            if not a.file: print("Need BL bin"); sys.exit(1)
            o.wait_bl() or sys.exit(1)
            d=open(a.file,"rb").read(); print("BL: %db"%len(d))
            o.erase(BL_TEMP,len(d)) or sys.exit(1)
            o.write(d,BL_TEMP) or sys.exit(1)
            o.s.close(); time.sleep(0.2)
            s2=serial.Serial(a.port,115200,timeout=0.2)
            s2.write(b"AT+OTA=BL\r\n"); s2.flush()
            time.sleep(0.5); s2.close()
            # Wait for upgrade result
            print("Waiting for BL self-upgrade...")
            time.sleep(1)
            s3=serial.Serial(a.port,115200,timeout=0.5)
            deadline=time.time()+18
            buf=b""
            while time.time()<deadline:
                if s3.in_waiting:
                    buf+=s3.read(s3.in_waiting)
                    if b"UPGRADE SUCCESS" in buf:
                        break
                    if b"UPGRADE FAILED" in buf:
                        break
                else: time.sleep(0.1)
            s3.close()
            for l in buf.decode("ascii","replace").split("\n"):
                if l.strip(): print("  "+l.strip()[:120])
            if b"UPGRADE SUCCESS" in buf:
                print("\n=== BL UPGRADE SUCCESS ===")
            else:
                print("\n=== BL upgrade: check output above ===")
        elif a.cmd=="info": o.wait_bl() or sys.exit(1); o.info()
        elif a.cmd=="boot": o.wait_bl() or sys.exit(1); o.boot()
    finally: o.close()

if __name__=="__main__": main()
