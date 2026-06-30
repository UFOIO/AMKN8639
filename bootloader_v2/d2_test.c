/* Test USART3 BRR readback - is D2 domain alive? */
#include <stdint.h>

#define RCC_CR         (*(volatile uint32_t*)0x58024400)
#define RCC_AHB4ENR    (*(volatile uint32_t*)0x580244E0)
#define RCC_APB1LENR   (*(volatile uint32_t*)0x580244E8)
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define USART3_BRR     (*(volatile uint32_t*)0x4000480C)
#define QSPI_CR        (*(volatile uint32_t*)0x52005000)

static void led_on(void){ GPIOI_BSRR=(1<<8); }
static void led_off(void){ GPIOI_BSRR=(1<<24); }
static void dly(volatile uint32_t n){ while(n--)__asm("nop"); }

__attribute__((noreturn))
void Reset_Handler(void){
    uint32_t r3, rq;

    while(!(RCC_CR&(1<<2))){}
    FLASH_ACR=0x04;
    RCC_AHB4ENR|=(1<<8);
    GPIOI_MODER&=~(3<<16); GPIOI_MODER|=(1<<16);

    /* Test USART3 (APB1L bit 18) */
    RCC_APB1LENR|=(1<<18);
    dly(64000);
    USART3_BRR = 0x5678;
    r3 = USART3_BRR;

    /* Test QSPI (AHB3 bit 14) - different domain, known working */
    RCC_AHB4ENR = RCC_AHB4ENR; /* dummy read to ensure stability */
    QSPI_CR = 0x9ABC;
    rq = QSPI_CR;
    QSPI_CR = 0;

    /* Blink results: USART3 first, then QSPI */
    /* USART3: 2 blinks = OK, 5 = FAIL */
    if(r3 == 0x5678){
        int i; for(i=0;i<2;i++){led_on();dly(800000);led_off();dly(200000);}
    } else {
        int i; for(i=0;i<5;i++){led_on();dly(200000);led_off();dly(200000);}
    }
    dly(6400000);

    /* QSPI: 2 blinks = OK, 5 = FAIL */
    if(rq == 0x9ABC){
        int i; for(i=0;i<2;i++){led_on();dly(800000);led_off();dly(200000);}
    } else {
        int i; for(i=0;i<5;i++){led_on();dly(200000);led_off();dly(200000);}
    }
    dly(6400000);

    /* Loop: repeat test */
    while(1){
        RCC_APB1LENR|=(1<<18); dly(64000);
        USART3_BRR=0x5678; r3=USART3_BRR;
        QSPI_CR=0x9ABC; rq=QSPI_CR; QSPI_CR=0;

        if(r3==0x5678){led_on();dly(800000);led_off();dly(200000);led_on();dly(800000);led_off();}
        else{int i;for(i=0;i<5;i++){led_on();dly(200000);led_off();dly(200000);}}
        dly(6400000);
        if(rq==0x9ABC){led_on();dly(800000);led_off();dly(200000);led_on();dly(800000);led_off();}
        else{int i;for(i=0;i<5;i++){led_on();dly(200000);led_off();dly(200000);}}
        dly(12800000);
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