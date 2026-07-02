path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

# Insert LED blink test at very start of Reset_Handler
old = b'    RCC_CR|=1;while(!(RCC_CR&(1<<2)));'
new = (
    b'    /* LED blink test (no USART) */\r\n'
    b'    RCC_AHB4ENR|=2;\r\n'
    b'    GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14);\r\n'
    b'    for(int i=0;i<3;i++){GPIOB->ODR&=~(1<<7);dly_ms(200);GPIOB->ODR|=(1<<7);dly_ms(200);}\r\n'
    b'    RCC_CR|=1;while(!(RCC_CR&(1<<2)));'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
