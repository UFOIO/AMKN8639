/* AMKN8639 V12 - RCC/QSPI Register Dump Diagnostic */

#include <stdint.h>

#define RCC_BASE        0x58024400
#define RCC_CR          (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_ICSCR       (*(volatile uint32_t*)(RCC_BASE+0x04))
#define RCC_CFGR        (*(volatile uint32_t*)(RCC_BASE+0x10))
#define RCC_D1CFGR      (*(volatile uint32_t*)(RCC_BASE+0x18))
#define RCC_D2CFGR      (*(volatile uint32_t*)(RCC_BASE+0x1C))
#define RCC_D3CFGR      (*(volatile uint32_t*)(RCC_BASE+0x20))
#define RCC_D1CCIPR     (*(volatile uint32_t*)(RCC_BASE+0x4C))
#define RCC_D2CCIP1R    (*(volatile uint32_t*)(RCC_BASE+0x50))
#define RCC_AHB4ENR     (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR     (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_AHB3RSTR    (*(volatile uint32_t*)(RCC_BASE+0xE4))
#define RCC_AHB3LPENR   (*(volatile uint32_t*)(RCC_BASE+0xF4))
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE+0xF0))
#define FLASH_ACR       (*(volatile uint32_t*)0x52002000)

#define GPIOA_MODER     (*(volatile uint32_t*)0x58020000)
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)0x58020008)
#define GPIOA_AFRH      (*(volatile uint32_t*)0x58020024)
#define GPIOF_MODER     (*(volatile uint32_t*)0x58021400)
#define GPIOF_OSPEEDR   (*(volatile uint32_t*)0x58021408)
#define GPIOF_PUPDR     (*(volatile uint32_t*)0x5802140C)
#define GPIOF_AFRL      (*(volatile uint32_t*)0x58021420)
#define GPIOF_AFRH      (*(volatile uint32_t*)0x58021424)
#define GPIOG_MODER     (*(volatile uint32_t*)0x58021800)
#define GPIOG_OSPEEDR   (*(volatile uint32_t*)0x58021808)
#define GPIOG_PUPDR     (*(volatile uint32_t*)0x5802180C)
#define GPIOG_AFRH      (*(volatile uint32_t*)0x58021824)
#define GPIOI_MODER     (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR      (*(volatile uint32_t*)0x58022018)

#define QSPI_CR         (*(volatile uint32_t*)0x52005000)
#define QSPI_DCR        (*(volatile uint32_t*)0x52005004)
#define QSPI_SR         (*(volatile uint32_t*)0x52005008)
#define QSPI_FCR        (*(volatile uint32_t*)0x5200500C)
#define QSPI_DLR        (*(volatile uint32_t*)0x52005010)
#define QSPI_CCR        (*(volatile uint32_t*)0x52005014)
#define QSPI_AR         (*(volatile uint32_t*)0x52005018)
#define QSPI_DR         (*(volatile uint32_t*)0x52005020)

#define USART1_CR1      (*(volatile uint32_t*)0x40011000)
#define USART1_BRR      (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR      (*(volatile uint32_t*)0x4001101C)
#define USART1_TDR      (*(volatile uint32_t*)0x40011028)

static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void delay(volatile uint32_t n) { while(n--) __asm("nop"); }
static void led_blip(int ms) { led_on(); delay((uint32_t)ms * 64000); led_off(); }
static void led_blink(int n, int ms) { for(int i=0;i<n;i++){led_blip(ms);delay((uint32_t)ms*64000);} }

static void uart_putc(char c) {
    volatile int to = 1000000;
    while(!(USART1_ISR & (1<<7)) && --to) {}
    USART1_TDR = c;
}
static void uart_puts(const char *s) { while(*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v) {
    static const char h[] = "0123456789ABCDEF";
    for(int i = 28; i >= 0; i -= 4) uart_putc(h[(v >> i) & 0xF]);
}
#define PR(fmt, reg) do{uart_puts(fmt"=");uart_puthex(reg);uart_puts("\r\n");}while(0)

__asm void boot_jump(uint32_t sp, uint32_t pc) {
    MSR MSP, r0
    BX  r1
}

__attribute__((noreturn))
void Reset_Handler(void) {
    while(!(RCC_CR & (1<<2))) {}
    FLASH_ACR = 0x04;
    
    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16); GPIOI_MODER |= (1<<16);
    led_blink(2,30); delay(640000);
    
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);
    GPIOA_MODER &= ~((3<<18)|(3<<20));
    GPIOA_MODER |= (2<<18)|(2<<20);
    GPIOA_OSPEEDR |= (3<<18)|(3<<20);
    GPIOA_AFRH &= ~(0xFF<<4);
    GPIOA_AFRH |= (7<<4)|(7<<8);
    USART1_BRR = 64000000 / 115200;
    USART1_CR1 = (1<<3)|(1<<2)|(1<<0);
    { uint32_t p = GPIOG_IDR & (1<<6); uart_puts("PG6(pre)="); uart_puts(p?"HI\r\n":"LO\r\n"); }
    
    uart_puts("\r\n=== V14d Force-HI+RCC_RST ===\r\n");
    
    PR("RCC_CR      ", RCC_CR);
    PR("RCC_ICSCR   ", RCC_ICSCR);
    PR("RCC_CFGR    ", RCC_CFGR);
    PR("RCC_D1CFGR  ", RCC_D1CFGR);
    PR("RCC_D2CFGR  ", RCC_D2CFGR);
    PR("RCC_D3CFGR  ", RCC_D3CFGR);
    PR("RCC_D1CCIPR ", RCC_D1CCIPR);
    PR("RCC_D2CCIP1R", RCC_D2CCIP1R);
    PR("RCC_AHB3ENR ", RCC_AHB3ENR);
    PR("RCC_AHB3RSTR", RCC_AHB3RSTR);
    PR("RCC_AHB3LPENR", RCC_AHB3LPENR);
    
    /* Enable QSPI + GPIO clocks */
    RCC_AHB4ENR |= (1<<5)|(1<<6);
    /* Force QSPI pins HIGH as GPIO output first */
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((1<<16)|(1<<18)|(1<<20));
    GPIOG_MODER &= ~(3<<12); GPIOG_MODER |= (1<<12);
    GPIOF_BSRR = (1<<8)|(1<<9)|(1<<10);
    GPIOG_BSRR = (1<<6);
    __asm("dsb"); delay(100);
    /* Full RCC QSPI reset: enable, assert reset, disable clk, re-enable, deassert */
    RCC_AHB3ENR |= (1<<14); __asm("dsb"); delay(10);
    RCC_AHB3RSTR |= (1<<14); __asm("dsb"); delay(100);
    RCC_AHB3ENR &= ~(1<<14); __asm("dsb"); delay(1000);
    RCC_AHB3ENR |= (1<<14); __asm("dsb"); delay(100);
    RCC_AHB3RSTR &= ~(1<<14); __asm("dsb"); delay(1000);
    __asm("dsb"); delay(100);
    PR("AHB3ENR(after)", RCC_AHB3ENR);
    
    /* GPIO config */
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_PUPDR |= (1<<16)|(1<<18)|(1<<20);
    GPIOF_AFRL &= ~0xFF; GPIOF_AFRL |= (9<<0)|(9<<4);
    GPIOF_AFRH &= ~(0xF<<8); GPIOF_AFRH |= (9<<8);
    GPIOG_MODER &= ~(3<<12); GPIOG_MODER |= (2<<12);
    GPIOG_OSPEEDR |= (3<<12);
    GPIOG_PUPDR &= ~(3<<12); GPIOG_PUPDR |= (1<<12);
    GPIOG_AFRH &= ~(0xF<<8); GPIOG_AFRH |= (9<<8);
    
    PR("GPIOF_MODER", GPIOF_MODER);
    PR("GPIOG_MODER", GPIOG_MODER);
    
    /* Dump QSPI reset state */
    PR("QSPI_CR(rst) ", QSPI_CR);
    PR("QSPI_DCR(rst)", QSPI_DCR);
    PR("QSPI_SR(rst) ", QSPI_SR);
    PR("QSPI_CCR(rst)", QSPI_CCR);
    
    /* Init QSPI */
    QSPI_CR = 0; __asm("dsb"); delay(10);
    QSPI_FCR = 0x1F; __asm("dsb");
    QSPI_DCR = (0<<0) | (2<<8) | (22<<16);
    QSPI_CR = (3<<24) | (1<<9) | (1<<4) | (1<<6);
    __asm("dsb");
    
    PR("QSPI_DCR(set)", QSPI_DCR);
    PR("QSPI_CR(set) ", QSPI_CR);
    
    /* JEDEC */
    QSPI_CCR = 0x9F | (1<<8) | (1<<24) | (1<<26);
    QSPI_AR = 0;
    QSPI_DLR = 2;
    __asm("dsb");
    PR("QSPI_CCR     ", QSPI_CCR);
    PR("QSPI_SR(pre) ", QSPI_SR);
    
    QSPI_CR |= 1;
    __asm("dsb");
    delay(100);
    uint32_t sr = QSPI_SR;
    PR("QSPI_SR(post) ", sr);
    PR("QSPI_CR(post) ", QSPI_CR);
    
    if(sr & (1<<1)) {
        uint8_t b0 = (uint8_t)QSPI_DR;
        uint8_t b1 = (uint8_t)QSPI_DR;
        uint8_t b2 = (uint8_t)QSPI_DR;
        uint32_t id = ((uint32_t)b0<<16) | ((uint32_t)b1<<8) | b2;
        uart_puts("JEDEC OK ID="); uart_puthex(id); uart_puts("\r\n");
    } else {
        uart_puts("JEDEC FAIL (no TCF)\r\n");
    }
    
    led_blink(10, 20);
    
    uint32_t sp = *(volatile uint32_t*)0x08020000;
    uint32_t pc = *(volatile uint32_t*)0x08020004;
    uart_puts("Booting APP...\r\n\r\n");
    led_blip(150); delay(320000);
    QSPI_CR = 0; USART1_CR1 = 0; __asm("dsb");
    boot_jump(sp, pc);
}

void NMI_Handler(void) { while(1); }
void HardFault_Handler(void) { while(1){led_off();delay(2000000);led_on();delay(2000000);} }
void MemManage_Handler(void) { while(1); }
void BusFault_Handler(void) { while(1); }
void UsageFault_Handler(void) { while(1); }
void SysTick_Handler(void) {}

__attribute__((section(".vectors")))
const uint32_t vectors[] = {
    (uint32_t)0x2001FFC0, (uint32_t)Reset_Handler, (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler, (uint32_t)MemManage_Handler, (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler, 0,0,0,0,0,0,0,0, (uint32_t)SysTick_Handler,
};
