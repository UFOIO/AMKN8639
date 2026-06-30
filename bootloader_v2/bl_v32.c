/* AMKN8639 BootLoader V3.2 - Fast boot, LED-only status */
#include <stdint.h>

#define RCC_BASE       0x58024400
#define RCC_CR         (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR    (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR    (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_APB2ENR    (*(volatile uint32_t*)(RCC_BASE+0xF8))
#define FLASH_ACR      (*(volatile uint32_t*)0x52002000)
#define GPIOI_MODER    (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR     (*(volatile uint32_t*)0x58022018)
#define GPIOF_MODER    (*(volatile uint32_t*)0x58021400)
#define GPIOF_OSPEEDR  (*(volatile uint32_t*)0x58021408)
#define GPIOF_PUPDR    (*(volatile uint32_t*)0x5802140C)
#define GPIOF_AFRH     (*(volatile uint32_t*)0x58021424)
#define GPIOF_AFRL     (*(volatile uint32_t*)0x58021420)
#define GPIOG_MODER    (*(volatile uint32_t*)0x58021800)
#define GPIOG_BSRR     (*(volatile uint32_t*)0x58021818)
#define QSPI_CR        (*(volatile uint32_t*)0x52005000)
#define QSPI_DCR       (*(volatile uint32_t*)0x52005004)
#define QSPI_CCR       (*(volatile uint32_t*)0x52005014)
#define QSPI_AR        (*(volatile uint32_t*)0x52005018)
#define QSPI_DR        (*(volatile uint32_t*)0x52005020)
#define QSPI_DLR       (*(volatile uint32_t*)0x52005010)
#define APP_BASE       0x08020000

static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void delay(volatile uint32_t n){ while(n--) __asm("nop"); }

static void led_blip(int ms){ led_on(); delay((uint32_t)ms * 64000); led_off(); }
static void led_blink(int n, int ms){
    int i; for(i=0;i<n;i++){ led_blip(ms); delay((uint32_t)ms * 64000); }
}

__asm void boot_jump(uint32_t sp, uint32_t pc){
    MSR MSP, r0
    BX  r1
}

static int valid_ram(uint32_t a){
    return (a>=0x00000000 && a<0x00010000)||(a>=0x20000000 && a<0x20020000)||
           (a>=0x24000000 && a<0x24080000)||(a>=0x30000000 && a<0x30048000)||
           (a>=0x38000000 && a<0x38010000);
}
static int valid_flash(uint32_t a){ return a>=0x08020000 && a<0x08200000; }

static uint32_t qspi_id(void){
    uint8_t r[3];
    GPIOG_BSRR = (1<<22);
    QSPI_CCR = 0x9F | (1<<8) | (3<<26);
    QSPI_DLR = 2; QSPI_AR = 0; QSPI_CR = 1;
    r[0]=QSPI_DR; r[1]=QSPI_DR; r[2]=QSPI_DR;
    QSPI_CR = 0; GPIOG_BSRR = (1<<6);
    return ((uint32_t)r[0]<<16)|((uint32_t)r[1]<<8)|r[2];
}

__attribute__((noreturn))
void Reset_Handler(void){
    /* HSI + Flash */
    while(!(RCC_CR & (1<<2))){}
    FLASH_ACR = 0x04;

    /* LED init */
    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16); GPIOI_MODER |= (1<<16);

    /* BL START: 3 fast blips */
    led_blink(3, 30);

    /* QSPI + Flash detect */
    RCC_AHB3ENR |= (1<<14);
    RCC_AHB4ENR |= (1<<5)|(1<<6);
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR |= (1<<16)|(1<<18)|(1<<20);
    GPIOF_AFRL &= ~0xFF; GPIOF_AFRL |= (9<<0)|(9<<4);
    GPIOF_AFRH &= ~(0xF<<8); GPIOF_AFRH |= (9<<8);
    GPIOG_MODER &= ~(3<<12); GPIOG_MODER |= (1<<12);
    GPIOG_BSRR = (1<<6);
    QSPI_DCR = (1<<0)|((24-1)<<8);

    uint32_t fid = qspi_id();
    if(fid && fid != 0xFFFFFF){
        led_blink(2, 50);   /* Flash OK */
    } else {
        led_blink(5, 20);   /* Flash FAIL */
        delay(640000);
    }

    /* OTA check */
    uint32_t ota_magic = *(volatile uint32_t*)0x081A0000;
    uint32_t ota_status = *(volatile uint32_t*)0x081A0004;
    if(ota_magic == 0x4F544148 && ota_status == 1){
        /* OTA pending - fast double blink */
        led_blink(4, 30);
    }

    /* APP boot */
    uint32_t sp = *(volatile uint32_t*)APP_BASE;
    uint32_t pc = *(volatile uint32_t*)(APP_BASE + 4);

    if(valid_ram(sp) && valid_flash(pc)){
        led_blip(150);              /* Long blip = booting APP */
        delay(320000);              /* 5ms settle */
        boot_jump(sp, pc);
    }

    /* No APP - slow SOS */
    while(1){
        led_on();  delay(6400000);
        led_off(); delay(6400000);
    }
}

void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1){led_off();delay(2000000);led_on();delay(2000000);}}
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