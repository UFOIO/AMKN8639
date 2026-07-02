path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

old = b'    for(volatile int i=0;i<5000000;i++);\r\n    while(1){}'
new = (
    b'    /* In a loop, keep incrementing magic3 so ST-Link reads can see it grow */\r\n'
    b'    volatile uint32_t *magic3 = (volatile uint32_t*)0x20000018;\r\n'
    b'    uint32_t c = 0;\r\n'
    b'    while(1){\r\n'
    b'        *magic3 = c++;\r\n'
    b'        *(volatile uint32_t*)0x2000001C = RCC_AHB4ENR;\r\n'
    b'    }'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
