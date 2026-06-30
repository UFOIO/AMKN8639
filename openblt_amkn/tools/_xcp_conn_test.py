import serial, time
ser = serial.Serial('COM46', 115200, timeout=2.0)
ser.reset_input_buffer()
time.sleep(0.2)

print('TX: 01 FF')
ser.write(bytes([0x01, 0xFF]))
ser.flush()

time.sleep(0.8)
resp = ser.read(ser.in_waiting or 256)

if resp:
    print(f'RX ({len(resp)} bytes): {resp.hex(" ")}')
    xcp_idx = resp.find(b'\xff')
    if xcp_idx >= 0 and len(resp) >= xcp_idx + 8:
        xcp = resp[xcp_idx:xcp_idx+8]
        print(f'XCP: {xcp.hex(" ")}')
        print(f'  Resources=0x{xcp[1]:02X} CTO={xcp[3]} DTO={xcp[4]}')
        print(f'  Proto={xcp[5]}.{xcp[6]} Transport={xcp[7]}')
        if xcp[1] & 0x10: print('  PGM AVAILABLE')
        print('[OK] CONNECT!')
    else:
        print(f'No full XCP packet at offset {xcp_idx}')
else:
    print('[FAIL] No response')
ser.close()
