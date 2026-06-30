/* AMKN8639 BootLoader V3.1 - Fix APP SP check for ITCM */
#include <stdint.h>

#define RCC_BASE       0x58024400
#define RCC_CR         (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR    (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR    (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_APB2ENR    (*(volatile uint32_t*)(RCC_BASE+0xF8))
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define GPIOA_MODER    (*(volatile uint32_t*)0x58020000)
#define GPIOA_OSPEEDR  (*(volatile uint32_t*)0x58020008)
#define GPIOA_PUPDR    (*(volatile uint32_t*)0x5802000C)
#define GPIOA_AFRH     (*(volatile uint32_t*)0x58020024)
#define GPIOF_MODER    (*(volatile uint32_t*)0x58021400)
#define GPIOF_OSPEEDR  (*(volatile uint32_t*)0x58021408)
#define GPIOF_PUPDR    (*(volatile uint32_t*)0x5802140C)
#define GPIOF_AFRH     (*(volatile uint32_t*)0x58021424)
#define GPIOF_AFRL     (*(volatile uint32_t*)0x58021420)
#define GPIOG_MODER    (*(volatile uint32_t*)0x58021800)
#define GPIOG_BSRR     (*(volatile uint32_t*)0x58021818)
#define USART1_CR1     (*(volatile uint32_t*)0x40011000)
#define USART1_BRR     (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR     (*(volatile uint32_t*)0x4001101C)
#define USART1_TDR     (*(volatile uint32_t*)0x40011028)
#define QSPI_CR        (*(volatile uint32_t*)0x52005000)
#define QSPI_DCR       (*(volatile uint32_t*)0x52005004)
#define QSPI_CCR       (*(volatile uint32_t*)0x52005014)
#define QSPI_AR        (*(volatile uint32_t*)0x52005018)
#define QSPI_DR        (*(volatile uint32_t*)0x52005020)
#define QSPI_DLR       (*(volatile uint32_t*)0x52005010)
#define APP_BASE       0x08020000

static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void delay_loops(volatile uint32_t n){ while(n--) __asm("nop"); }

static void led_blink(int times, int ms){
    int i;
    for(i=0;i<times;i++){
        led_on();  delay_loops((uint32_t)ms * 64000);
        led_off(); delay_loops((uint32_t)ms * 64000);
    }
}

static void uart_putc(char c){
    volatile int tmo = 2000000;
    while(!(USART1_ISR & (1<<7)) && --tmo){}
    USART1_TDR = c;
}
static void uart_puts(const char* s){ while(*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v, int n){
    static const char h[]="0123456789ABCDEF";
    int i;
    for(i=n-1;i>=0;i--) uart_putc(h[(v>>(i*4))&0xF]);
}

__asm void boot_jump(uint32_t sp, uint32_t pc){
    MSR MSP, r0
    BX  r1
}

/* Check if address is in valid RAM range (ITCM, DTCM, SRAM1-4, AXI) */
static int is_valid_ram(uint32_t addr){
    return (addr >= 0x00000000 && addr < 0x00010000) ||  /* ITCM */
           (addr >= 0x20000000 && addr < 0x20020000) ||  /* DTCM */
           (addr >= 0x24000000 && addr < 0x24080000) ||  /* AXI SRAM */
           (addr >= 0x30000000 && addr < 0x30048000) ||  /* SRAM1-3 */
           (addr >= 0x38000000 && addr < 0x38010000);    /* SRAM4 */
}
static int is_valid_flash(uint32_t addr){
    return addr >= 0x08020000 && addr < 0x08200000;
}

static uint32_t qspi_read_id(void){
    uint8_t rx[3];
    GPIOG_BSRR = (1<<22);
    QSPI_CCR = 0x9F | (1<<8) | (3<<26);
    QSPI_DLR = 2;
    QSPI_AR = 0;
    QSPI_CR = 1;
    rx[0] = (uint8_t)(QSPI_DR & 0xFF);
    rx[1] = (uint8_t)(QSPI_DR & 0xFF);
    rx[2] = (uint8_t)(QSPI_DR & 0xFF);
    QSPI_CR = 0;
    GPIOG_BSRR = (1<<6);
    return ((uint32_t)rx[0]<<16) | ((uint32_t)rx[1]<<8) | rx[2];
}

__attribute__((noreturn))
void Reset_Handler(void){
    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16);
    GPIOI_MODER |= (1<<16);
    led_on();
    delay_loops(640000);
    led_blink(3, 50);  /* BL START */

    /* UART init */
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);
    GPIOA_MODER &= ~(3<<18);
    GPIOA_MODER |= (2<<18);
    GPIOA_OSPEEDR |= (3<<18);
    GPIOA_PUPDR |= (1<<18);
    GPIOA_AFRH &= ~(0xF<<4);
    GPIOA_AFRH |= (7<<4);
    USART1_BRR = 64000000 / 115200;
    USART1_CR1 = (1<<0);
    delay_loops(640000);
    USART1_CR1 |= (1<<3);
    {volatile int t=5000000; while(!(USART1_ISR & (1<<21)) && --t){}}
    uart_puts("\nBL V3.1\n");

    /* QSPI + Flash */
    RCC_AHB3ENR |= (1<<14);
    RCC_AHB4ENR |= (1<<5) | (1<<6);
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR |= (1<<16)|(1<<18)|(1<<20);
    GPIOF_AFRL &= ~(0xFF);
    GPIOF_AFRL |= (9<<0)|(9<<4);
    GPIOF_AFRH &= ~(0xF<<8);
    GPIOF_AFRH |= (9<<8);
    GPIOG_MODER &= ~(3<<12);
    GPIOG_MODER |= (1<<12);
    GPIOG_BSRR = (1<<6);
    QSPI_DCR = (1<<0) | ((24-1)<<8);

    uart_puts("Flash: ");
    uint32_t fid = qspi_read_id();
    uart_puthex(fid, 6);
    if(fid != 0 && fid != 0xFFFFFF){
        uart_puts(" OK\n");
        led_blink(2, 80);
    } else {
        uart_puts(" FAIL\n");
        led_blink(5, 30);
    }

    /* APP check with ITCM support */
    uint32_t app_sp = *(volatile uint32_t*)APP_BASE;
    uint32_t app_pc = *(volatile uint32_t*)(APP_BASE + 4);
    uart_puts("APP: SP="); uart_puthex(app_sp, 8);
    uart_puts(" PC="); uart_puthex(app_pc, 8);
    uart_puts("\n");

    if(is_valid_ram(app_sp) && is_valid_flash(app_pc)){
        uart_puts("BOOT APP\n");
        led_blink(1, 200);
        delay_loops(6400000);
        boot_jump(app_sp, app_pc);
    }

    uart_puts("NO APP\n");
    while(1){
        led_on();  delay_loops(6400000);
        led_off(); delay_loops(6400000);
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){
    led_off(); delay_loops(2000000);
    led_on();  delay_loops(2000000);
}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={
    (uint32_t)0x2001FFC0,
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,0,0,0,0,0,0,0,
    (uint32_t)SysTick_Handler,
};