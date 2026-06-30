/* AMKN8639 V14c - GPIO Force-High + RCC Full Reset + Monitor */

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
#define GPIOF_IDR       (*(volatile uint32_t*)0x58021410)
#define GPIOF_BSRR      (*(volatile uint32_t*)0x58021418)
#define GPIOF_AFRL      (*(volatile uint32_t*)0x58021420)
#define GPIOF_AFRH      (*(volatile uint32_t*)0x58021424)
#define GPIOG_MODER     (*(volatile uint32_t*)0x58021800)
#define GPIOG_OSPEEDR   (*(volatile uint32_t*)0x58021808)
#define GPIOG_PUPDR     (*(volatile uint32_t*)0x5802180C)
#define GPIOG_IDR       (*(volatile uint32_t*)0x58021810)
#define GPIOG_BSRR      (*(volatile uint32_t*)0x58021818)
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
#define P(f,v) do{uart_puts(f"=");uart_puthex(v);uart_puts("\r\n");}while(0)

__asm void boot_jump(uint32_t sp, uint32_t pc) { MSR MSP, r0; BX r1; }

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
    
    uart_puts("\r\n=== V14c Force-HI + HardRST ===\r\n");
    
    /* Enable GPIOF + GPIOG clocks */
    RCC_AHB4ENR |= (1<<5)|(1<<6);
    
    /* === STEP 1: Force QSPI pins HIGH === */
    uart_puts("--- Step1: Force pins HI (GPIO out) ---\r\n");
    
    /* PF8,PF9,PF10,PG6 as GPIO output */
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((1<<16)|(1<<18)|(1<<20));
    GPIOG_MODER &= ~(3<<12); GPIOG_MODER |= (1<<12);
    
    /* Set all HIGH */
    GPIOF_BSRR = (1<<8)|(1<<9)|(1<<10);
    GPIOG_BSRR = (1<<6);
    __asm("dsb"); delay(100);
    
    P("  GPIOF_IDR", GPIOF_IDR);
    P("  GPIOG_IDR", GPIOG_IDR);
    
    /* === STEP 2: Full RCC QSPI reset === */
    uart_puts("--- Step2: RCC hard reset QSPI ---\r\n");
    
    /* Enable QSPI clock first */
    RCC_AHB3ENR |= (1<<14);
    __asm("dsb"); delay(10);
    
    /* Assert QSPI reset */
    RCC_AHB3RSTR |= (1<<14);
    __asm("dsb"); delay(100);
    
    /* Disable QSPI clock while in reset */
    RCC_AHB3ENR &= ~(1<<14);
    __asm("dsb"); delay(1000);
    
    /* Re-enable QSPI clock */
    RCC_AHB3ENR |= (1<<14);
    __asm("dsb"); delay(100);
    
    /* Deassert QSPI reset */
    RCC_AHB3RSTR &= ~(1<<14);
    __asm("dsb"); delay(1000);
    
    P("  AHB3ENR", RCC_AHB3ENR);
    P("  AHB3RSTR", RCC_AHB3RSTR);
    
    /* === STEP 3: Switch to AF9 === */
    uart_puts("--- Step3: Switch to AF9 ---\r\n");
    
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
    
    P("  GPIOF_IDR", GPIOF_IDR);
    P("  GPIOG_IDR", GPIOG_IDR);
    uint32_t pg6 = GPIOG_IDR & (1<<6);
    uart_puts("  PG6="); uart_puts(pg6 ? "HI\r\n" : "LO\r\n");
    
    /* === STEP 4: QSPI Init === */
    uart_puts("--- Step4: QSPI Init ---\r\n");
    
    QSPI_CR = 0; __asm("dsb"); delay(10);
    QSPI_FCR = 0x1F; __asm("dsb");
    QSPI_DCR = (0<<0) | (2<<8) | (22<<16);
    QSPI_CR = (3<<24) | (1<<9) | (1<<4) | (1<<6);
    __asm("dsb");
    
    P("  DCR", QSPI_DCR);
    P("  CR", QSPI_CR);
    
    /* === STEP 5: JEDEC === */
    uart_puts("--- Step5: JEDEC ---\r\n");
    
    QSPI_CCR = 0x9F | (1<<8) | (1<<24) | (1<<26);
    QSPI_AR = 0;
    QSPI_DLR = 2;
    __asm("dsb");
    
    P("  CCR", QSPI_CCR);
    P("  SR(pre)", QSPI_SR);
    pg6 = GPIOG_IDR & (1<<6);
    uart_puts("  PG6(pre)="); uart_puts(pg6 ? "HI\r\n" : "LO\r\n");
    
    QSPI_CR |= 1;
    __asm("dsb");
    delay(10);
    
    P("  SR(en1)", QSPI_SR);
    pg6 = GPIOG_IDR & (1<<6);
    uart_puts("  PG6(en)="); uart_puts(pg6 ? "HI\r\n" : "LO\r\n");
    
    volatile int to = 5000000;
    while(--to) {
        uint32_t sr = QSPI_SR;
        if(sr & (1<<1)) break;  /* TCF */
        if(sr & (1<<0)) break;  /* TE */
    }
    
    P("  SR(end)", QSPI_SR);
    pg6 = GPIOG_IDR & (1<<6);
    uart_puts("  PG6(end)="); uart_puts(pg6 ? "HI\r\n" : "LO\r\n");
    
    if(QSPI_SR & (1<<1)) {
        uint32_t id = ((uint32_t)(uint8_t)QSPI_DR<<16)|((uint32_t)(uint8_t)QSPI_DR<<8)|(uint8_t)QSPI_DR;
        P("JEDEC OK", id);
    } else {
        uart_puts("JEDEC FAIL\r\n");
    }
    
    QSPI_CR &= ~1; __asm("dsb");
    
    uint32_t sp = *(volatile uint32_t*)0x08020000;
    uint32_t pc = *(volatile uint32_t*)0x08020004;
    uart_puts("Booting APP...\r\n\r\n");
    led_blip(150); delay(320000);
    USART1_CR1 = 0; __asm("dsb");
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