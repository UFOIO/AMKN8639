#include "stm32h7xx_hal.h"
void SystemInit(void) { }
int main(void)
{
    volatile unsigned int *RCC_APB2ENR = (volatile unsigned int*)0x580244F0u;
    volatile unsigned int *RCC_AHB4ENR = (volatile unsigned int*)0x580244E0u;
    volatile unsigned int *GPIOA_MODER = (volatile unsigned int*)0x58020000u;
    volatile unsigned int *GPIOA_AFRH  = (volatile unsigned int*)0x58020024u;
    volatile unsigned int *GPIOA_PUPDR = (volatile unsigned int*)0x5802000Cu;
    volatile unsigned int *GPIOA_OSPEEDR = (volatile unsigned int*)0x58020008u;
    volatile unsigned int *USART1_CR1  = (volatile unsigned int*)0x40011000u;
    volatile unsigned int *USART1_BRR  = (volatile unsigned int*)0x4001100Cu;
    volatile unsigned int *USART1_ISR  = (volatile unsigned int*)0x4001101Cu;
    volatile unsigned int *USART1_TDR  = (volatile unsigned int*)0x40011028u;
    volatile unsigned int *GPIOI_MODER = (volatile unsigned int*)0x58022000u;
    volatile unsigned int *GPIOI_BSRR  = (volatile unsigned int*)0x58022018u;

    *RCC_AHB4ENR |= (1u << 0) | (1u << 8);
    *RCC_APB2ENR |= (1u << 4);
    (void)*RCC_APB2ENR;

    *GPIOA_MODER = (*GPIOA_MODER & ~((3u<<18)|(3u<<20))) | (2u<<18) | (2u<<20);
    *GPIOA_OSPEEDR = (*GPIOA_OSPEEDR & ~((3u<<18)|(3u<<20))) | (3u<<18) | (3u<<20);
    *GPIOA_PUPDR = (*GPIOA_PUPDR & ~((3u<<18)|(3u<<20))) | (1u<<18) | (1u<<20);
    *GPIOA_AFRH = (*GPIOA_AFRH & ~((0xFu<<4)|(0xFu<<8))) | (7u<<4) | (7u<<8);

    *USART1_CR1 = 0;
    *USART1_BRR = 0x1A0ABu;  // 9600 @ 64MHz
    *USART1_CR1 = (1u << 29) | 1u;
    *USART1_CR1 |= (1u << 3);
    { volatile int t = 500000; while (!(*USART1_ISR & (1u<<21)) && --t) {} }

    *GPIOI_MODER |= (1u << 16);
    *GPIOI_BSRR = (1u<<8);

    // Send continuous 0x55 (alternating bits, easy to spot)
    while (1) {
        { volatile int t = 500000; while (!(*USART1_ISR & (1u<<7)) && --t) {} }
        *USART1_TDR = 0x55;
    }
}
