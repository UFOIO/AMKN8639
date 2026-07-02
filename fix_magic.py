path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

# Replace LED test with: write magic to RAM then while(1)
old = b'    /* LED-ON test */\r\n    RCC_AHB4ENR|=2;\r\n    GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14);\r\n    GPIOB->ODR&=~(1<<7); /* LED ON */\r\n    for(volatile int i=0;i<5000000;i++); /* delay so ST-Link can read */\r\n    while(1){} /* HALT: dont run the rest */'
new = (
    b'    /* Magic write test */\r\n'
    b'    *(volatile uint32_t*)0x30000000 = 0xDEADBEEF; /* SRAM1 mag ic0 */\r\n'
    b'    *(volatile uint32_t*)0x30000004 = 0xCAFEBABE; /* magic1 */\r\n'
    b'    while(1){}\r\n'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
