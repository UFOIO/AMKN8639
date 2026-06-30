#include <stdint.h>

#define RCC_CR         (*(volatile uint32_t*)0x58024400)
#define RCC_AHB4ENR    (*(volatile uint32_t*)(0x58024400+0xE0))
#define RCC_APB2ENR    (*(volatile uint32_t*)(0x58024400+0xF0))
#define SCB_VTOR       (*(volatile uint32_t*)0xE000ED08)
#define SCB_CPACR      (*(volatile uint32_t*)0xE000ED88)
#define FLASH_KEYR     (*(volatile uint32_t*)(0x52002000+0x04))
#define FLASH_SR       (*(volatile uint32_t*)(0x52002000+0x10))
#define FLASH_CR       (*(volatile uint32_t*)(0x52002000+0x18))
#define FLASH_SR_BSY   (1U<<0)
#define FLASH_CR_PG    (1U<<0)
#define FLASH_CR_SER   (1U<<1)
#define FLASH_CR_START (1U<<16)
#define FLASH_CR_PSIZE (3U<<4)
#define FLASH_LOCK     (1U<<31)
#define RTC_BKP0R      (*(volatile uint32_t*)0x58004000)
#define BKP_ENTER_XCP  0x424C4F54
#define BKP_UPGRADE_BL 0x424C5550

typedef struct { volatile uint32_t MODER,OTYPER,OSPEEDR,PUPDR,IDR,ODR,BSRR,LCKR,AFRL,AFRH; } Gpio;
#define GPIOA ((Gpio*)0x58020000)
#define GPIOB ((Gpio*)0x58020400)
typedef struct { volatile uint32_t CR1,CR2,CR3,BRR,GTPR,RTOR,RQR,ISR,ICR,RDR,TDR; } Uart;
#define USART1 ((Uart*)0x40011000)
#define ITCM_RAM 0x00000000
#define BL_TEMP  0x081E0000

void dly_ms(int ms){for(int i=0;i<ms*6400;i++)__asm("nop");}
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
int flash_erasector(uint32_t a){if(flash_unlock())return-1;if(flash_wait(1000000))return-2;uint32_t s=(a-0x08000000)>>17;FLASH_CR=(FLASH_CR&~(0xFFU<<8))|(s<<8)|FLASH_CR_PSIZE|FLASH_CR_SER;FLASH_CR|=FLASH_CR_START;if(flash_wait(200000000))return-3;FLASH_CR&=~(FLASH_CR_SER|FLASH_CR_START);return 0;}
int flash_wword(uint32_t a,uint32_t d){if(flash_wait(1000000))return-1;FLASH_CR=(FLASH_CR&~FLASH_CR_SER)|FLASH_CR_PSIZE|FLASH_CR_PG;*(volatile uint32_t*)a=d;if(flash_wait(1000000))return-2;FLASH_CR&=~FLASH_CR_PG;if(*(volatile uint32_t*)a!=d)return-3;return 0;}
void flash_lock(void){FLASH_CR|=FLASH_LOCK;}

uint8_t xsum(uint8_t*b,int n){uint8_t c=0;while(n--)c^=*b++;return c;}
int recv_pkt(uint8_t*b,int m){uint32_t t=5000000;while(!urdy()&&--t)__asm("nop");if(!t)return-1;if(ugc()!=0xAA)return-1;t=2000000;while(!urdy()&&--t)__asm("nop");if(!t)return-2;int l=ugc();if(l<2||l>m+1)return-2;for(int i=0;i<l;i++){t=2000000;while(!urdy()&&--t)__asm("nop");if(!t)return-3;b[i]=ugc();}if(b[l-1]!=xsum(b,l-1))return-4;return l-1;}
void send_ack(int s){uint8_t a[4]={0xAA,0x02,(uint8_t)s,0};a[3]=xsum(a+1,2);for(int i=0;i<4;i++)upc(a[i]);}

__attribute__((noreturn))
void boot_app(uint32_t pc){USART1->CR1=0;RCC_AHB4ENR&=~3;RCC_APB2ENR&=~(1<<4);__asm("cpsid i");__asm("dsb");__asm("isb");SCB_VTOR=0x08020000;__asm("cpsie i");__asm("dsb");__asm("isb");((void(*)(void))pc)();while(1){}}

/* BL self-upgrade: copy from temp area to BL sector.
   This function is copied to ITCM and executed there,
   so it can safely erase+program sector 0. */
static int bl_upgrade_ram(void) {
    volatile uint32_t *src = (volatile uint32_t*)(BL_TEMP);
    volatile uint32_t *dst = (volatile uint32_t*)0x08000000;
    int ok = 1;

    /* Must be running from ITCM! */
    /* Erase sector 0 */
    FLASH_KEYR = 0x45670123;
    FLASH_KEYR = 0xCDEF89AB;
    while (FLASH_SR & FLASH_SR_BSY) __asm("nop");
    FLASH_CR = (0U<<8) | FLASH_CR_PSIZE | FLASH_CR_SER;
    FLASH_CR |= FLASH_CR_START;
    while (FLASH_SR & FLASH_SR_BSY) __asm("nop");
    FLASH_CR &= ~(FLASH_CR_SER | FLASH_CR_START);

    /* Copy word by word */
    for (int i = 0; i < 32768; i++) { /* up to 128KB */
        uint32_t w = src[i];
        if (w == 0xFFFFFFFF) break; /* end */
        FLASH_CR = (FLASH_CR & ~FLASH_CR_SER) | FLASH_CR_PSIZE | FLASH_CR_PG;
        dst[i] = w;
        while (FLASH_SR & FLASH_SR_BSY) __asm("nop");
        FLASH_CR &= ~FLASH_CR_PG;
        if (dst[i] != w) { ok = 0; break; }
    }
    FLASH_CR |= FLASH_LOCK;
    return ok ? 0 : -1;
}

/* Copy function to ITCM and execute. Returns 0 on success. */
static int do_bl_upgrade(void) {
    /* Copy bl_upgrade_ram to ITCM */
    volatile uint32_t *src = (volatile uint32_t*)bl_upgrade_ram;
    volatile uint32_t *dst = (volatile uint32_t*)ITCM_RAM;
    for (int i = 0; i < 128; i++) dst[i] = src[i];
    __asm("dsb"); __asm("isb");

    /* Execute from ITCM */
    int (*fn)(void) = (int(*)(void))ITCM_RAM;
    return fn();
}

__attribute__((noreturn))
void Reset_Handler(void){
    RCC_CR|=1;while(!(RCC_CR&(1<<2)));
    SCB_CPACR|=((3UL<<20)|(3UL<<22));__asm("dsb");__asm("isb");
    RCC_AHB4ENR|=1;RCC_APB2ENR|=0x10;
    GPIOA->MODER&=~((3U<<18)|(3U<<20));GPIOA->MODER|=((2U<<18)|(2U<<20));
    GPIOA->AFRH&=~((0xFU<<4)|(0xFU<<8));GPIOA->AFRH|=((7U<<4)|(7U<<8));
    USART1->BRR=64000000/115200;USART1->CR1=(1U<<3)|(1U<<2)|(1U<<0);dly_ms(50);
    led_init();
    ups("\n=== AMKN8639 BL V7.2b ===\n");

    uint32_t bkp=RTC_BKP0R;
    if(bkp==BKP_UPGRADE_BL){RTC_BKP0R=0;ups("BL self-upgrade...\n");led_on();int r=do_bl_upgrade();if(r==0){ups("OK! Rebooting...\n");dly_ms(100);*(volatile uint32_t*)0xE000ED0C=0x05FA0004;while(1){}}ups("FAIL!\n");led_off();}

    uint32_t*vt=(uint32_t*)0x08020000;uint32_t asp=vt[0],apc=vt[1];
    int appok=((asp>=0x20000000&&asp<=0x24080000)||(asp<=0x00010000))&&(apc>=0x08020000&&apc<0x08200000)&&(apc&1);
    ups("APP SP=");uph(asp,8);ups(" PC=");uph(apc,8);ups(appok?" OK\n":" INVALID\n");

    int xcp=0;
    if(bkp==BKP_ENTER_XCP){RTC_BKP0R=0;xcp=1;ups("Entered via APP request\n");}
    if(!xcp){ups("Wait 10s for XCP...\n");for(int t=0;t<10000000;t++){if(urdy()){ugc();xcp=1;ups("\nXCP!\n");break;}__asm("nop");if(t%500000==0)GPIOB->ODR^=0x80;}led_off();}
    if(!xcp&&appok){ups("Boot APP...\n");dly_ms(100);boot_app(apc);}if(!appok){ups("APP invalid\n");xcp=1;}
    if(!flash_unlock())ups("Flash OK\n");else ups("Flash LOCKED\n");

    uint8_t pk[256];ups("READY.\n");
    while(1){int pl=recv_pkt(pk,sizeof(pk));if(pl<1)continue;uint8_t c=pk[0];uint8_t*d=pk+1;int dl=pl-1;        switch(c){case 1:if(dl>=4){uint32_t a=*(uint32_t*)d;ups("Erase ");uph(a,8);ups("\n");send_ack(flash_erasector(a)==0?0:1);}else send_ack(0xFF);break;case 2:if(dl>=8&&(dl&3)==0){int ok2=1;uint32_t a2=*(uint32_t*)d;for(int i2=0;i2<(dl-4)/4;i2++){if(flash_wword(a2+i2*4,*(uint32_t*)(d+4+i2*4))!=0){ok2=0;break;}}send_ack(ok2?0:1);}else send_ack(0xFF);break;case 3:send_ack(0);dly_ms(200);flash_lock();if(appok)boot_app(apc);ups("No APP\n");break;case 4:{uint8_t i[]={6,0,(128>>8),128,8,2,0,0,(uint8_t)appok};upc(0xAA);upc(10);upc(0x84);uint8_t ck=0x84;for(int j=0;j<9;j++){upc(i[j]);ck^=i[j];}upc(ck);}break;case 5:ups("LED\n");for(int i=0;i<3;i++){led_on();dly_ms(200);led_off();dly_ms(200);}send_ack(0);break;default:send_ack(0xFE);break;}}
}

void NMI_Handler(void){while(1){}}
void HardFault_Handler(void){while(1){}}
void MemManage_Handler(void){while(1){}}
void BusFault_Handler(void){while(1){}}
void UsageFault_Handler(void){while(1){}}

__attribute__((section(".vectors")))
const uint32_t vectors[]={0x2001FFF0,(uint32_t)Reset_Handler,(uint32_t)NMI_Handler,(uint32_t)HardFault_Handler,(uint32_t)MemManage_Handler,(uint32_t)BusFault_Handler,(uint32_t)UsageFault_Handler};



/* ARM EABI runtime - needed because --no_scanlib */
__asm void __aeabi_memcpy4(void *d, void *s, unsigned int n) {
    PUSH    {r4, lr}
    CMP     r2, #0
    BEQ     done
loop
    LDR     r4, [r1], #4
    STR     r4, [r0], #4
    SUBS    r2, r2, #4
    BNE     loop
done
    POP     {r4, pc}
}

__asm void __aeabi_memcpy(void *d, void *s, unsigned int n) {
    PUSH    {r4, lr}
    CMP     r2, #0
    BEQ     done2
loop2
    LDRB    r4, [r1], #1
    STRB    r4, [r0], #1
    SUBS    r2, r2, #1
    BNE     loop2
done2
    POP     {r4, pc}
}
