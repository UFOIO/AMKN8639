/* UART Test - force HSI clock + OVER8 */
#include <stdint.h>

#define RCC_BASE       0x58024400
#define RCC_CR         (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR    (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_APB2ENR    (*(volatile uint32_t*)(RCC_BASE+0xF8))
#define RCC_D2CCIP2R   (*(volatile uint32_t*)(RCC_BASE+0x84))
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define GPIOA_MODER    (*(volatile uint32_t*)0x58020000)
#define GPIOA_OSPEEDR  (*(volatile uint32_t*)0x58020008)
#define GPIOA_PUPDR    (*(volatile uint32_t*)0x5802000C)
#define GPIOA_AFRH     (*(volatile uint32_t*)0x58020024)
#define USART1_CR1     (*(volatile uint32_t*)0x40011000)
#define USART1_CR3     (*(volatile uint32_t*)0x40011008)
#define USART1_BRR     (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR     (*(volatile uint32_t*)0x4001101C)
#define USART1_TDR     (*(volatile uint32_t*)0x40011028)

static void led_on(void){ GPIOI_BSRR = (1<<8); }
static void led_off(void){ GPIOI_BSRR = (1<<24); }
static void dly(volatile uint32_t n){ while(n--) __asm("nop"); }

__attribute__((noreturn))
void Reset_Handler(void){
    volatile int i;
    uint32_t isr;

    /* HSI + Flash */
    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    /* LED init */
    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16); GPIOI_MODER |= (1<<16);

    /* Force USART1 clock = HSI (64MHz) */
    RCC_D2CCIP2R &= ~(3<<2);   /* Clear USART1SEL */
    RCC_D2CCIP2R |= (3<<2);    /* Set to HSI */

    /* Enable clocks */
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);

    /* PA9 TX: AF7 + max drive */
    GPIOA_MODER &= ~(3<<18); GPIOA_MODER |= (2<<18);
    GPIOA_OSPEEDR |= (3<<18);
    GPIOA_PUPDR |= (1<<18);
    GPIOA_AFRH &= ~(0xF<<4); GPIOA_AFRH |= (7<<4);

    /* Disable USART, configure, re-enable */
    USART1_CR1 = 0;
    dly(1000);

    /* OVER8=1: 8x oversampling, BRR = fck/(8*baud) */
    /* 64000000/(8*115200) = 69.444 -> DIV=69, FRAC=4/8 */
    USART1_BRR = (69<<4) | 4;
    USART1_CR1 = (1<<15);  /* OVER8 */

    dly(1000);
    USART1_CR1 |= (1<<0);  /* UE */
    dly(64000);            /* ~1ms settle */

    USART1_CR1 |= (1<<3);  /* TE */
    {volatile int t=5000000; while(!(USART1_ISR & (1<<21)) && --t){}}

    /* Read ISR */
    isr = USART1_ISR;
    led_on(); dly(3200000); led_off(); dly(3200000);  /* blink 1 */

    /* Try TXE */
    if(isr & (1<<7)){
        /* TXE=1! Fast blink = success */
        led_on(); dly(800000); led_off(); dly(200000);
        led_on(); dly(800000); led_off(); dly(200000);
        led_on(); dly(800000); led_off(); dly(200000);
        /* Send test chars */
        USART1_TDR = 'H'; dly(640000);
        USART1_TDR = 'S'; dly(640000);
        USART1_TDR = 'I'; dly(640000);
    } else {
        /* TXE=0! Slow blink = fail */
        led_on(); dly(6400000); led_off(); dly(6400000);
    }

    while(1){
        led_on(); dly(6400000); led_off(); dly(6400000);
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){led_off();dly(2000000);led_on();dly(2000000);}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={
    (uint32_t)0x2001FFC0,(uint32_t)Reset_Handler,(uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,(uint32_t)MemManage_Handler,(uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,0,0,0,0,0,0,0,0,(uint32_t)SysTick_Handler,
};