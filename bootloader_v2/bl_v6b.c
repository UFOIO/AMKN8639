/* AMKN8639 BootLoader V6b - EN=1 always, trigger by AR write */
#include <stdint.h>
#include "flash_map.h"

#define RCC_BASE        0x58024400
#define RCC_CR          (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR     (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR     (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE+0xF0))
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
#define USART1_RDR      (*(volatile uint32_t*)0x40011024)
#define USART1_TDR      (*(volatile uint32_t*)0x40011028)

#define FLASH_KEYR      (*(volatile uint32_t*)0x52002004)
#define FLASH_CR        (*(volatile uint32_t*)0x5200200C)
#define FLASH_SR        (*(volatile uint32_t*)0x52002010)

static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void delay(volatile uint32_t n) { while(n--) __asm("nop"); }
static void led_blip(int ms) { led_on(); delay((uint32_t)ms*64000); led_off(); }
static void led_blink(int n, int ms) { for(int i=0;i<n;i++){led_blip(ms);delay((uint32_t)ms*64000);} }

static void uart_putc(char c) { volatile int t=1000000; while(!(USART1_ISR&(1<<7))&&--t){} USART1_TDR=c; }
static void uart_puts(const char *s) { while(*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v) { static const char h[]="0123456789ABCDEF"; for(int i=28;i>=0;i-=4)uart_putc(h[(v>>i)&0xF]); }
static void uart_print(const char *s, uint32_t v) { uart_puts(s); uart_puthex(v); uart_puts("\r\n"); }
static int uart_rx_ready(void) { return (USART1_ISR&(1<<5))!=0; }
static char uart_getc(void) { while(!uart_rx_ready()){} return (char)USART1_RDR; }
static char rx_buf[256]; static int rx_len;

__asm void boot_jump(uint32_t sp, uint32_t pc) { MSR MSP,r0; BX r1; }
static int valid_ram(uint32_t a) { return (a>=0x00000000&&a<0x00010000)||(a>=0x20000000&&a<0x20020000)||(a>=0x24000000&&a<0x24080000)||(a>=0x30000000&&a<0x30048000)||(a>=0x38000000&&a<0x38010000); }
static int valid_flash(uint32_t a) { return a>=SLOT_A_BASE&&a<(SLOT_A_BASE+SLOT_A_SIZE); }

static uint32_t crc32_table[256];
static void crc32_init(void) { for(uint32_t i=0;i<256;i++){uint32_t c=i;for(int j=0;j<8;j++)c=(c>>1)^((c&1)?0xEDB88320:0);crc32_table[i]=c;} }
static uint32_t crc32_calc(const uint8_t *d,uint32_t n,uint32_t c) { c^=0xFFFFFFFF;for(uint32_t i=0;i<n;i++)c=(c>>8)^crc32_table[(c^d[i])&0xFF];return c^0xFFFFFFFF; }

/* === QSPI: EN=1 at init, never disable === */
static void qspi_init(void) {
    QSPI_CR = 0; delay(100);
    QSPI_FCR = 0x1F; delay(100);

    /* DCR first per STM recommendation */
    QSPI_DCR = (0<<0)|(5<<8)|(22<<16);  /* CKMODE=0, CSHT=5, FSIZE=22 */
    __asm("dsb");

    /* CR with EN=1 enabled from the start */
    QSPI_CR = (3<<24)|(1<<9)|(1<<6)|(1<<0);  /* PRESCALER=3, FTHRES=1, TCEN=1, EN=1 */
    __asm("dsb");
    delay(1000);

    uart_print("  QSPI_CR=", QSPI_CR);
    uart_print("  QSPI_DCR=", QSPI_DCR);
    uart_print("  QSPI_SR=", QSPI_SR);
    uart_print("  AHB3ENR=", RCC_AHB3ENR);
}

/* === QSPI: Wait TCF (timeout ~100ms) === */
static int qspi_wait_tcf(void) {
    volatile uint32_t sr;
    for(volatile int t=0;t<1000000;t++) {
        sr = QSPI_SR;
        if(sr&(1<<1)) return 0;
        if(sr&(1<<0)) { uart_print("TE SR=",sr); QSPI_FCR=(1<<0); return -1; }
    }
    uart_print("TO SR=",QSPI_SR);
    return -2;
}

/* === QSPI: JEDEC ID via AR write trigger === */
static uint32_t qspi_jedec_id(void) {
    uint32_t id=0;

    /* Abort any pending transfer */
    QSPI_CR |= (1<<1);  /* ABORT */
    __asm("dsb");
    delay(100);
    QSPI_FCR = 0x1F;
    __asm("dsb");

    /* Configure CCR for JEDEC ID (0x9F):
       IMODE=single, ADMODE=none, DMODE=single, FMODE=indirect read */
    QSPI_CCR = 0x9F|(1<<8)|(1<<24)|(1<<26);
    __asm("dsb");

    /* Set AR and DLR - AR write triggers xfer when EN=1 and FMODE=indirect read */
    /* IMPORTANT: AR must be written BEFORE DLR on H7! */
    QSPI_AR = 0;
    __asm("dsb");

    /* DLR write as last step */
    *(volatile uint32_t*)0x52005010 = 2;  /* DLR=2 (3 bytes) */
    __asm("dsb");
    __asm("dsb");

    uart_print("  SR=", QSPI_SR);

    if(qspi_wait_tcf()==0) {
        uint8_t b0=(uint8_t)QSPI_DR, b1=(uint8_t)QSPI_DR, b2=(uint8_t)QSPI_DR;
        id=((uint32_t)b0<<16)|((uint32_t)b1<<8)|b2;
    }
    return id;
}

static ota_status_t ota_status;
static int ota_status_read(void) {
    const uint32_t *s=(const uint32_t*)OTA_STATUS_BASE;
    uint32_t *d=(uint32_t*)&ota_status;
    for(int i=0;i<sizeof(ota_status_t)/4;i++)d[i]=s[i];
    uint32_t c=crc32_calc((uint8_t*)&ota_status,sizeof(ota_status_t)-4,0);
    return(ota_status.magic==OTA_MAGIC&&c==ota_status.crc32)?0:-1;
}

static void flash_unlock(void){ FLASH_KEYR=0x45670123;FLASH_KEYR=0xCDEF89AB; }
static void flash_wait(void){ while(FLASH_SR&(1<<0)){} }
static void flash_erase_sector(uint32_t a){ int s=(a-0x08000000)/FLASH_SECTOR_SIZE;FLASH_CR&=~(0xFF<<8);FLASH_CR|=(s<<8)|(2<<0)|(1<<16);flash_wait();FLASH_CR&=~((2<<0)|(1<<16)); }
static void flash_write32(uint32_t a,const uint32_t *d,int n){ FLASH_CR|=(1<<1);flash_wait();volatile uint32_t *p=(volatile uint32_t*)a;for(int i=0;i<n;i++)p[i]=d[i];__asm("dsb 0xF");FLASH_CR|=(1<<16);flash_wait();FLASH_CR&=~((1<<16)|(1<<1)); }

__attribute__((noreturn))
void Reset_Handler(void) {
    while(!(RCC_CR&(1<<2))){}
    FLASH_ACR=0x04;

    RCC_AHB4ENR|=(1<<8);GPIOI_MODER&=~(3<<16);GPIOI_MODER|=(1<<16);
    led_blink(3,30);delay(640000);

    RCC_AHB4ENR|=(1<<0);RCC_APB2ENR|=(1<<4);
    GPIOA_MODER&=~((3<<18)|(3<<20));GPIOA_MODER|=(2<<18)|(2<<20);
    GPIOA_OSPEEDR|=(3<<18)|(3<<20);GPIOA_AFRH&=~(0xFF<<4);GPIOA_AFRH|=(7<<4)|(7<<8);
    USART1_BRR=64000000/115200;USART1_CR1=(1<<3)|(1<<2)|(1<<0);

    uart_puts("\r\n=== AMKN8639 MCUBoot V6b ===\r\n");
    crc32_init();

    /* GPIO: PF8(AF9), PF9(AF9), PF10(AF9), PG6(AF9) */
    RCC_AHB3ENR |= (1<<14);
    RCC_AHB4ENR |= (1<<5)|(1<<6);

    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_PUPDR |= (1<<16)|(1<<18)|(1<<20);
    GPIOF_AFRL &= ~0xFF; GPIOF_AFRL |= (9<<0)|(9<<4);
    GPIOF_AFRH &= ~(0xF<<8); GPIOF_AFRH |= (9<<8);

    GPIOG_MODER &= ~(3<<12); GPIOG_MODER |= (2<<12);
    GPIOG_OSPEEDR |= (3<<12); GPIOG_PUPDR |= (1<<12);
    GPIOG_AFRH &= ~(0xF<<8); GPIOG_AFRH |= (9<<8);

    uart_puts("--- Init ---\r\n");
    qspi_init();

    uint32_t jedec=0;
    uart_puts("--- JEDEC ---\r\n");
    for(int r=0;r<3;r++) {
        uart_puts("Try "); uart_putc('0'+r); uart_puts(": ");
        jedec=qspi_jedec_id();
        uart_puthex(jedec); uart_puts("\r\n");
        if(jedec&&jedec!=0xFFFFFF) break;
        delay(640000);
    }

    if(jedec&&jedec!=0xFFFFFF) {
        uart_puts("Flash OK\r\n"); led_blink(2,50);
    } else {
        uart_puts("Flash FAIL\r\n"); led_blink(5,20); delay(640000);
    }

    uart_puts("OTA: ");
    if(ota_status_read()==0) uart_print("OK ",ota_status.boot_status);
    else uart_puts("none\r\n");

    uint32_t sp=*(volatile uint32_t*)SLOT_A_BASE;
    uint32_t pc=*(volatile uint32_t*)(SLOT_A_BASE+4);
    uart_puts("App SP=");uart_puthex(sp);uart_puts(" PC=");uart_puthex(pc);uart_puts("\r\n");

    if(valid_ram(sp)&&valid_flash(pc)) {
        uart_puts("Hold 100ms...\r\n");delay(6400000);
        uart_puts("Boot APP\r\n");led_blip(150);delay(320000);
        QSPI_CR&=~1;USART1_CR1=0;__asm("dsb");
        boot_jump(sp,pc);
    }
    uart_puts("NO APP\r\n");
    while(1){led_on();delay(6400000);led_off();delay(6400000);}
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){led_off();delay(2000000);led_on();delay(2000000);}}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}
void SysTick_Handler(void){}

__attribute__((section(".vectors")))
const uint32_t vectors[]={0x2001FFC0,(uint32_t)Reset_Handler,(uint32_t)NMI_Handler,(uint32_t)HardFault_Handler,(uint32_t)MemManage_Handler,(uint32_t)BusFault_Handler,(uint32_t)UsageFault_Handler,0,0,0,0,0,0,0,0,(uint32_t)SysTick_Handler};