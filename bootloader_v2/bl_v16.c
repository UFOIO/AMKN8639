/* AMKN8639 V16 - PLL Clock Config + QSPI test
 * HSI=64MHz -> PLL VCO=480MHz -> SYSCLK=240MHz
 */

#include <stdint.h>

#define RCC_BASE        0x58024400
#define PWR_BASE        0x58024800
#define FLASH_BASE      0x52002000

#define RCC_CR          (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_ICSCR       (*(volatile uint32_t*)(RCC_BASE+0x04))
#define RCC_CFGR        (*(volatile uint32_t*)(RCC_BASE+0x10))
#define RCC_D1CFGR      (*(volatile uint32_t*)(RCC_BASE+0x18))
#define RCC_D2CFGR      (*(volatile uint32_t*)(RCC_BASE+0x1C))
#define RCC_D3CFGR      (*(volatile uint32_t*)(RCC_BASE+0x20))
#define RCC_PLLCKSELR   (*(volatile uint32_t*)(RCC_BASE+0x28))
#define RCC_PLLCFGR     (*(volatile uint32_t*)(RCC_BASE+0x2C))
#define RCC_PLL1DIVR    (*(volatile uint32_t*)(RCC_BASE+0x30))
#define RCC_PLL1FRACR   (*(volatile uint32_t*)(RCC_BASE+0x34))
#define RCC_D1CCIPR     (*(volatile uint32_t*)(RCC_BASE+0x4C))
#define RCC_AHB4ENR     (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR     (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_AHB3RSTR    (*(volatile uint32_t*)(RCC_BASE+0xE4))
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE+0xF0))

#define PWR_CR3         (*(volatile uint32_t*)(PWR_BASE+0x0C))
#define PWR_D3CR        (*(volatile uint32_t*)(PWR_BASE+0x18))
#define PWR_CSR1        (*(volatile uint32_t*)(PWR_BASE+0x04))

#define FLASH_ACR       (*(volatile uint32_t*)(FLASH_BASE+0x00))

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

static void delay(volatile uint32_t n) { while(n--) __asm("nop"); }
static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void led_blip(int ms) { led_on(); delay((uint32_t)ms * 64000); led_off(); }

static void uart_putc(char c) {
    volatile int to = 1000000;
    while(!(USART1_ISR & (1<<7)) && --to) {}
    USART1_TDR = c;
}
static void uart_puts(const char *s) { while(*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v) {
    static const char h[] = "0123456789ABCDEF";
    for(int i=28; i>=0; i-=4) uart_putc(h[(v>>i)&0xF]);
}
#define P(f,v) do{uart_puts(f"=");uart_puthex(v);uart_puts("\r\n");}while(0)

__asm void boot_jump(uint32_t sp, uint32_t pc) { MSR MSP, r0; BX r1; }

/* === PLL Clock Config: HSI 64MHz -> PLL VCO 480MHz -> SYSCLK 240MHz === */
static void SystemClock_Config(void) {
    /* 1. Set voltage scaling for 240MHz+ (VOS1) */
    PWR_D3CR |= (1<<14);           /* VOS = 01 (scale 1) */
    while(!(PWR_CSR1 & (1<<14))) {} /* Wait for ACTVOSRDY */
    
    /* 2. Set flash wait states for 240MHz (4 WS required) */
    FLASH_ACR = 0x04;              /* LATENCY=4 */
    
    /* 3. Configure PLL: HSI / DIVM * DIVN / DIVP */
    /* DIVM=4 (input=16MHz), DIVN=60 (VCO=960... too high!)
       Let's use: DIVM=4, DIVN=40, DIVP=2 -> VCO=640, SYSCLK=320 -> too high
       Better: DIVM=8 (input=8MHz), DIVN=100, DIVP=2 -> VCO=800, SYSCLK=400 -> too high
       
       Let's try: DIVM=4 (input=16), DIVN=30 (VCO=480), DIVP=2 -> SYSCLK=240 ? */
    
    /* PLLCKSELR: DIVM=4, PLLSRC=HSI */
    RCC_PLLCKSELR = (4 << 4)        /* DIVM = 4 */
                  | (0 << 0);       /* PLLSRC = HSI */
    
    /* PLLCFGR: DIVN=30, PLL1RGE=2 (4-8MHz range for VCO input)
       Wait, input is 16MHz after DIVM=4. PLL1RGE: 00=1-2, 01=2-4, 10=4-8, 11=8-16MHz
       DIVM=4 gives 64/4=16MHz. But DIVM range is 1-63, actual divide is DIVM+0? 
       
       In H7: Fref = HSI / DIVM. DIVM=4 actually gives... 
       
       Actually in H7 RM: PLL input clock = selected source / DIVMx where DIVMx is 1..63.
       But register value = DIVMx - 1. So DIVM=4 in register means actual divider = 5.
       
       Let me redo: register DIVM=3 ? actual=4 ? input=16MHz.
       PLL1RGE: 8-16 MHz range ? bits[3:2] of PLLCFGR = 11
       
       DIVN register value: VCO/input - 1 = 480/16 - 1 = 29 = 0x1D
    */
    
    /* PLLCFGR: PLL1VCOSEL=0 (wide VCO: 150-420 MHz? no, wide is 192-836) */
    /* Actually, let me simplify. PLL1VCOSEL=1 for wide range, PLL1RGE=11 for 8-16MHz */
    RCC_PLLCFGR = (1 << 1)         /* PLL1VCOSEL = wide range */
                | (3 << 2)         /* PLL1RGE = 8-16 MHz */
                | (29 << 4);       /* DIVN = 30 (register = 29) */
    
    /* PLL1DIVR: DIVP=2, DIVQ=?, DIVR=? 
       register DIVP = 1 (actual = 2) */
    RCC_PLL1DIVR = (1 << 9)        /* P = 2 */
                 | (2 << 0);       /* R = 3? no, just basic */
    
    /* Enable PLL */
    RCC_CR |= (1 << 24);           /* PLL1ON */
    while(!(RCC_CR & (1 << 25))) {} /* Wait PLL1RDY */
    
    /* 4. Configure domain prescalers */
    /* D1: SYSCLK from PLL, D1CPRE=1 (no division) */
    RCC_D1CFGR = (0 << 0);         /* D1CPRE = /1 */
    RCC_D2CFGR = (0 << 4)         /* D2PPRE1 = /1 */
               | (0 << 8);        /* D2PPRE2 = /1 */
    RCC_D3CFGR = (0 << 0);        /* D3PPRE = /1 */
    
    /* 5. Switch system clock to PLL1 */
    RCC_CFGR = (3 << 0);           /* SW = PLL1 */
    while((RCC_CFGR & (7<<3)) != (3<<3)) {}  /* Wait SWS = PLL1 */
    
    /* Update UART baud rate for new clock */
    USART1_BRR = 240000000 / 115200;
}

__attribute__((noreturn))
void Reset_Handler(void) {
    while(!(RCC_CR & (1<<2))) {}    /* Wait HSI ready */
    
    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16); GPIOI_MODER |= (1<<16);
    led_blip(30); delay(64000);
    
    /* UART1 init at HSI speed first */
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);
    GPIOA_MODER &= ~((3<<18)|(3<<20));
    GPIOA_MODER |= (2<<18)|(2<<20);
    GPIOA_OSPEEDR |= (3<<18)|(3<<20);
    GPIOA_AFRH &= ~(0xFF<<4);
    GPIOA_AFRH |= (7<<4)|(7<<8);
    USART1_BRR = 64000000 / 115200;
    USART1_CR1 = (1<<3)|(1<<2)|(1<<0);
    
    uart_puts("\r\n=== V16 PLL+QSPI ===\r\n");
    
    /* Configure PLL */
    uart_puts("Config PLL...\r\n");
    SystemClock_Config();
    uart_puts("PLL done.\r\n");
    P("RCC_CR", RCC_CR);
    P("RCC_CFGR", RCC_CFGR);
    P("RCC_D1CFGR", RCC_D1CFGR);
    P("RCC_D2CFGR", RCC_D2CFGR);
    
    /* Now QSPI init */
    RCC_AHB4ENR |= (1<<5)|(1<<6);
    RCC_AHB3ENR |= (1<<14);
    
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
    
    QSPI_CR = 0; __asm("dsb"); delay(10);
    QSPI_FCR = 0x1F; __asm("dsb");
    QSPI_DCR = (0<<0) | (5<<8) | (22<<16);   /* CSHT=5 per CubeMX */
    QSPI_CR = (3<<24) | (1<<9) | (1<<4) | (1<<6);
    __asm("dsb");
    
    P("DCR", QSPI_DCR);
    P("CR", QSPI_CR);
    
    /* JEDEC */
    QSPI_CCR = 0x9F | (1<<8) | (1<<24) | (1<<26);
    QSPI_AR = 0; QSPI_DLR = 2;
    __asm("dsb");
    
    P("SR(pre)", QSPI_SR);
    QSPI_CR |= 1; __asm("dsb"); delay(100);
    P("SR(post)", QSPI_SR);
    
    volatile int to = 5000000;
    while(--to) { if(QSPI_SR & ((1<<1)|(1<<0))) break; }
    P("SR(end)", QSPI_SR);
    
    if(QSPI_SR & (1<<1)) {
        uint32_t id = ((uint32_t)(uint8_t)QSPI_DR<<16)|((uint32_t)(uint8_t)QSPI_DR<<8)|(uint8_t)QSPI_DR;
        P("JEDEC OK", id);
    } else {
        uart_puts("JEDEC FAIL\r\n");
    }
    
    QSPI_CR &= ~1;
    
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
