#include <stdint.h>

#define RCC_CR         (*(volatile uint32_t*)0x58024400)
#define FLASH_ACR      (*(volatile uint32_t*)(0x52002000+0x00))
#define RCC_AHB4ENR    (*(volatile uint32_t*)(0x58024400+0xE0))
#define RCC_APB2ENR    (*(volatile uint32_t*)(0x58024400+0xF0))
#define SCB_VTOR       (*(volatile uint32_t*)0xE000ED08)
#define SCB_CPACR      (*(volatile uint32_t*)0xE000ED88)
#define SCB_AIRCR      (*(volatile uint32_t*)0xE000ED0C)
#define FLASH_KEYR     (*(volatile uint32_t*)(0x52002000+0x04))
#define FLASH_SR       (*(volatile uint32_t*)(0x52002000+0x10))
#define FLASH_CR       (*(volatile uint32_t*)(0x52002000+0x0C))
#define FLASH_SR_BSY   (1U<<0)
#define FLASH_CR_PG    (1U<<0)
#define FLASH_CR_SER   (1U<<1)
#define FLASH_CR_START (1U<<16)
#define FLASH_CR_PSIZE (3U<<4)
#define FLASH_LOCK     (1U<<31)
#define RTC_BKP0R      (*(volatile uint32_t*)0x58004000)
#define BKP_ENTER_XCP  0x424C4F54
#define BKP_UPGRADE_BL 0x424C5550
#define BKP_UPGRADED_OK 0x424C4F4B

typedef struct { volatile uint32_t MODER,OTYPER,OSPEEDR,PUPDR,IDR,ODR,BSRR,LCKR,AFRL,AFRH; } Gpio;
#define GPIOA ((Gpio*)0x58020000)
#define GPIOB ((Gpio*)0x58020400)
typedef struct { volatile uint32_t CR1,CR2,CR3,BRR,GTPR,RTOR,RQR,ISR,ICR,RDR,TDR; } Uart;
#define USART1 ((Uart*)0x40011000)
#define SRAM_BASE    0x20000000
#define BL_TEMP      0x081E0000

void dly_ms(int ms){for(volatile int i=0;i<ms*6400;i++)__asm("nop");}
void upc(char c){while(!(USART1->ISR&(1<<7)));USART1->TDR=c;}
void ups(const char*s){while(*s)upc(*s++);}
void uph(uint32_t v,int n){const char h[]="0123456789ABCDEF";for(int i=n-1;i>=0;i--)upc(h[(v>>(i*4))&15]);}
int  urdy(void){return(USART1->ISR&(1<<5))!=0;}
uint8_t ugc(void){while(!urdy());return(uint8_t)USART1->RDR;}
void led_init(void){RCC_AHB4ENR|=2;GPIOB->MODER&=~(3<<14);GPIOB->MODER|=(1<<14);GPIOB->ODR|=(1<<7);}
void led_on(void){GPIOB->ODR&=~(1<<7);}
void led_off(void){GPIOB->ODR|=(1<<7);}

int flash_unlock(void){if(!(FLASH_CR&FLASH_LOCK))return 0;FLASH_KEYR=0x45670123;FLASH_KEYR=0xCDEF89AB;return(FLASH_CR&FLASH_LOCK)?-1:0;}
int flash_wait(int t){while((FLASH_SR&FLASH_SR_BSY)&&--t)__asm("nop");return(FLASH_SR&FLASH_SR_BSY)?-1:0;}
void flash_lock(void){FLASH_CR|=FLASH_LOCK;}

/* === XCP Protocol === */
uint8_t xsum(uint8_t*b,int n){uint8_t c=0;while(n--)c^=*b++;return c;}
int recv_pkt(uint8_t*b,int m){
    uint32_t t=5000000;while(!urdy()&&--t)__asm("nop");if(!t)return-1;
    if(ugc()!=0xAA)return-1;
    t=2000000;while(!urdy()&&--t)__asm("nop");if(!t)return-2;
    int l=ugc();if(l<2||l>m+1)return-2;
    for(int i=0;i<l;i++){t=2000000;while(!urdy()&&--t)__asm("nop");if(!t)return-3;b[i]=ugc();}
    if(b[l-1]!=xsum(b,l-1))return-4;
    return l-1;
}
void send_ack(int s){uint8_t a[4]={0xAA,0x02,(uint8_t)s,0};a[3]=xsum(a+1,2);for(int i=0;i<4;i++)upc(a[i]);}

/* === RAM-based flash erase (runs from SRAM to avoid flash bus stall) === */
__attribute__((noinline))
static int erase_sector_sram(uint32_t sector) {
    if(FLASH_CR&FLASH_LOCK){FLASH_KEYR=0x45670123;FLASH_KEYR=0xCDEF89AB;}
    volatile int t=1000000;while((FLASH_SR&FLASH_SR_BSY)&&--t)__asm("nop");
    if(FLASH_SR&FLASH_SR_BSY)return-1;
    FLASH_CR=(sector<<8)|FLASH_CR_PSIZE|FLASH_CR_SER;FLASH_CR|=FLASH_CR_START;
    t=200000000;while((FLASH_SR&FLASH_SR_BSY)&&--t)__asm("nop");
    FLASH_CR&=~(FLASH_CR_SER|FLASH_CR_START);
    if(FLASH_SR&FLASH_SR_BSY)return-2;
    return 0;
}

__attribute__((noinline))
static int write_word_sram(uint32_t addr,uint32_t data){
    if(FLASH_CR&FLASH_LOCK){FLASH_KEYR=0x45670123;FLASH_KEYR=0xCDEF89AB;}
    volatile int t=1000000;while((FLASH_SR&FLASH_SR_BSY)&&--t)__asm("nop");
    if(FLASH_SR&FLASH_SR_BSY)return-1;
    FLASH_CR=(FLASH_CR&~FLASH_CR_SER)|FLASH_CR_PSIZE|FLASH_CR_PG;
    *(volatile uint32_t*)addr=data;
    t=10000000;while((FLASH_SR&FLASH_SR_BSY)&&--t)__asm("nop");
    FLASH_CR&=~FLASH_CR_PG;
    if(*(volatile uint32_t*)addr!=data)return-2;
    return 0;
}

/* Execute fn from SRAM. Returns fn's return value. */
typedef int (*erase_fn_t)(uint32_t);
typedef int (*write_fn_t)(uint32_t,uint32_t);

static int call_from_sram(void* fn, uint32_t arg1, uint32_t arg2, int is_write) {
    uint32_t* src = (uint32_t*)fn;
    uint32_t* dst = (uint32_t*)SRAM_BASE;
    /* Copy 128 instructions (512 bytes) - enough for our functions */
    for(int i=0;i<128;i++)dst[i]=src[i];
    __asm("dsb"); __asm("isb");
    if(is_write){
        return ((write_fn_t)(SRAM_BASE | 1))(arg1, arg2);
    } else {
        return ((erase_fn_t)(SRAM_BASE | 1))(arg1);
    }
}

static int flash_erase_safe(uint32_t addr){
    uint32_t sector=(addr-0x08000000)>>17;
    return call_from_sram((void*)erase_sector_sram, sector, 0, 0);
}
static int flash_write_safe(uint32_t addr, uint32_t data){
    return call_from_sram((void*)write_word_sram, addr, data, 1);
}

/* === BL self-upgrade (runs from SRAM) === */
static int bl_upgrade_sram(void){
    volatile uint32_t*src=(volatile uint32_t*)BL_TEMP;
    volatile uint32_t*dst=(volatile uint32_t*)0x08000000;
    int words=0;
    FLASH_KEYR=0x45670123;FLASH_KEYR=0xCDEF89AB;
    while(FLASH_SR&FLASH_SR_BSY)__asm("nop");
    FLASH_CR=(0U<<8)|FLASH_CR_PSIZE|FLASH_CR_SER;FLASH_CR|=FLASH_CR_START;
    while(FLASH_SR&FLASH_SR_BSY)__asm("nop");
    FLASH_CR&=~(FLASH_CR_SER|FLASH_CR_START);
    for(int i=0;i<32768;i++){
        uint32_t w=src[i];if(w==0xFFFFFFFF)break;
        FLASH_CR=(FLASH_CR&~FLASH_CR_SER)|FLASH_CR_PSIZE|FLASH_CR_PG;
        dst[i]=w;while(FLASH_SR&FLASH_SR_BSY)__asm("nop");
        FLASH_CR&=~FLASH_CR_PG;
        if(dst[i]!=w){words=-(i+100);break;}
        words=i+1;
    }
    FLASH_CR|=FLASH_LOCK;return words;
}
static int do_bl_upgrade_sram(void){
    uint32_t*s=(uint32_t*)bl_upgrade_sram;
    uint32_t*d=(uint32_t*)SRAM_BASE;
    for(int i=0;i<256;i++)d[i]=s[i];
    __asm("dsb");__asm("isb");
    return ((int(*)(void))(SRAM_BASE | 1))();
}

__attribute__((noreturn))
void boot_app(uint32_t pc){
    USART1->CR1=0;RCC_AHB4ENR&=~3;RCC_APB2ENR&=~(1<<4);
    __asm("cpsid i");__asm("dsb");__asm("isb");
    SCB_VTOR=0x08020000;
    __asm("cpsie i");__asm("dsb");__asm("isb");
    ((void(*)(void))pc)();
    while(1){}
}

void Reset_Handler(void);
__attribute__((section(".vectors"), used))
const uint32_t vectors[2]={0x20020000,(uint32_t)Reset_Handler};

__attribute__((noreturn))
void Reset_Handler(void){
    RCC_CR|=1;while(!(RCC_CR&(1<<2)));
    FLASH_ACR=0x04;
    SCB_CPACR|=((3UL<<20)|(3UL<<22));__asm("dsb");__asm("isb");
    RCC_AHB4ENR|=1;RCC_APB2ENR|=0x10;
    GPIOA->MODER&=~((3U<<18)|(3U<<20));GPIOA->MODER|=((2U<<18)|(2U<<20));
    GPIOA->AFRH&=~((0xFU<<4)|(0xFU<<8));GPIOA->AFRH|=((7U<<4)|(7U<<8));
    USART1->BRR=64000000/115200;USART1->CR1=(1U<<3)|(1U<<2)|(1U<<0);dly_ms(50);
    led_init();

    uint32_t bkp=RTC_BKP0R;
    if(bkp==BKP_UPGRADED_OK){RTC_BKP0R=0;ups("\n=== BL UPGRADED ===\n");for(int i=0;i<3;i++){led_on();dly_ms(300);led_off();dly_ms(200);}}
    ups("\n=== AMKN8639 BL V7.1 ===\n");

    if(bkp==BKP_UPGRADE_BL){
        RTC_BKP0R=0;ups("Self-upgrade...\n");led_on();
        int r=do_bl_upgrade_sram();
        if(r>0){ups("OK! ");uph(r*4,8);ups(" bytes\n");for(int i=0;i<3;i++){led_on();dly_ms(400);led_off();dly_ms(200);}RTC_BKP0R=BKP_UPGRADED_OK;dly_ms(2000);SCB_AIRCR=0x05FA0004;while(1){}}
        ups("FAIL!\n");led_off();
    }

    uint32_t*vt=(uint32_t*)0x08020000;uint32_t asp=vt[0],apc=vt[1];
    int appok=((asp>=0x20000000&&asp<=0x24080000)||(asp<=0x00010000))&&(apc>=0x08020000&&apc<0x08200000)&&(apc&1);
    ups("APP SP=");uph(asp,8);ups(" PC=");uph(apc,8);ups(appok?" OK\n":" INVALID\n");

    int xcp=0;
    if(bkp==BKP_ENTER_XCP){RTC_BKP0R=0;xcp=1;ups("Entered via APP request\n");}
    if(!xcp){ups("Wait 10s for XCP...\n");for(int t=0;t<10000000;t++){if(urdy()){ugc();xcp=1;ups("XCP!\n");break;}__asm("nop");if(t%500000==0)GPIOB->ODR^=0x80;}led_off();}
    if(!xcp&&appok){ups("Boot APP...\n");dly_ms(100);boot_app(apc);}
    if(!appok){ups("APP invalid\n");xcp=1;}
    if(!flash_unlock())ups("Flash OK\n");else ups("Flash LOCKED\n");

    uint8_t pk[256];ups("READY.\n");
    while(1){
        int pl=recv_pkt(pk,sizeof(pk));if(pl<1)continue;
        uint8_t c=pk[0];uint8_t*d=pk+1;int dl=pl-1;
        switch(c){
            case 1:if(dl>=4){uint32_t a=*(uint32_t*)d;ups("Erase ");uph(a,8);ups("\n");send_ack(flash_erase_safe(a)==0?0:1);}else send_ack(0xFF);break;
            case 2:if(dl>=8&&(dl&3)==0){int ok2=1;uint32_t a2=*(uint32_t*)d;for(int i2=0;i2<(dl-4)/4;i2++){if(flash_write_safe(a2+i2*4,*(uint32_t*)(d+4+i2*4))!=0){ok2=0;break;}}send_ack(ok2?0:1);}else send_ack(0xFF);break;
            case 3:send_ack(0);dly_ms(200);flash_lock();if(appok)boot_app(apc);ups("No APP\n");break;
            case 4:{uint8_t i[]={7,0,(128>>8),128,8,7,1,0,(uint8_t)appok};upc(0xAA);upc(10);upc(0x84);uint8_t ck=0x84;for(int j=0;j<9;j++){upc(i[j]);ck^=i[j];}upc(ck);}break;
            case 5:ups("LED\n");for(int i=0;i<3;i++){led_on();dly_ms(200);led_off();dly_ms(200);}send_ack(0);break;
            default:send_ack(0xFE);break;
        }
    }
}

void NMI_Handler(void){while(1){}}
void HardFault_Handler(void){while(1){}}
void MemManage_Handler(void){while(1){}}
void BusFault_Handler(void){while(1){}}
void UsageFault_Handler(void){while(1){}}