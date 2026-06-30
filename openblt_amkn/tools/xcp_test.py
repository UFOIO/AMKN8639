#!/usr/bin/env python3
"""Quick XCP CONNECT test - verifies BL communication"""
import serial, time, sys

def test(port):
    ser = serial.Serial(port, 115200, timeout=2.0)
    print(f"=== XCP Connect Test on {port} ===")
    time.sleep(0.5)
    ser.reset_input_buffer()
    
    # CONNECT: [length=1][PID=0xFF]
    print("TX: 01 FF")
    ser.write(bytes([0x01, 0xFF]))
    ser.flush()
    
    # Read raw response (BL sends without length prefix)
    # CONNECT response: FF [resource] [comm] [max_cto] [max_dto] [proto] [minor] [transp]
    time.sleep(0.5)
    resp = ser.read(ser.in_waiting or 32)
    
    if resp:
        print(f"RX ({len(resp)} bytes): {resp.hex(' ')}")
        if resp[0] == 0xFF:
            print("[OK] CONNECT successful!")
            res = resp[1]
            print(f"  Resources: 0x{res:02X}")
            if res & 0x10: print("  PGM (Programming) available")
            if res & 0x01: print("  CAL/PAG available")
            print(f"  Max CTO: {resp[3]} bytes")
            print(f"  Max DTO: {resp[4]} bytes")
        elif resp[0] == 0xFE:
            print(f"[ERR] Code: 0x{resp[1]:02X}")
        else:
            # Might be debug banner - keep reading
            more = ser.read(512)
            text = (resp + more).decode('latin-1', errors='replace')
            print(f"  Text: {text[:200]}")
            print("[INFO] BL sent banner (not XCP response)")
    else:
        print("[FAIL] No response")
    
    ser.close()

if __name__ == '__main__':
    port = sys.argv[1] if len(sys.argv) > 1 else 'COM46'
    test(port)
