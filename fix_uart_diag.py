path = r'BL_BUILD\src\bootloader.c'
with open(path, 'rb') as f:
    data = f.read()

# Restore USART initialization and add diagnostic output
old = b'    /* In a loop, keep incrementing magic3 so ST-Link reads can see it grow */\r\n    volatile uint32_t *magic3 = (volatile uint32_t*)0x20000018;\r\n    uint32_t c = 0;\r\n    while(1){\r\n        *magic3 = c++;\r\n        *(volatile uint32_t*)0x2000001C = RCC_AHB4ENR;\r\n    }'
new = (
    b'    /* Original USART init code with FIFOEN+ACK */\r\n'
    b'    RCC_AHB4ENR|=1;RCC_APB2ENR|=0x10;\r\n'
    b'    GPIOA->MODER&=~((3U<<18)|(3U<<20));GPIOA->MODER|=((2U<<18)|(2U<<20));\r\n'
    b'    GPIOA->AFRH&=~((0xFU<<4)|(0xFU<<8));GPIOA->AFRH|=((7U<<4)|(7U<<8));\r\n'
    b'    USART1->CR1=0;\r\n'
    b'    USART1->BRR=64000000/115200;\r\n'
    b'    USART1->CR1=(1U<<29)|(1U<<13); /* FIFOEN|UE */\r\n'
    b'    {volatile int t=5000000;while(!(USART1->ISR&(1U<<22))&&--t)__asm("nop");}\r\n'
    b'    USART1->CR1|=(1U<<3)|(1U<<2);\r\n'
    b'    {volatile int t=5000000;while(!(USART1->ISR&(1U<<21))&&--t)__asm("nop");}\r\n'
    b'    dly_ms(50);\r\n'
    b'    /* Send diagnostic to USART1 */\r\n'
    b'    ups("\\n=== BL V7.1 DIAG OK ===\\n");\r\n'
    b'    ups("RCC_AHB4ENR="); uph(RCC_AHB4ENR,8); ups("\\n");\r\n'
    b'    ups("CR1="); uph(USART1->CR1,8); ups(" ISR="); uph(USART1->ISR,8); ups("\\n");\r\n'
    b'    while(1){__asm("nop");}'
)
print('Found old x', data.count(old))
assert data.count(old) == 1
data = data.replace(old, new, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data))
