#include <stdint.h>

__attribute__((noreturn))
void Reset_Handler(void){
    volatile uint32_t *RCC_CR = (volatile uint32_t*)0x58024400;
    volatile uint32_t *FLASH_ACR = (volatile uint32_t*)0x52002000;
    volatile uint32_t *RCC_AHB4 = (volatile uint32_t*)0x580244E0;
    volatile uint32_t *GPIOI_MODER = (volatile uint32_t*)0x58022000;
    volatile uint32_t *GPIOI_BSRR = (volatile uint32_t*)0x58022014;
    volatile int i;
    /* 1. Enable HSI */
    *RCC_CR |= (1<<0);
    while(!(*RCC_CR & (1<<2)));
    /* 2. Flash 4 wait states — CRITICAL for H743 @ 64MHz */
    *FLASH_ACR = 0x04;
    /* 3. LED blink */
    *RCC_AHB4 |= (1<<8);
    *GPIOI_MODER &= ~(3<<16);
    *GPIOI_MODER |= (1<<16);
    while(1){
        *GPIOI_BSRR = (1<<8);
        for(i=0;i<5000000;i++)__asm("nop");
        *GPIOI_BSRR = (1<<24);
        for(i=0;i<5000000;i++)__asm("nop");
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){volatile uint32_t *BS=(volatile uint32_t*)0x58022014;*BS=(1<<24);for(volatile int i=0;i<2000000;i++)__asm("nop");*BS=(1<<8);for(volatile int i=0;i<2000000;i++)__asm("nop");}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={
    (uint32_t)0x2001FFF0,
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,0,0,0,0,0,0,0,
    (uint32_t)SysTick_Handler,
};