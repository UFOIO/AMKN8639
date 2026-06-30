#include <stdint.h>

__attribute__((noreturn))
void Reset_Handler(void) {
    volatile uint32_t *RCC_CR      = (uint32_t*)0x58024400u;
    volatile uint32_t *RCC_AHB4ENR = (uint32_t*)0x580244E0u;
    volatile uint32_t *FLASH_ACR   = (uint32_t*)0x52002000u;
    volatile uint32_t *GPIOI_MODER = (uint32_t*)0x58022000u;
    volatile uint32_t *GPIOI_BSRR  = (uint32_t*)0x58022018u;
    volatile int i;

    while (!(*RCC_CR & (1u << 2))) {}
    *FLASH_ACR = 0x04u;
    *RCC_AHB4ENR |= (1u << 8);
    *GPIOI_MODER &= ~(3u << 16);
    *GPIOI_MODER |= (1u << 16);

    while (1) {
        *GPIOI_BSRR = (1u << 8);
        for (i = 0; i < 3200000; i++) __asm("nop");
        *GPIOI_BSRR = (1u << 24);
        for (i = 0; i < 3200000; i++) __asm("nop");
    }
}
void NMI_Handler(void) { while(1) {} }
void HardFault_Handler(void) {
    volatile uint32_t *GPIOI_BSRR = (uint32_t*)0x58022018u;
    *GPIOI_BSRR = (1u << 24); /* LED off on fault */
    while(1) {}
}
void MemManage_Handler(void) { while(1) {} }
void BusFault_Handler(void) { while(1) {} }
void UsageFault_Handler(void) { while(1) {} }
void SysTick_Handler(void) {}

__attribute__((section(".vectors")))
const uint32_t vectors[] = {
    0x2001FFC0,
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,0,0,0,0,0,0,0,
    (uint32_t)SysTick_Handler,
};