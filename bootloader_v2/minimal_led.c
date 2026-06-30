#include <stdint.h>

/* ---- H743 Direct Register Addresses ---- */
#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR   (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define FLASH_ACR     (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER   (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR    (*(volatile uint32_t*)0x58022018)

__attribute__((noreturn))
void Reset_Handler(void){
    volatile int i;

    /* 1. HSI already ON after reset, just confirm ready */
    while(!(RCC_CR & (1<<2))){}

    /* 2. Flash 4 wait states for 64MHz HSI */
    FLASH_ACR = 0x04;

    /* 3. Enable GPIOI clock (AHB4 bit 8) */
    RCC_AHB4ENR |= (1<<8);

    /* 4. PI8 as output */
    GPIOI_MODER &= ~(3<<16);
    GPIOI_MODER |= (1<<16);

    /* 5. Blink forever: ON 200ms / OFF 200ms */
    while(1){
        /* LED ON: set PI8 */
        GPIOI_BSRR = (1<<8);
        for(i=0;i<3200000;i++) __asm("nop");

        /* LED OFF: reset PI8 */
        GPIOI_BSRR = (1<<24);
        for(i=0;i<3200000;i++) __asm("nop");
    }
}

/* Fault handlers - safe infinite loops */
void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){
    /* Blink fast on fault: rapid on/off */
    GPIOI_BSRR=(1<<24); for(volatile int i=0;i<1000000;i++)__asm("nop");
    GPIOI_BSRR=(1<<8);  for(volatile int i=0;i<1000000;i++)__asm("nop");
}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

/* Vector table at 0x08000000 */
__attribute__((section(".vectors")))
const uint32_t vectors[]={
    (uint32_t)0x2001FFC0,       /* Initial SP */
    (uint32_t)Reset_Handler,    /* Reset */
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,0,0,0,0,0,0,0,
    (uint32_t)SysTick_Handler,
};