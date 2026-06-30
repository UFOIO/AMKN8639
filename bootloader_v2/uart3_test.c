/* Test USART3 only - 2 slow=OK, 5 fast=FAIL */
#include <stdint.h>
#define RCC_CR         (*(volatile uint32_t*)0x58024400)
#define RCC_AHB4ENR    (*(volatile uint32_t*)0x580244E0)
#define RCC_APB1LENR   (*(volatile uint32_t*)0x580244E8)
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define USART3_BRR     (*(volatile uint32_t*)0x4000480C)

static void led_on(void){ GPIOI_BSRR=(1<<8); }
static void led_off(void){ GPIOI_BSRR=(1<<24); }
static void dly(volatile uint32_t n){ while(n--)__asm("nop"); }

__attribute__((noreturn))
void Reset_Handler(void){
    uint32_t r;
    int i;
    while(!(RCC_CR&(1<<2))){}
    FLASH_ACR=0x04;
    RCC_AHB4ENR|=(1<<8);
    GPIOI_MODER&=~(3<<16); GPIOI_MODER|=(1<<16);

    RCC_APB1LENR|=(1<<18); dly(64000);
    USART3_BRR=0x5678; r=USART3_BRR;

    while(1){
        if(r==0x5678){
            for(i=0;i<2;i++){led_on();dly(3200000);led_off();dly(1600000);}
        }else{
            for(i=0;i<5;i++){led_on();dly(800000);led_off();dly(400000);}
        }
        dly(12800000); /* long pause */
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