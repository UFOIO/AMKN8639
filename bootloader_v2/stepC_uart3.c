#include <stdint.h>

#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR   (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_APB1LENR   (*(volatile uint32_t*)(RCC_BASE+0xE8))
#define FLASH_ACR     (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER   (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR    (*(volatile uint32_t*)0x58022018)
#define GPIOB_MODER   (*(volatile uint32_t*)0x58020400)
#define GPIOB_OTYPER  (*(volatile uint32_t*)0x58020404)
#define GPIOB_OSPEEDR (*(volatile uint32_t*)0x58020408)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x5802040C)
#define GPIOB_AFRH    (*(volatile uint32_t*)0x58020424)
#define USART3_CR1    (*(volatile uint32_t*)0x40004800)
#define USART3_BRR    (*(volatile uint32_t*)0x4000480C)
#define USART3_ISR    (*(volatile uint32_t*)0x4000481C)
#define USART3_TDR    (*(volatile uint32_t*)0x40004828)

__attribute__((noreturn))
void Reset_Handler(void){
    volatile int i;

    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    RCC_AHB4ENR |= (1<<8);   /* GPIOI */
    RCC_AHB4ENR |= (1<<1);   /* GPIOB */
    RCC_APB1LENR |= (1<<18); /* USART3 */

    /* LED */
    GPIOI_MODER &= ~(3<<16);
    GPIOI_MODER |= (1<<16);
    GPIOI_BSRR = (1<<8);

    /* PB10 TX: AF7, push-pull, high speed, pull-up */
    GPIOB_MODER &= ~(3<<20);
    GPIOB_MODER |= (2<<20);
    GPIOB_OTYPER &= ~(1<<10);
    GPIOB_OSPEEDR |= (3<<20);
    GPIOB_PUPDR |= (1<<20);
    GPIOB_AFRH &= ~(0xF<<8);
    GPIOB_AFRH |= (7<<8);

    /* USART3: 115200 */
    USART3_BRR = (64000000 / 115200);
    USART3_CR1 = (1<<0);               /* UE */
    for(i=0;i<100000;i++) __asm("nop");
    USART3_CR1 |= (1<<3);              /* TE */
    {volatile int t=5000000; while(!(USART3_ISR & (1<<21)) && --t){}}
    /* Try TX */
    {volatile int t=5000000; while(!(USART3_ISR & (1<<7)) && --t){}}
    USART3_TDR = 'U';
    {volatile int t=5000000; while(!(USART3_ISR & (1<<7)) && --t){}}
    USART3_TDR = '3';

    while(1){
        GPIOI_BSRR = (1<<24);
        for(i=0;i<3200000;i++) __asm("nop");
        GPIOI_BSRR = (1<<8);
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