path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

# Replace LED blink with LED-ON + delay + while(1)
old = b'    /* LED blink test (no USART) */\r\n    RCC_AHB4ENR|=2;\r\n    GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14);\r\n    for(int i=0;i<3;i++){GPIOB->ODR&=~(1<<7);dly_ms(200);GPIOB->ODR|=(1<<7);dly_ms(200);}\r\n    RCC_CR|=1;while(!(RCC_CR&(1<<2)));'
new = (
    b'    /* LED-ON test */\r\n'
    b'    RCC_AHB4ENR|=2;\r\n'
    b'    GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14);\r\n'
    b'    GPIOB->ODR&=~(1<<7); /* LED ON */\r\n'
    b'    for(volatile int i=0;i<5000000;i++); /* delay so ST-Link can read */\r\n'
    b'    while(1){} /* HALT: dont run the rest */\r\n'
    b'    RCC_CR|=1;while(!(RCC_CR&(1<<2)));'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
