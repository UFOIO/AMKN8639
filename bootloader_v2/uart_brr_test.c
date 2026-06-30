/* Test: write BRR, read back, blink result */
#include <stdint.h>

#define RCC_CR         (*(volatile uint32_t*)0x58024400)
#define RCC_AHB4ENR    (*(volatile uint32_t*)0x580244E0)
#define RCC_APB2ENR    (*(volatile uint32_t*)0x580244F8)
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define USART1_CR1     (*(volatile uint32_t*)0x40011000)
#define USART1_BRR     (*(volatile uint32_t*)0x4001100C)

static void led_on(void){ GPIOI_BSRR=(1<<8); }
static void led_off(void){ GPIOI_BSRR=(1<<24); }
static void dly(volatile uint32_t n){ while(n--)__asm("nop"); }

__attribute__((noreturn))
void Reset_Handler(void){
    uint32_t brr_read;

    while(!(RCC_CR&(1<<2))){}
    FLASH_ACR=0x04;
    RCC_AHB4ENR|=(1<<8);
    GPIOI_MODER&=~(3<<16); GPIOI_MODER|=(1<<16);

    RCC_APB2ENR|=(1<<4);           /* USART1 clock */
    dly(64000);                    /* let clock settle */

    USART1_BRR = 0x1234;           /* write test value */
    brr_read = USART1_BRR;         /* read back */

    if(brr_read == 0x1234){
        /* OK: 3 fast blinks */
        led_blink: led_on();dly(800000);led_off();dly(200000);
        goto led_blink;  /* infinite: USART alive! */
    } else {
        /* FAIL: slow blink */
        led_on(); dly(6400000); led_off(); dly(6400000);
        while(1){}  /* hang */
    }
    while(1);
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