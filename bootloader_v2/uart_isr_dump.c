/* UART ISR dump via LED - blink out ISR bits */
#include <stdint.h>

#define RCC_CR         (*(volatile uint32_t*)0x58024400)
#define RCC_AHB4ENR    (*(volatile uint32_t*)0x580244E0)
#define RCC_APB2ENR    (*(volatile uint32_t*)0x580244F8)
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define GPIOA_MODER    (*(volatile uint32_t*)0x58020000)
#define GPIOA_OSPEEDR  (*(volatile uint32_t*)0x58020008)
#define GPIOA_PUPDR    (*(volatile uint32_t*)0x5802000C)
#define GPIOA_AFRH     (*(volatile uint32_t*)0x58020024)
#define USART1_CR1     (*(volatile uint32_t*)0x40011000)
#define USART1_BRR     (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR     (*(volatile uint32_t*)0x4001101C)

static void led_on(void){ GPIOI_BSRR = (1<<8); }
static void led_off(void){ GPIOI_BSRR = (1<<24); }
static void dly(volatile uint32_t n){ while(n--) __asm("nop"); }

/* Blink ISR value: long=1, short=0, pause between bits */
static void blink_isr(uint32_t val){
    int i;
    /* Start marker: 3 fast */
    led_on(); dly(200000); led_off(); dly(200000);
    led_on(); dly(200000); led_off(); dly(200000);
    led_on(); dly(200000); led_off(); dly(200000);
    dly(3200000);  /* pause */

    /* 24 bits: bit 23 down to bit 0 */
    for(i=23;i>=0;i--){
        if(val & (1<<i)){
            led_on(); dly(3200000); led_off();  /* LONG = 1 */
        } else {
            led_on(); dly(800000); led_off();   /* SHORT = 0 */
        }
        dly(1600000);  /* gap between bits */
    }
    /* End marker: 3 fast */
    led_on(); dly(200000); led_off(); dly(200000);
    led_on(); dly(200000); led_off(); dly(200000);
    led_on(); dly(200000); led_off(); dly(200000);
    dly(12800000);  /* long pause before repeat */
}

__attribute__((noreturn))
void Reset_Handler(void){
    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    RCC_AHB4ENR |= (1<<8);  RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);
    GPIOI_MODER &= ~(3<<16); GPIOI_MODER |= (1<<16);

    /* PA9 TX */
    GPIOA_MODER &= ~(3<<18); GPIOA_MODER |= (2<<18);
    GPIOA_OSPEEDR |= (3<<18); GPIOA_PUPDR |= (1<<18);
    GPIOA_AFRH &= ~(0xF<<4); GPIOA_AFRH |= (7<<4);

    /* Init USART1 sequence */
    USART1_CR1 = 0;
    dly(1000);
    USART1_BRR = 64000000/115200;
    USART1_CR1 = (1<<0);  /* UE */
    dly(64000);
    USART1_CR1 |= (1<<3); /* TE */
    {volatile int t=5000000; while(!(USART1_ISR & (1<<21)) && --t){}}

    /* Read and blink ISR forever */
    while(1){
        uint32_t isr = USART1_ISR;
        blink_isr(isr);
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