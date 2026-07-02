import struct
regs = {}
for n, p in [('RCC_CR','rcc_cr.bin'),('RCC_AHB4ENR','ahb4enr.bin'),('GPIOB_MODER','gpiob_moder.bin'),('GPIOB_ODR','gpiob_odr.bin')]:
    with open(p,'rb') as f: d=f.read()
    regs[n] = struct.unpack('<I',d)[0]
for n,v in regs.items():
    print(n.ljust(15),'= 0x%08X' % v)
print()
print('--- Analysis ---')
en = regs['RCC_AHB4ENR']
mod = regs['GPIOB_MODER']
odr = regs['GPIOB_ODR']
cr  = regs['RCC_CR']
print('RCC_AHB4ENR bit1 (GPIOBEN) =', (en>>1)&1, ' (should be 1 if LED init ran)')
print('GPIOB_MODER PB7 mode      =', (mod>>14)&3, ' (should be 1=output if config ran)')
print('GPIOB_ODR PB7 (LED bit7)  =', (odr>>7)&1, ' (should be 0=ON if LED ON ran)')
print('RCC_CR bit2 (HSIRDY)      =', (cr>>2)&1, ' (1=HSI ready)')
print('RCC_CR bit0 (HSION)       =', (cr>>0)&1)
