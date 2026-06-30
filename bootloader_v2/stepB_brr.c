#include <stdint.h>

#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR   (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_APB2ENR   (*(volatile uint32_t*)(RCC_BASE+0xF8))
#define FLASH_ACR     (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER   (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR    (*(volatile uint32_t*)0x58022018)
#define GPIOA_MODER   (*(volatile uint32_t*)0x58020000)
#define GPIOA_OTYPER  (*(volatile uint32_t*)0x58020004)
#define GPIOA_OSPEEDR (*(volatile uint32_t*)0x58020008)
#define GPIOA_PUPDR   (*(volatile uint32_t*)0x5802000C)
#define GPIOA_AFRH    (*(volatile uint32_t*)0x58020024)
#define USART1_CR1    (*(volatile uint32_t*)0x40011000)
#define USART1_BRR    (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR    (*(volatile uint32_t*)0x4001101C)
#define USART1_TDR    (*(volatile uint32_t*)0x40011028)

/* Wait with timeout for USART flag */
static int wait_flag(uint32_t flag, int timeout){
    while(!(USART1_ISR & flag) && --timeout){}
    return timeout > 0;
}

__attribute__((noreturn))
void Reset_Handler(void){
    volatile int i, ok;

    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    RCC_AHB4ENR |= (1<<8);  /* GPIOI */
    RCC_AHB4ENR |= (1<<0);  /* GPIOA */
    RCC_APB2ENR |= (1<<4);  /* USART1 */

    /* LED */
    GPIOI_MODER &= ~(3<<16);
    GPIOI_MODER |= (1<<16);
    GPIOI_BSRR = (1<<8);

    /* PA9 TX: AF7, push-pull, high speed, pull-up */
    GPIOA_MODER &= ~(3<<18);
    GPIOA_MODER |= (2<<18);
    GPIOA_OTYPER &= ~(1<<9);
    GPIOA_OSPEEDR |= (3<<18);
    GPIOA_PUPDR |= (1<<18);
    GPIOA_AFRH &= ~(0xF<<4);
    GPIOA_AFRH |= (7<<4);

    /* USART1 with PROPER BRR: 64000000/115200 = 555.555... */
    /* BRR = (555 << 4) | 9 = 0x22B9 */
    USART1_BRR = 0x22B9;

    /* UE only first */
    USART1_CR1 = (1<<0);
    ok = wait_flag(1<<0, 5000000);  /* wait TEACK-like: actually wait for UE */

    /* Now TE */
    USART1_CR1 |= (1<<3);
    ok = wait_flag(1<<21, 5000000); /* wait TEACK */

    /* Now try TXE */
    ok = wait_flag(1<<7, 5000000);
    if(ok){
        USART1_TDR = 'O';
        ok = wait_flag(1<<7, 5000000);
        if(ok) USART1_TDR = 'K';
    }

    while(1){
        GPIOI_BSRR = (1<<24);
        for(i=0;i<3200000;i++) __asm("nop");
        GPIOI_BSRR = (1<<8);
        /* Try to send TICK */
        if(wait_flag(1<<7, 1000000)) USART1_TDR = 'T';
        if(wait_flag(1<<7, 1000000)) USART1_TDR = 'I';
        if(wait_flag(1<<7, 1000000)) USART1_TDR = 'C';
        if(wait_flag(1<<7, 1000000)) USART1_TDR = 'K';
        if(wait_flag(1<<7, 1000000)) USART1_TDR = '\n';
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