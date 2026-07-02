import sys
path = 'BL_BUILD/src/bootloader.c'
with open(path, 'rb') as f:
    data = f.read()
old = b'    led_init();\r\n\r\n    uint32_t bkp=RTC_BKP0R;'
diag = (
    b'    led_init();\r\n'
    b'    ups("DBG CR1="); uph(USART1->CR1,8); ups(" ISR="); uph(USART1->ISR,8);\r\n'
    b'    ups(" BRR="); uph(USART1->BRR,8);\r\n'
    b'    ups(" APB2ENR="); uph(*(volatile uint32_t*)0x580244F0,8);\r\n'
    b'    ups(" AHB4ENR="); uph(*(volatile uint32_t*)0x580244E0,8); ups("\\\\n");\r\n'
    b'    while(1){__asm("nop");}\r\n'
    b'\r\n'
    b'    uint32_t bkp=RTC_BKP0R;'
)
count = data.count(old)
print('Found old x', count)
assert count == 1
data = data.replace(old, diag, 1)
with open(path, 'wb') as f:
    f.write(data)
print('size:', len(data), 'newlines:', data.count(b'\n'))
