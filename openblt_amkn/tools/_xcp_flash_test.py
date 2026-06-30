import serial, time, sys

PORT = 'COM46'
TEST_ADDR = 0x081C0000  # Safe: last sectors of bank 2
TEST_SIZE = 256          # Small test

XCP_RES = 0xFF
XCP_ERR = 0xFE

def send(ser, payload):
    frame = bytes([len(payload)]) + payload
    print(f'TX: {frame.hex(" ")}')
    ser.write(frame)
    ser.flush()

def recv(ser, timeout=3.0):
    t0 = time.time()
    while (time.time() - t0) < timeout:
        b = ser.read(1)
        if not b: continue
        if b[0] == XCP_RES:
            more = ser.read(32)
            return b + more
        elif b[0] == XCP_ERR:
            more = ser.read(1)
            return b + more
    return b''

def test():
    ser = serial.Serial(PORT, 115200, timeout=2.0)
    ser.reset_input_buffer()
    time.sleep(0.3)

    print('=== XCP Flash Test ===')

    # 1. CONNECT
    print('\n[1] CONNECT')
    send(ser, bytes([0xFF]))
    r = recv(ser)
    if not r or r[0] != XCP_RES:
        print(f'FAIL: {r.hex(" ") if r else "no response"}')
        return
    print(f'  OK  CTO={r[3]} PGM={"YES" if r[1]&0x10 else "NO"}')

    # 2. SET_MTA
    print(f'\n[2] SET_MTA 0x{TEST_ADDR:08X}')
    send(ser, bytes([0xF6,0,0,0,0,
        TEST_ADDR&0xFF, (TEST_ADDR>>8)&0xFF,
        (TEST_ADDR>>16)&0xFF, (TEST_ADDR>>24)&0xFF]))
    r = recv(ser)
    if not r or r[0] != XCP_RES: print('FAIL'); return
    print('  OK')

    # 3. PROGRAM_CLEAR (erase)
    print(f'\n[3] ERASE 0x{TEST_ADDR:08X} +{TEST_SIZE} bytes')
    send(ser, bytes([0xD1,0,0,
        TEST_SIZE&0xFF, (TEST_SIZE>>8)&0xFF,
        (TEST_SIZE>>16)&0xFF, (TEST_SIZE>>24)&0xFF]))
    r = recv(ser, timeout=10.0)
    if not r or r[0] != XCP_RES:
        print(f'FAIL: {r.hex(" ") if r else "no response"}')
        return
    print('  OK')

    # 4. PROGRAM (write test pattern)
    print(f'\n[4] WRITE test pattern')
    data = bytes([i & 0xFF for i in range(TEST_SIZE)])
    test_mta = TEST_ADDR
    for off in range(0, TEST_SIZE, 62):
        chunk = data[off:off+62]
        # SET_MTA
        addr = test_mta + off
        send(ser, bytes([0xF6,0,0,0,0,
            addr&0xFF,(addr>>8)&0xFF,(addr>>16)&0xFF,(addr>>24)&0xFF]))
        r = recv(ser)
        if not r or r[0] != XCP_RES:
            print(f'SET_MTA FAIL @ {addr:08X}')
            return
        # PROGRAM
        send(ser, bytes([0xD0, len(chunk)]) + chunk)
        r = recv(ser, timeout=5.0)
        if not r or r[0] != XCP_RES:
            print(f'WRITE FAIL @ {addr:08X}')
            return
        print(f'  {off:4d}/{TEST_SIZE} OK')
    print('  All chunks OK')

    # 5. FINALIZE (PROGRAM size=0)
    print('\n[5] FINALIZE')
    send(ser, bytes([0xD0, 0]))
    r = recv(ser, timeout=10.0)
    if not r or r[0] != XCP_RES:
        print(f'FAIL: {r.hex(" ") if r else "no response"}')
        return
    print('  OK')

    # 6. RESET
    print('\n[6] RESET')
    send(ser, bytes([0xCF]))
    time.sleep(0.5)
    ser.close()
    print('[DONE] XCP Flash test PASSED!')

if __name__ == '__main__':
    test()
