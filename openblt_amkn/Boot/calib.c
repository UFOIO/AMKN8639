#include "stm32h7xx_hal.h"
void SystemInit(void) { }
int main(void)
{
    volatile unsigned int *RCC_AHB4ENR = (volatile unsigned int*)0x580244E0u;
    volatile unsigned int *GPIOI_MODER = (volatile unsigned int*)0x58022000u;
    volatile unsigned int *GPIOI_BSRR  = (volatile unsigned int*)0x58022018u;
    *RCC_AHB4ENR |= (1u << 8);
    *GPIOI_MODER |= (1u << 16);
    // Toggle PI8 at exactly 1Hz = 500ms on, 500ms off
    // At 64MHz HSI, each nop ~1 cycle = 15.6ns
    // 500ms = 32,000,000 nops
    while (1) {
        *GPIOI_BSRR = (1u<<8);  // ON
        { volatile unsigned int d = 32000000; while(--d) { __asm("nop"); } }
        *GPIOI_BSRR = (1u<<(8+16));  // OFF
        { volatile unsigned int d = 32000000; while(--d) { __asm("nop"); } }
    }
}
