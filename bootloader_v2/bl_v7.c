/* AMKN8639 MCUBoot V7 */
#include <stdint.h>
#include "flash_map.h"

#define RCC_CR          (*(volatile uint32_t*)0x58024400)
#define RCC_AHB4ENR     (*(volatile uint32_t*)0x580244E0)
#define RCC_AHB3ENR     (*(volatile uint32_t*)0x580244D4)
#define RCC_APB2ENR     (*(volatile uint32_t*)0x580244F0)
#define FLASH_ACR       (*(volatile uint32_t*)0x52002000)
#define GPIOA_MODER     (*(volatile uint32_t*)0x58020000)
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)0x58020008)
#define GPIOA_AFRH      (*(volatile uint32_t*)0x58020024)
#define GPIOF_MODER     (*(volatile uint32_t*)0x58021400)
#define GPIOF_OSPEEDR   (*(volatile uint32_t*)0x58021408)
#define GPIOF_PUPDR     (*(volatile uint32_t*)0x5802140C)
#define GPIOF_AFRL      (*(volatile uint32_t*)0x58021420)
#define GPIOF_AFRH      (*(volatile uint32_t*)0x58021424)
#define GPIOF_BSRR      (*(volatile uint32_t*)0x58021418)
#define GPIOG_MODER     (*(volatile uint32_t*)0x58021800)
#define GPIOG_OSPEEDR   (*(volatile uint32_t*)0x58021808)
#define GPIOG_PUPDR     (*(volatile uint32_t*)0x5802180C)
#define GPIOG_AFRH      (*(volatile uint32_t*)0x58021824)
#define GPIOI_MODER     (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR      (*(volatile uint32_t*)0x58022018)

#define QSPI_CR         (*(volatile uint32_t*)0x52005000)
#define QSPI_DCR        (*(volatile uint32_t*)0x52005004)
#define QSPI_SR         (*(volatile uint32_t*)0x52005008)
#define QSPI_FCR        (*(volatile uint32_t*)0x5200500C)
#define QSPI_DLR        (*(volatile uint32_t*)0x52005010)
#define QSPI_CCR        (*(volatile uint32_t*)0x52005014)
#define QSPI_AR         (*(volatile uint32_t*)0x52005018)
#define QSPI_DR         (*(volatile uint32_t*)0x52005020)

#define USART1_CR1      (*(volatile uint32_t*)0x40011000)
#define USART1_BRR      (*(volatile uint32_t*)0x4001100C)
#define USART1_ISR      (*(volatile uint32_t*)0x4001101C)
#define USART1_TDR      (*(volatile uint32_t*)0x40011028)

static void led_on(void){GPIOI_BSRR=(1<<8);}
static void led_off(void){GPIOI_BSRR=(1<<24);}
static void dly(volatile uint32_t n){while(n--)__asm("nop");}
static void blip(int ms){led_on();dly((uint32_t)ms*64000);led_off();}
static void blink(int n,int ms){for(int i=0;i<n;i++){blip(ms);dly((uint32_t)ms*64000);}}

static void upc(char c){volatile int t=1000000;while(!(USART1_ISR&(1<<7))&&--t){}USART1_TDR=c;}
static void ups(const char *s){while(*s)upc(*s++);}
static void uph(uint32_t v){static const char h[]="0123456789ABCDEF";for(int i=28;i>=0;i-=4)upc(h[(v>>i)&0xF]);}
static void upp(const char *s,uint32_t v){ups(s);uph(v);ups("\r\n");}
static int urx(void){return(USART1_ISR&(1<<5))!=0;}

__asm void jump(uint32_t sp,uint32_t pc){MSR MSP,r0;BX r1;}
static int vram(uint32_t a){return(a>=0x20000000&&a<0x20020000)||(a>=0x24000000&&a<0x24080000)||(a>=0x30000000&&a<0x30048000)||(a>=0x38000000&&a<0x38010000);}
static int vflash(uint32_t a){return a>=SLOT_A_BASE&&a<(SLOT_A_BASE+SLOT_A_SIZE);}

static uint32_t ct[256];
static void crcinit(void){for(uint32_t i=0;i<256;i++){uint32_t c=i;for(int j=0;j<8;j++)c=(c>>1)^((c&1)?0xEDB88320:0);ct[i]=c;}}

static void qinit(void){
    volatile uint32_t reg;

    QSPI_CR = 0; dly(100);
    QSPI_FCR = 0x1F; dly(100);

    QSPI_DCR = (0<<0)|(5<<8)|(22<<16);
    reg = QSPI_DCR;
    upp("DCR=",reg);

    /* EN=1 from the start */
    QSPI_CR = (3<<24)|(1<<9)|(1<<6)|(1<<0);
    dly(1000);
    reg = QSPI_CR;
    upp("CR=",reg);
    reg = QSPI_SR;
    upp("SR=",reg);
    reg = RCC_AHB3ENR;
    upp("AHB3=",reg);
}

static uint32_t jid(void){
    uint32_t id=0,cr,sr;

    /* Abort + disable */
    QSPI_CR |= (1<<1); dly(10);
    QSPI_CR &= ~1; dly(10);
    QSPI_FCR = 0x1F;

    /* CCR: JEDEC ID 0x9F */
    QSPI_CCR = 0x9F|(1<<8)|(1<<24)|(1<<26);
    upp("CCR=",QSPI_CCR);

    /* AR + DLR */
    QSPI_AR = 0;
    QSPI_DLR = 2;
    upp("DLR=",QSPI_DLR);
    upp("SR0=",QSPI_SR);

    /* EN=1 */
    QSPI_CR |= 1;
    upp("CRen=",QSPI_CR);
    upp("SR1=",QSPI_SR);

    /* Wait */
    {volatile int to=5000000;
    while(to--){
        sr=QSPI_SR;
        if(sr&(1<<1))break;
        if(sr&(1<<0)){upp("TE=",sr);QSPI_FCR=(1<<0);break;}
    }
    if(to<=0)upp("TO=",QSPI_SR);}

    if(QSPI_SR&(1<<1)){
        uint8_t b0=(uint8_t)QSPI_DR,b1=(uint8_t)QSPI_DR,b2=(uint8_t)QSPI_DR;
        id=((uint32_t)b0<<16)|((uint32_t)b1<<8)|b2;
    }
    upp("ID=",id);
    return id;
}

static ota_status_t ota;
static int ota_rd(void){
    const uint32_t*s=(const uint32_t*)OTA_STATUS_BASE;
    uint32_t*d=(uint32_t*)&ota;
    for(int i=0;i<sizeof(ota_status_t)/4;i++)d[i]=s[i];
    return(ota.magic==OTA_MAGIC)?0:-1;
}

__attribute__((noreturn))
void Reset_Handler(void){
    while(!(RCC_CR&(1<<2))){}
    FLASH_ACR=0x04;

    RCC_AHB4ENR|=(1<<8);GPIOI_MODER&=~(3<<16);GPIOI_MODER|=(1<<16);
    blink(3,30);dly(640000);

    RCC_AHB4ENR|=(1<<0);RCC_APB2ENR|=(1<<4);
    GPIOA_MODER&=~((3<<18)|(3<<20));GPIOA_MODER|=(2<<18)|(2<<20);
    GPIOA_OSPEEDR|=(3<<18)|(3<<20);GPIOA_AFRH&=~(0xFF<<4);GPIOA_AFRH|=(7<<4)|(7<<8);
    USART1_BRR=64000000/115200;USART1_CR1=(1<<3)|(1<<2)|(1<<0);

    ups("\r\n=== V7 ===\r\n");
    crcinit();

    RCC_AHB3ENR|=(1<<14);
    RCC_AHB4ENR|=(1<<5)|(1<<6);

    GPIOF_MODER&=~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER|=((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR|=(3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR&=~((3<<16)|(3<<18)|(3<<20));
    GPIOF_PUPDR|=(1<<16)|(1<<18)|(1<<20);
    GPIOF_AFRL&=~0xFF;GPIOF_AFRL|=(9<<0)|(9<<4);
    GPIOF_AFRH&=~(0xF<<8);GPIOF_AFRH|=(9<<8);

    GPIOG_MODER&=~(3<<12);GPIOG_MODER|=(2<<12);
    GPIOG_OSPEEDR|=(3<<12);GPIOG_PUPDR|=(1<<12);
    GPIOG_AFRH&=~(0xF<<8);GPIOG_AFRH|=(9<<8);

    qinit();

    uint32_t jd=0;
    for(int r=0;r<3;r++){
        upp("Try",r);
        jd=jid();
        if(jd&&jd!=0xFFFFFF)break;
        dly(640000);
    }

    if(jd&&jd!=0xFFFFFF){ups("Flash OK\r\n");blink(2,50);}
    else{ups("Flash FAIL\r\n");blink(5,20);dly(640000);}

    ups("OTA: ");if(ota_rd()==0)uph(ota.boot_status);else ups("none");ups("\r\n");

    uint32_t sp=*(volatile uint32_t*)SLOT_A_BASE;
    uint32_t pc=*(volatile uint32_t*)(SLOT_A_BASE+4);
    upp("SP=",sp);upp("PC=",pc);

    if(vram(sp)&&vflash(pc)){
        ups("Hold\r\n");dly(6400000);
        ups("Boot\r\n");blip(150);dly(320000);
        QSPI_CR&=~1;USART1_CR1=0;
        jump(sp,pc);
    }
    ups("NO APP\r\n");
    while(1){led_on();dly(6400000);led_off();dly(6400000);}
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){led_off();dly(2000000);led_on();dly(2000000);}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={0x2001FFC0,(uint32_t)Reset_Handler,(uint32_t)NMI_Handler,(uint32_t)HardFault_Handler,(uint32_t)MemManage_Handler,(uint32_t)BusFault_Handler,(uint32_t)UsageFault_Handler,0,0,0,0,0,0,0,0,(uint32_t)SysTick_Handler};
