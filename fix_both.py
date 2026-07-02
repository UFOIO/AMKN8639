path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

old = b'    *(volatile uint32_t*)0x20000000 = 0xDEADBEEF; /* DTCM magic0 */\r\n    *(volatile uint32_t*)0x20000004 = 0xCAFEBABE; /* DTCM magic1 */\r\n    while(1){}'
new = (
    b'    *(volatile uint32_t*)0x20000000 = 0xDEADBEEF; /* DTCM magic0 */\r\n'
    b'    *(volatile uint32_t*)0x20000004 = 0xCAFEBABE; /* DTCM magic1 */\r\n'
    b'    /* LED test */\r\n'
    b'    RCC_AHB4ENR|=2; /* GPIOBEN */\r\n'
    b'    *(volatile uint32_t*)0x20000008 = RCC_AHB4ENR; /* save RCC after |=2 */\r\n'
    b'    GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14); /* PB7 output */\r\n'
    b'    GPIOB->ODR&=~(1<<7); /* LED ON */\r\n'
    b'    *(volatile uint32_t*)0x20000010 = GPIOB->ODR; /* save ODR */\r\n'
    b'    *(volatile uint32_t*)0x20000014 = GPIOB->MODER; /* save MODER */\r\n'
    b'    for(volatile int i=0;i<5000000;i++);\r\n'
    b'    while(1){}'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
