/* AMKN8639 BootLoader V11 - Fix: no EN during init, DCYC=0 for JEDEC
 *
 * Key fixes vs V10:
 *  1. Init does NOT enable QSPI (avoids BUSY stuck in indirect write mode)
 *  2. DCYC=0 for JEDEC ID (ADMODE=none, DCYC bug may not apply)
 *  3. BUSY check uses bit 2 (not bit 5)
 *  4. ABORT sequence before each transfer
 */

#include <stdint.h>
#include "flash_map.h"

/* === STM32H743 Registers === */
#define RCC_BASE        0x58024400
#define RCC_CR          (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR     (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR     (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_AHB3RSTR    (*(volatile uint32_t*)(RCC_BASE+0xE4))
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

/* === LED helpers === */
static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void delay(volatile uint32_t n) { while(n--) __asm("nop"); }
static void led_blip(int ms) { led_on(); delay((uint32_t)ms * 64000); led_off(); }
static void led_blink(int n, int ms) {
    for(int i = 0; i < n; i++) { led_blip(ms); delay((uint32_t)ms * 64000); }
}

/* === UART1 === */
static void uart_putc(char c) {
    volatile int to = 1000000;
    while(!(USART1_ISR & (1<<7)) && --to) {}
    USART1_TDR = c;
}
static void uart_puts(const char *s) { while(*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v) {
    static const char h[] = "0123456789ABCDEF";
    for(int i = 28; i >= 0; i -= 4) uart_putc(h[(v >> i) & 0xF]);
}
static void uart_print(const char *s, uint32_t v) {
    uart_puts(s); uart_puthex(v); uart_puts("\r\n");
}
static int uart_rx_ready(void) { return (USART1_ISR & (1<<5)) != 0; }
static char rx_buf[256];
static int  rx_len;

/* === APP Jump === */
__asm void boot_jump(uint32_t sp, uint32_t pc) {
    MSR MSP, r0
    BX  r1
}
static int valid_ram(uint32_t a) {
    return (a >= 0x00000000 && a < 0x00010000) ||
           (a >= 0x20000000 && a < 0x20020000) ||
           (a >= 0x24000000 && a < 0x24080000) ||
           (a >= 0x30000000 && a < 0x30048000) ||
           (a >= 0x38000000 && a < 0x38010000);
}
static int valid_flash(uint32_t a) {
    return a >= SLOT_A_BASE && a < (SLOT_A_BASE + SLOT_A_SIZE);
}

/* === CRC32 === */
static uint32_t crc32_table[256];
static void crc32_init(void) {
    for(uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for(int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        crc32_table[i] = crc;
    }
}
static uint32_t crc32_calc(const uint8_t *data, uint32_t len, uint32_t crc) {
    crc ^= 0xFFFFFFFF;
    for(uint32_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

/* === OTA Status === */
static ota_status_t ota_status;
static int ota_status_read(void) {
    const uint32_t *src = (const uint32_t*)OTA_STATUS_BASE;
    uint32_t *dst = (uint32_t*)&ota_status;
    for(int i = 0; i < sizeof(ota_status_t)/4; i++) dst[i] = src[i];
    uint32_t calc = crc32_calc((uint8_t*)&ota_status, sizeof(ota_status_t)-4, 0);
    return (ota_status.magic == OTA_MAGIC && calc == ota_status.crc32) ? 0 : -1;
}

/* === Internal Flash === */
static void flash_unlock(void) {
    FLASH_KEYR = 0x45670123; FLASH_KEYR = 0xCDEF89AB;
}
static void flash_wait(void) { while(FLASH_SR & (1<<0)) {} }
static void flash_erase_sector(uint32_t addr) {
    int sec = (addr - 0x08000000) / FLASH_SECTOR_SIZE;
    FLASH_CR &= ~(0xFF << 8);
    FLASH_CR |= (sec << 8) | (2 << 0) | (1 << 16);
    flash_wait();
    FLASH_CR &= ~((2 << 0) | (1 << 16));
}
static void flash_write32(uint32_t addr, const uint32_t *d, int n) {
    FLASH_CR |= (1 << 1); flash_wait();
    volatile uint32_t *p = (volatile uint32_t *)addr;
    for(int i = 0; i < n; i++) p[i] = d[i];
    __asm("dsb 0xF");
    FLASH_CR |= (1 << 16); flash_wait();
    FLASH_CR &= ~((1 << 16) | (1 << 1));
}

/* === QSPI: Abort ongoing operation === */
static void qspi_abort(void) {
    uint32_t sr = QSPI_SR;
    if(!(sr & (1<<2))) return;  /* BUSY=0, nothing to abort */
    
    uart_print("  ABORT needed, SR=", sr);
    QSPI_CR |= (1<<1);  /* Set ABORT bit */
    __asm("dsb");
    volatile int to = 1000000;
    while((QSPI_CR & (1<<1)) && --to) {}  /* Wait for ABORT to self-clear */
    if(to <= 0) uart_puts("  ABORT timeout\r\n");
    
    QSPI_FCR = 0x1F;  /* Clear all flags */
    __asm("dsb");
    uart_print("  SR(after ABORT)=", QSPI_SR);
}

/* === QSPI RCC Reset === */
static void qspi_rcc_reset(void) {
    uart_puts("QSPI RCC reset...\r\n");
    /* V15: Full RCC reset instead of simple clock enable */
    RCC_AHB3ENR |= (1<<14); __asm("dsb"); delay(10);
    RCC_AHB3RSTR |= (1<<14); __asm("dsb"); delay(100);
    RCC_AHB3ENR &= ~(1<<14); __asm("dsb"); delay(1000);
    RCC_AHB3ENR |= (1<<14); __asm("dsb"); delay(100);
    RCC_AHB3RSTR &= ~(1<<14); __asm("dsb"); delay(1000);
    uart_print("AHB3ENR(rst)=", RCC_AHB3ENR);
    __asm("dsb"); delay(100);
    
    RCC_AHB3RSTR |= (1<<14);
    __asm("dsb"); delay(1000);
    
    RCC_AHB3RSTR &= ~(1<<14);
    __asm("dsb"); delay(1000);
}

/* === QSPI Init: configure but do NOT enable === */
static void qspi_init(void) {
    uart_puts("--- QSPI Init (EN stays 0) ---\r\n");
    
    /* Full reset: disable + clear */
    QSPI_CR = 0;
    __asm("dsb");
    QSPI_FCR = 0x1F;
    __asm("dsb");
    
    /* DCR: CKMODE=0, CSHT=2, FSIZE=22 (8MB for W25Q64) */
    QSPI_DCR = (0 << 0) | (2 << 8) | (22 << 16);
    __asm("dsb");
    
    /* CR: PRESCALER=3(16MHz), FTHRES=1, SSHIFT=1, TCEN=1, EN=0 */
    QSPI_CR = (3 << 24) | (1 << 9) | (1 << 4) | (1 << 6);
    __asm("dsb");
    
    uart_print("  DCR=", QSPI_DCR);
    uart_print("  CR(EN=0)=", QSPI_CR);
    uart_print("  SR=", QSPI_SR);
}

/* === QSPI: JEDEC ID read === */
static uint32_t qspi_jedec_id(void) {
    uint32_t id = 0;
    
    for(int retry = 0; retry < 3; retry++) {
        uart_puts("  JEDEC retry "); uart_putc('0'+retry); uart_puts(":\r\n");
        
        /* Abort any stuck operation first */
        qspi_abort();
        
        /* Disable, clear flags */
        QSPI_CR &= ~1;
        __asm("dsb"); delay(10);
        QSPI_FCR = 0x1F;
        __asm("dsb");
        
        /* CCR: JEDEC ID (0x9F), IMODE=1line, ADMODE=none, 
         * DCYC=0 (no dummy for JEDEC ID), DMODE=1line, FMODE=indirect read */
        QSPI_CCR = 0x9F
                 | (1 << 8)       /* IMODE = single */
                 | (0 << 10)      /* ADMODE = none */
                 | (0 << 18)      /* DCYC = 0 */
                 | (1 << 24)      /* DMODE = single */
                 | (1 << 26);     /* FMODE = indirect read */
        
        QSPI_AR = 0;
        QSPI_DLR = 2;  /* 3 bytes total */
        __asm("dsb");
        
        uart_print("    CCR=", QSPI_CCR);
        uart_print("    SR_pre=", QSPI_SR);
        
        /* Enable QSPI ? transfer starts NOW */
        QSPI_CR |= 1;
        __asm("dsb");
        
        uart_print("    SR_en=", QSPI_SR);
        
        /* Wait for TCF (bit 1) */
        volatile int to = 5000000;
        while(--to) {
            uint32_t sr = QSPI_SR;
            if(sr & (1<<1)) {   /* TCF */
                uint8_t b0 = (uint8_t)QSPI_DR;
                uint8_t b1 = (uint8_t)QSPI_DR;
                uint8_t b2 = (uint8_t)QSPI_DR;
                id = ((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | b2;
                uart_puts("    TCF! ID="); uart_puthex(id); uart_puts("\r\n");
                QSPI_CR &= ~1;
                return id;
            }
            if(sr & (1<<0)) {   /* TE */
                uart_print("    TE SR=", sr);
                QSPI_FCR = 0x1F;
                break;
            }
            if(sr & (1<<2)) {   /* BUSY - just wait */
                continue;
            }
        }
        uart_print("    TO SR=", QSPI_SR);
        QSPI_CR &= ~1;
        delay(640000);
    }
    return id;
}

/* ====== MAIN ====== */
__attribute__((noreturn))
void Reset_Handler(void) {
    while(!(RCC_CR & (1<<2))) {}
    FLASH_ACR = 0x04;

    /* LED init */
    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16); GPIOI_MODER |= (1<<16);
    led_blink(2, 30); delay(640000);

    /* UART1 init */
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);
    GPIOA_MODER &= ~((3<<18)|(3<<20));
    GPIOA_MODER |= (2<<18)|(2<<20);
    GPIOA_OSPEEDR |= (3<<18)|(3<<20);
    GPIOA_AFRH &= ~(0xFF<<4);
    GPIOA_AFRH |= (7<<4)|(7<<8);
    USART1_BRR = 64000000 / 115200;
    USART1_CR1 = (1<<3)|(1<<2)|(1<<0);

    uart_puts("\r\n=== AMKN8639 MCUBoot V15 (Force-HI+RCC_RST) ===\r\n");
    crc32_init();

    /* === QSPI === */
    RCC_AHB4ENR |= (1<<5)|(1<<6);  /* GPIOF + GPIOG */
    
    /* RCC reset QSPI */
    qspi_rcc_reset();
    uart_print("AHB3ENR=", RCC_AHB3ENR);
    
    /* GPIO: PF8(AF9), PF9(AF9), PF10(AF9), PG6(AF9) */
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_PUPDR |= (1<<16)|(1<<18)|(1<<20);
    GPIOF_AFRL &= ~0xFF; GPIOF_AFRL |= (9<<0)|(9<<4);
    GPIOF_AFRH &= ~(0xF<<8); GPIOF_AFRH |= (9<<8);

    GPIOG_MODER &= ~(3<<12);
    GPIOG_MODER |= (2<<12);
    GPIOG_OSPEEDR |= (3<<12);
    GPIOG_PUPDR &= ~(3<<12);
    GPIOG_PUPDR |= (1<<12);
    GPIOG_AFRH &= ~(0xF<<8); GPIOG_AFRH |= (9<<8);
    
    /* Init QSPI (no EN) */
    qspi_init();
    
    /* JEDEC */
    uart_puts("--- JEDEC Scan ---\r\n");
    uint32_t jedec = qspi_jedec_id();
    
    if(jedec && jedec != 0xFFFFFF) {
        uart_puts("SPI Flash: OK ID="); uart_puthex(jedec); uart_puts("\r\n");
        led_blink(2, 50);
    } else {
        uart_puts("SPI Flash: FAIL\r\n");
        led_blink(5, 20); delay(640000);
    }

    /* OTA check */
    uart_puts("OTA Status: ");
    if(ota_status_read() == 0) {
        uart_print("magic OK status=", ota_status.boot_status);
    } else {
        uart_puts("none (clean boot)\r\n");
    }

    /* Boot APP */
    uint32_t sp = *(volatile uint32_t*)SLOT_A_BASE;
    uint32_t pc = *(volatile uint32_t*)(SLOT_A_BASE + 4);
    uart_puts("Slot A: SP="); uart_puthex(sp);
    uart_puts(" PC="); uart_puthex(pc); uart_puts("\r\n");

    if(valid_ram(sp) && valid_flash(pc)) {
        uart_puts("Hold 100ms...\r\n");
        int stay = 0; rx_len = 0;
        volatile int w = 100 * 64000;
        while(w--) {
            if(uart_rx_ready()) {
                char c = (char)USART1_RDR;
                if(c == '\r' || c == '\n') {
                    if(rx_len > 0) {
                        rx_buf[rx_len] = 0;
                        if(rx_buf[0] == 'A' && rx_buf[1] == 'T') { stay = 1; break; }
                        rx_len = 0;
                    }
                } else if(rx_len < 255) rx_buf[rx_len++] = c;
            }
        }
        if(stay) {
            uart_puts("BL console ready.\r\n> ");
            while(1) {
                if(uart_rx_ready()) {
                    char c = (char)USART1_RDR;
                    if(c == '\r' || c == '\n') {
                        if(rx_len > 0) {
                            rx_buf[rx_len] = 0;
                            uart_puts("\r\nCMD: "); uart_puts(rx_buf); uart_puts("\r\n");
                            if(rx_buf[0] == 'R') {
                                volatile uint32_t *aircr = (volatile uint32_t*)0xE000ED0C;
                                *aircr = (0x5FA << 16) | (1 << 2);
                                while(1) {}
                            }
                            rx_len = 0;
                        }
                        uart_puts("> ");
                    } else if(rx_len < 255) { rx_buf[rx_len++] = c; uart_putc(c); }
                }
            }
        }
        uart_puts("Booting APP @ 08020000...\r\n\r\n");
        led_blip(150); delay(320000);
        QSPI_CR &= ~1; USART1_CR1 = 0; __asm("dsb");
        boot_jump(sp, pc);
    }
    uart_puts("ERROR: No valid APP!\r\n");
    while(1) { led_on(); delay(6400000); led_off(); delay(6400000); }
}

/* === Vector Table === */
void NMI_Handler(void) { while(1); }
void HardFault_Handler(void) { while(1){led_off();delay(2000000);led_on();delay(2000000);} }
void MemManage_Handler(void) { while(1); }
void BusFault_Handler(void) { while(1); }
void UsageFault_Handler(void) { while(1); }
void SysTick_Handler(void) {}

__attribute__((section(".vectors")))
const uint32_t vectors[] = {
    (uint32_t)0x2001FFC0, (uint32_t)Reset_Handler, (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler, (uint32_t)MemManage_Handler, (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler, 0,0,0,0,0,0,0,0, (uint32_t)SysTick_Handler,
};
