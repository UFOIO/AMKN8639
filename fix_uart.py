path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

old = b'    USART1->BRR=64000000/115200;USART1->CR1=(1U<<13)|(1U<<3)|(1U<<2); /* UE+TE+RE */dly_ms(50);\r\n'
new = (
    b'    USART1->CR1=0;\r\n'
    b'    USART1->BRR=64000000/115200;\r\n'
    b'    USART1->CR1=(1U<<29)|(1U<<13); /* FIFOEN|UE */\r\n'
    b'    {volatile int t=5000000;while(!(USART1->ISR&(1U<<22))&&--t)__asm("nop");} /* wait REACK */\r\n'
    b'    USART1->CR1|=(1U<<3)|(1U<<2); /* TE|RE */\r\n'
    b'    {volatile int t=5000000;while(!(USART1->ISR&(1U<<21))&&--t)__asm("nop");} /* wait TEACK */\r\n'
    b'    dly_ms(50);\r\n'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
