#include <stdint.h>
#include <string.h>

/* ---- Registers ---- */
#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR   (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_APB2ENR   (*(volatile uint32_t*)(RCC_BASE+0xF8))

typedef struct { volatile uint32_t MODER,OTYPER,OSPEEDR,PUPDR,IDR,ODR,BSRR,LCKR,AFRL,AFRH; } GPIO_Reg;
#define GPIOA ((GPIO_Reg*)0x58020000)
#define GPIOB ((GPIO_Reg*)0x58020400)
#define GPIOI ((GPIO_Reg*)0x58022000)

typedef struct { volatile uint32_t CR1,CR2,CR3,BRR,GTPR,RTOR,RQR,ISR,ICR,RDR,TDR; } USART_Reg;
#define USART1 ((USART_Reg*)0x40011000)

typedef struct { volatile uint32_t CSR,RVR,CVR,CALIB; } SYST_Reg;
#define SYST ((SYST_Reg*)0xE000E010)

/* ---- Globals ---- */
volatile uint32_t g_msTick;

/* ---- UART ---- */
void uart_putc(char c){while(!(USART1->ISR&(1<<7)));USART1->TDR=c;}
void uart_puts(const char*s){while(*s)uart_putc(*s++);}
void uart_puthex(uint32_t v,int n){static const char h[]="0123456789ABCDEF";int i;for(i=n-1;i>=0;i--)uart_putc(h[(v>>(i*4))&0xF]);}

/* ---- SysTick ---- */
void SysTick_Handler(void){g_msTick++;}
uint32_t millis(void){return g_msTick;}
void delay_ms(uint32_t ms){uint32_t s=millis();while((millis()-s)<ms){__asm("nop");}}

/* ---- LED ---- */
void led_on(void){GPIOI->BSRR=(1<<8);}
void led_off(void){GPIOI->BSRR=(1<<24);}
void led_blink(int n,int ms){int i;for(i=0;i<n;i++){led_on();delay_ms(ms);led_off();delay_ms(ms);}}

/* ---- SPI ---- */
uint8_t spi_xfer(uint8_t tx){
    while(!(*(volatile uint32_t*)(0x40013000+0x08)&(1<<1)));
    *(volatile uint8_t*)(0x40013000+0x0C)=tx;
    while(!(*(volatile uint32_t*)(0x40013000+0x08)&(1<<0)));
    return *(volatile uint8_t*)(0x40013000+0x0C);
}

__attribute__((noreturn))
void Reset_Handler(void){
    int hsi_ok, spi_ok;
    uint8_t mfg, dev;

    /* ====== 1. HSI 64MHz ====== */
    RCC_CR|=(1<<0);while(!(RCC_CR&(1<<2)));

    /* ====== 2. Flash 4 wait states ====== */
    *(volatile uint32_t*)0x52002000=0x04;
    g_msTick=0;

    /* ====== 3. PWR VOS Scale 1 (same as V1 BL) ====== */
    *(volatile uint32_t*)0x58024800|=(1<<14);

    /* ====== 4. Systick 1ms ====== */
    SYST->RVR=63999;SYST->CVR=0;SYST->CSR=7;

    /* ====== 5. LED init + ON ====== */
    RCC_AHB4ENR|=(1<<8);
    GPIOI->MODER&=~(3<<16);GPIOI->MODER|=(1<<16);
    led_on();

    /* ====== 6. UART1 init: PA9/PA10 AF7 115200 ====== */
    RCC_AHB4ENR|=(1<<0);
    RCC_APB2ENR|=(1<<4);
    GPIOA->MODER&=~((3<<18)|(3<<20));GPIOA->MODER|=((2<<18)|(2<<20));
    GPIOA->AFRH&=~((0xF<<4)|(0xF<<8));GPIOA->AFRH|=((7<<4)|(7<<8));
    USART1->BRR=64000000/115200;
    USART1->CR1=(1<<3)|(1<<2)|(1<<0);
    {volatile int i;for(i=0;i<10000;i++)__asm("nop");}

    /* ====== 7. SPI1 init: PA5/PA6/PA7 AF5 ====== */
    RCC_AHB4ENR|=(1<<1);
    RCC_APB2ENR|=(1<<12);
    GPIOA->MODER&=~((3<<10)|(3<<12)|(3<<14));GPIOA->MODER|=((2<<10)|(2<<12)|(2<<14));
    GPIOA->AFRL&=~((0xF<<20)|(0xF<<24));GPIOA->AFRL|=((5<<20)|(5<<24));
    GPIOA->AFRH&=~(0xF<<0);GPIOA->AFRH|=(5<<0);
    GPIOB->MODER&=~(3<<0);GPIOB->MODER|=(1<<0);GPIOB->BSRR=(1<<0);
    *(volatile uint32_t*)0x40013000=(1<<2)|(1<<1)|(3<<3)|(1<<6);

    /* ====== 8. UART: banner ====== */
    uart_puts("\nAMKN8639 BL V2 TEST\n");

    /* ====== 9. SPI Flash detect ====== */
    GPIOB->BSRR=(1<<16); /* CS low */
    spi_xfer(0x9F);
    mfg=spi_xfer(0xFF);dev=spi_xfer(0xFF);
    GPIOB->BSRR=(1<<0); /* CS high */
    if(mfg!=0&&mfg!=0xFF){
        uart_puts("SPI FLASH: ");uart_puthex(mfg,2);uart_puthex(dev,2);uart_puts("\n");
    }else{
        uart_puts("NO SPI FLASH\n");
    }

    /* ====== 10. OTA status check ====== */
    {
        uint32_t magic=*(volatile uint32_t*)0x081A0000;
        uint32_t status=*(volatile uint32_t*)0x081A0004;
        uint32_t sec=*(volatile uint32_t*)0x081A000C;
        uart_puts("OTA: magic=");uart_puthex(magic,8);
        uart_puts(" status=");uart_puthex(status,8);
        uart_puts(" sec=");uart_puthex(sec,8);uart_puts("\n");
    }

    /* ====== 11. TEST LOOP ====== */
    uart_puts("=== TEST LOOP ===\n");
    led_off();
    while(1){
        led_on();uart_puts("TICK\n");delay_ms(500);
        led_off();delay_ms(500);
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){GPIOI->BSRR=(1<<24);for(volatile int i=0;i<3000000;i++)__asm("nop");GPIOI->BSRR=(1<<8);for(volatile int i=0;i<3000000;i++)__asm("nop");}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}

__attribute__((section(".vectors")))
const uint32_t vectors[]={
    (uint32_t)0x2001FFF0,
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,0,0,0,0,0,0,0,
    (uint32_t)SysTick_Handler,
};