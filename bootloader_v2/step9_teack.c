#include <stdint.h>

#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR   (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_APB2ENR   (*(volatile uint32_t*)(RCC_BASE+0xF8))
#define FLASH_ACR     (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER   (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR    (*(volatile uint32_t*)0x58022018)
#define GPIOA_MODER   (*(volatile uint32_t*)0x58020000)
#define GPIOA_AFRH    (*(volatile uint32_t*)0x58020024)
#define USART1_CR1    (*(volatile uint32_t*)0x40011000)
#define USART1_BRR    (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR    (*(volatile uint32_t*)0x4001101C)
#define USART1_TDR    (*(volatile uint32_t*)0x40011028)

static void uart_putc(char c){
    volatile int timeout = 5000000;
    while(!(USART1_ISR & (1<<7)) && --timeout){}
    USART1_TDR = c;
}
static void uart_puts(const char* s){
    while(*s) uart_putc(*s++);
}

__attribute__((noreturn))
void Reset_Handler(void){
    volatile int i;

    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    RCC_AHB4ENR |= (1<<8);
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);

    GPIOI_MODER &= ~(3<<16);
    GPIOI_MODER |= (1<<16);
    GPIOI_BSRR = (1<<8);   /* LED ON immediately */

    GPIOA_MODER &= ~((3<<18)|(3<<20));
    GPIOA_MODER |= ((2<<18)|(2<<20));
    GPIOA_AFRH &= ~((0xF<<4)|(0xF<<8));
    GPIOA_AFRH |= ((7<<4)|(7<<8));

    USART1_BRR = 64000000 / 115200;
    USART1_CR1 = (1<<3)|(1<<0);  /* TE + UE only, NO RE */

    /* Wait for TEACK (ISR bit 21) - critical on H7 */
    {volatile int tmo=5000000; while(!(USART1_ISR & (1<<21)) && --tmo){}}

    uart_puts("\n**** BL UART WORKING ****\n");

    while(1){
        GPIOI_BSRR = (1<<24);
        for(i=0;i<3200000;i++) __asm("nop");
        GPIOI_BSRR = (1<<8);
        uart_puts("TICK\n");
        for(i=0;i<3200000;i++) __asm("nop");
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){
    GPIOI_BSRR=(1<<24); for(volatile int i=0;i<500000;i++)__asm("nop");
    GPIOI_BSRR=(1<<8);  for(volatile int i=0;i<500000;i++)__asm("nop");
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