#include <stdint.h>

#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR   (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_APB2ENR   (*(volatile uint32_t*)(RCC_BASE+0xF8))

typedef struct { volatile uint32_t MODER,OSPEEDR,PUPDR,IDR,ODR,BSRR,LCKR,AFRL,AFRH; } GPIO_Reg;
#define GPIOA ((GPIO_Reg*)0x58020000)
#define GPIOI ((GPIO_Reg*)0x58022000)

typedef struct { volatile uint32_t CR1,CR2,CR3,BRR,GTPR,RTOR,RQR,ISR,ICR,RDR,TDR; } USART_Reg;
#define USART1 ((USART_Reg*)0x40011000)

static void uart_putc(char c){while(!(USART1->ISR&(1<<7)));USART1->TDR=c;}
static void uart_puts(const char*s){while(*s)uart_putc(*s++);}
static void uart_puthex(uint32_t v,int n){static const char h[]="0123456789ABCDEF";int i;for(i=n-1;i>=0;i--)uart_putc(h[(v>>(i*4))&0xF]);}

__attribute__((noreturn))
void Reset_Handler(void){
    int i;
    /* 1. Enable HSI */
    RCC_CR|=(1<<0);while(!(RCC_CR&(1<<2)));
    /* 2. Flash ACR 4WS */
    (*(volatile uint32_t*)0x52002000)=0x04;
    /* 3. LED on immediately */
    RCC_AHB4ENR|=(1<<8);
    GPIOI->MODER&=~(3<<16);GPIOI->MODER|=(1<<16);
    GPIOI->BSRR=(1<<8);
    /* 4. Init UART1: PA9/PA10 AF7, 115200 */
    RCC_AHB4ENR|=(1<<0);  /* GPIOA clock */
    RCC_APB2ENR|=(1<<4);  /* USART1 clock */
    GPIOA->MODER&=~((3<<18)|(3<<20)); GPIOA->MODER|=((2<<18)|(2<<20));
    GPIOA->AFRH&=~((0xF<<4)|(0xF<<8)); GPIOA->AFRH|=((7<<4)|(7<<8));
    USART1->BRR=64000000/115200;
    USART1->CR1=(1<<3)|(1<<2)|(1<<0);
    for(i=0;i<10000;i++)__asm("nop");
    /* 5. Send test pattern */
    uart_puts("\nTEST V3\n");
    /* 6. Blink LED: on-off-on-off to confirm we passed UART */
    while(1){
        GPIOI->BSRR=(1<<24); /* LED off */
        for(i=0;i<5000000;i++)__asm("nop");
        GPIOI->BSRR=(1<<8);  /* LED on */
        for(i=0;i<5000000;i++)__asm("nop");
        uart_puts("HELLO\n");
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){GPIOI->BSRR=(1<<24);for(int i=0;i<2000000;i++)__asm("nop");GPIOI->BSRR=(1<<8);for(int i=0;i<2000000;i++)__asm("nop");}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={(uint32_t)0x2001FFF0,(uint32_t)Reset_Handler,(uint32_t)NMI_Handler,(uint32_t)HardFault_Handler,(uint32_t)MemManage_Handler,(uint32_t)BusFault_Handler,(uint32_t)UsageFault_Handler,0,0,0,0,0,0,0,0,(uint32_t)SysTick_Handler,};