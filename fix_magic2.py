path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

old = b'    *(volatile uint32_t*)0x30000000 = 0xDEADBEEF; /* SRAM1 mag ic0 */\r\n    *(volatile uint32_t*)0x30000004 = 0xCAFEBABE; /* magic1 */'
new = b'    *(volatile uint32_t*)0x20000000 = 0xDEADBEEF; /* DTCM magic0 */\r\n    *(volatile uint32_t*)0x20000004 = 0xCAFEBABE; /* DTCM magic1 */'
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
