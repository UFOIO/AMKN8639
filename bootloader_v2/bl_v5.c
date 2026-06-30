/* AMKN8639 BootLoader V5 - Full MCUBoot-style with UART1 AT commands
 * 
 * Features:
 *  - UART1 TX+RX (115200, polling) for RK3568 AT communication
 *  - QSPI W25Q64 detect + read
 *  - OTA status area check + boot decision
 *  - CRC32 for image verification
 *  - LED status indication
 *  - Secure APP jump with stack/PC validation
 */

#include <stdint.h>
#include "flash_map.h"

/* === STM32H743 Register Offsets === */
#define RCC_BASE        0x58024400
#define RCC_CR          (*(volatile uint32_t*)(RCC_BASE+0x00))
#define RCC_AHB4ENR     (*(volatile uint32_t*)(RCC_BASE+0xE0))
#define RCC_AHB3ENR     (*(volatile uint32_t*)(RCC_BASE+0xD4))
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE+0xF0))
#define FLASH_ACR       (*(volatile uint32_t*)0x52002000)

/* GPIO */
#define GPIOF_BSRR     (*(volatile uint32_t*)0x58021418)
#define GPIOF_IDR      (*(volatile uint32_t*)0x58021410)
#define GPIOG_IDR      (*(volatile uint32_t*)0x58021810)
#define GPIOI_MODER     (*(volatile uint32_t*)0x58022000)
#define GPIOI_BSRR      (*(volatile uint32_t*)0x58022018)
#define GPIOF_MODER     (*(volatile uint32_t*)0x58021400)
#define GPIOF_OSPEEDR   (*(volatile uint32_t*)0x58021408)
#define GPIOF_PUPDR     (*(volatile uint32_t*)0x5802140C)
#define GPIOF_AFRH      (*(volatile uint32_t*)0x58021424)
#define GPIOF_AFRL      (*(volatile uint32_t*)0x58021420)
#define GPIOG_MODER     (*(volatile uint32_t*)0x58021800)
#define GPIOG_BSRR      (*(volatile uint32_t*)0x58021818)
#define GPIOA_MODER     (*(volatile uint32_t*)0x58020000)
#define GPIOA_OSPEEDR   (*(volatile uint32_t*)0x58020008)
#define GPIOA_AFRH      (*(volatile uint32_t*)0x58020024)

/* QSPI */
#define QSPI_CR         (*(volatile uint32_t*)0x52005000)
#define QSPI_DCR        (*(volatile uint32_t*)0x52005004)
#define QSPI_CCR        (*(volatile uint32_t*)0x52005014)
#define QSPI_AR         (*(volatile uint32_t*)0x52005018)
#define QSPI_DR         (*(volatile uint32_t*)0x52005020)
#define QSPI_DLR        (*(volatile uint32_t*)0x52005010)

/* USART1 */
#define USART1_BASE     0x40011000
#define USART1_CR1      (*(volatile uint32_t*)(USART1_BASE+0x00))
#define USART1_CR3      (*(volatile uint32_t*)(USART1_BASE+0x08))
#define USART1_BRR      (*(volatile uint32_t*)(USART1_BASE+0x0C))
#define USART1_ISR      (*(volatile uint32_t*)(USART1_BASE+0x1C))
#define USART1_RDR      (*(volatile uint32_t*)(USART1_BASE+0x24))
#define USART1_TDR      (*(volatile uint32_t*)(USART1_BASE+0x28))

/* === LED helpers === */
static void led_on(void)  { GPIOI_BSRR = (1<<8); }
static void led_off(void) { GPIOI_BSRR = (1<<24); }
static void delay(volatile uint32_t n) { while(n--) __asm("nop"); }

static void led_blip(int ms) {
    led_on(); delay((uint32_t)ms * 64000); led_off();
}
static void led_blink(int n, int ms) {
    for(int i = 0; i < n; i++) { led_blip(ms); delay((uint32_t)ms * 64000); }
}

/* === UART1 TX === */
static void uart_wait_txe(void) {
    volatile int timeout = 1000000;
    while(!(USART1_ISR & (1<<7)) && --timeout) {}
}
static void uart_putc(char c) {
    uart_wait_txe();
    USART1_TDR = c;
}
static void uart_puts(const char *s) {
    while(*s) uart_putc(*s++);
}
static void uart_puthex(uint32_t v) {
    static const char hex[] = "0123456789ABCDEF";
    for(int i = 28; i >= 0; i -= 4) uart_putc(hex[(v >> i) & 0xF]);
}

/* === UART1 RX (polling, non-blocking) === */
static int uart_rx_ready(void) {
    return (USART1_ISR & (1<<5)) != 0;  /* RXNE */
}
static char uart_getc(void) {
    while(!uart_rx_ready()) {}
    return (char)USART1_RDR;
}

/* Simple line buffer for AT commands */
static char rx_buf[256];
static int  rx_len;

/* Read a line (blocks until \n or \r) */
static int uart_readline(int timeout_ms) {
    rx_len = 0;
    volatile int t = timeout_ms * 64000;
    while(t--) {
        if(uart_rx_ready()) {
            char c = (char)USART1_RDR;
            if(c == '\r' || c == '\n') {
                if(rx_len > 0) { rx_buf[rx_len] = 0; return rx_len; }
            } else if(rx_len < 255) {
                rx_buf[rx_len++] = c;
            }
        }
    }
    return 0;
}

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

/* === QSPI W25Q64 === */
static uint32_t qspi_cmd_read(uint8_t cmd, int dbytes) {
    uint32_t val = 0;
    GPIOG_BSRR = (1<<22);                   /* CS low */
    QSPI_CCR = cmd | (1<<8) | (3<<26);      /* 1-byte cmd, 3-byte addr disabled initially */
    if(dbytes > 0) QSPI_DLR = dbytes - 1;
    QSPI_AR = 0;
    /* CR already enabled */
    for(int i = 0; i < dbytes; i++) {
        val = (val << 8) | (uint8_t)QSPI_DR;
    }
    QSPI_CR = 0;
    GPIOG_BSRR = (1<<6);                    /* CS high */
    return val;
}

static void qspi_send_cmd(uint8_t cmd) {
    GPIOG_BSRR = (1<<22);
    __asm("dsb");
    QSPI_DLR = 0; QSPI_AR = 0;
    __asm("dsb");
    QSPI_CCR = cmd | (1<<8) | (0<<26) | (3<<24);
    while(!(*(volatile uint32_t*)0x52005008 & 2)) {}
    GPIOG_BSRR = (1<<6);
}static void qspi_reset_flash(void) {
    qspi_send_cmd(0x66); qspi_send_cmd(0x99);
    {volatile int i=3000; while(i--) __asm("nop");}
    qspi_send_cmd(0xAB);
    {volatile int i=3000; while(i--) __asm("nop");}
}
static uint32_t qspi_jedec_id(void) {
    uint32_t id;
    GPIOG_BSRR = (1<<22);  /* CS low */
    __asm("dsb");
    QSPI_DLR = 2;
    QSPI_AR = 0;
    __asm("dsb");
    QSPI_CCR = 0x9F | (1<<8) | (1<<26) | (3<<24);  /* CCR last: IMODE=1line, DMODE=1line */
    while(!(*(volatile uint32_t*)0x52005008 & 2)) {}  /* Wait TCF */
    id  = ((uint32_t)(uint8_t)QSPI_DR) << 16;
    id |= ((uint32_t)(uint8_t)QSPI_DR) << 8;
    id |= (uint8_t)QSPI_DR;
    volatile uint32_t sr = *(volatile uint32_t*)0x52005008;
    uart_puts(" SR=0x"); uart_puthex(sr);
    GPIOG_BSRR = (1<<6);   /* CS high */
    return id;
}static void crc32_init(void) {
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
    for(int i = 0; i < sizeof(ota_status_t)/4; i++) {
        dst[i] = src[i];
    }
    /* Validate CRC32 (exclude crc32 field itself) */
    uint32_t calc = crc32_calc((uint8_t*)&ota_status,
                                sizeof(ota_status_t) - 4, 0);
    return (ota_status.magic == OTA_MAGIC && calc == ota_status.crc32) ? 0 : -1;
}

/* === Main === */
__attribute__((noreturn))
void Reset_Handler(void) {
    /* Clock + Flash */
    while(!(RCC_CR & (1<<2))) {}
    FLASH_ACR = 0x04;

    /* LED init */
    RCC_AHB4ENR |= (1<<8);
    GPIOI_MODER &= ~(3<<16);
    GPIOI_MODER |= (1<<16);

    /* BL START: 3 fast blips */
    led_blink(3, 30);

    /* UART1 init: PA9(TX) PA10(RX) AF7, 115200 */
    RCC_AHB4ENR |= (1<<0);
    RCC_APB2ENR |= (1<<4);
    GPIOA_MODER &= ~((3<<18)|(3<<20));
    GPIOA_MODER |= (2<<18)|(2<<20);
    GPIOA_OSPEEDR |= (3<<18)|(3<<20);
    GPIOA_AFRH &= ~(0xFF<<4);
    GPIOA_AFRH |= (7<<4)|(7<<8);
    USART1_BRR = 64000000 / 115200;
    USART1_CR1 = (1<<3)|(1<<2)|(1<<0);     /* TE + RE + UE */

    uart_puts("\r\n=== AMKN8639 MCUBoot V5 ===\r\n");

    /* CRC32 init */
    crc32_init();

    /* QSPI reg dump */
    volatile uint32_t ahb3enr = *(volatile uint32_t*)(RCC_BASE+0xD4);
    *(volatile uint32_t*)(RCC_BASE+0xD4) = 0x4000;  /* Force QSPI clock */
    uart_puts("RCC_AHB3ENR(after write)=0x"); uart_puthex(*(volatile uint32_t*)(RCC_BASE+0xD4)); uart_puts("\r\n");
    volatile uint32_t ahb3rstr = *(volatile uint32_t*)(RCC_BASE+0xD0);
    uart_puts("RCC_AHB3ENR=0x"); uart_puthex(ahb3enr);
    uart_puts(" AHB3RSTR=0x"); uart_puthex(ahb3rstr);
    uart_puts("\r\n");

        RCC_AHB4ENR |= (1<<5)|(1<<6);  /* GPIOF + GPIOG clock */
    uart_puts("RCC_AHB4ENR=0x"); uart_puthex(RCC_AHB4ENR); uart_puts(" (after set)\r\n");
    uart_puts("GPIOF_MODER=0x"); uart_puthex(GPIOF_MODER); uart_puts("\r\n");
    uart_puts("GPIOG_MODER=0x"); uart_puthex(GPIOG_MODER); uart_puts("\r\n");

    /* GPIO pin test: PF8, PF9, PF10, PG6 */
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));  /* PF8=AF, PF9=AF, PF10=AF */
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR |= (1<<18);                      /* PF9 pull-up */
    GPIOF_AFRH &= ~((0xF<<0)|(0xF<<4)|(0xF<<8));
    GPIOF_AFRH |= (9<<0)|(9<<4)|(9<<8);  /* PF8=AF9, PF9=AF9, PF10=AF9 */

    /* Test PF10 (CLK) toggle */
    uart_puts("GPIO test: PF10 toggle...\r\n");
    volatile uint32_t idr;
    GPIOF_BSRR = (1<<26);  /* PF10 low */
    delay(640);
    idr = GPIOF_IDR;
    uart_puts("PF10=LO IDR=0x"); uart_puthex(idr); uart_puts("\r\n");
    GPIOF_BSRR = (1<<10);  /* PF10 high */
    delay(640);
    idr = GPIOF_IDR;
    uart_puts("PF10=HI IDR=0x"); uart_puthex(idr); uart_puts("\r\n");

    /* Test PF8 (IO0) toggle */
    GPIOF_BSRR = (1<<24);  /* PF8 low */
    delay(640);
    idr = GPIOF_IDR;
    uart_puts("PF8=LO  IDR=0x"); uart_puthex(idr); uart_puts("\r\n");
    GPIOF_BSRR = (1<<8);   /* PF8 high */
    delay(640);
    idr = GPIOF_IDR;
    uart_puts("PF8=HI  IDR=0x"); uart_puthex(idr); uart_puts("\r\n");

    /* Test PG6 (CS) */
    GPIOG_BSRR = (1<<22);  /* PG6 low */
    delay(640);
    idr = GPIOG_IDR;
    uart_puts("PG6=LO  IDR=0x"); uart_puthex(idr); uart_puts("\r\n");
    GPIOG_BSRR = (1<<6);   /* PG6 high */
    delay(640);
    idr = GPIOG_IDR;
    uart_puts("PG6=HI  IDR=0x"); uart_puthex(idr); uart_puts("\r\n");

    /* Read PF9 (MISO) - should be high due to pull-up if not driven */
    idr = GPIOF_IDR;
    uart_puts("PF9(MISO)=0x"); uart_puthex((idr>>9)&1); uart_puts("\r\n");

    /* QSPI W25Q64 detect */
    RCC_AHB3ENR |= (1<<14);  /* QSPI clock via D2 RCC */
    RCC_AHB4ENR |= (1<<5)|(1<<6);
    GPIOF_MODER &= ~((3<<16)|(3<<18)|(3<<20));
    GPIOF_MODER |= ((2<<16)|(2<<18)|(2<<20));
    GPIOF_OSPEEDR |= (3<<16)|(3<<18)|(3<<20);
    GPIOF_PUPDR |= (1<<16)|(1<<18)|(1<<20);
    /* PF6=LCD_BL, PF7=PWM4 - do not modify */
    GPIOF_AFRH = (GPIOF_AFRH & ~((0xF<<4)|(0xF<<8))) | (9<<4)|(9<<8);  /* PF9=AF9, PF10=AF9 */
    GPIOG_MODER &= ~(3<<12);
    GPIOG_MODER |= (1<<12);
    GPIOG_BSRR = (1<<6);                   /* CS high */

    /* QSPI DCR not used for bit-bang */         /* 24-bit address */

    /* Reset flash from QPI to SPI mode */
    qspi_reset_flash();
    uart_puts("Flash reset done.\r\n");

    /* Power-up delay for W25Q64 */
    uart_puts("Waiting for flash power-up...\r\n");
    delay(6400000);  /* ~100ms */
    
    /* Retry JEDEC ID a few times */
    uint32_t jedec = 0;
    for(int retry = 0; retry < 5; retry++) {
        jedec = qspi_jedec_id();
        uart_puts("ret"); uart_puthex(jedec); uart_puts(" "); if(jedec && jedec != 0xFFFFFF) break;
        delay(64000);  /* 1ms */
    }

    if(jedec && jedec != 0xFFFFFF) {
        uart_puts("SPI Flash: W25Q");
        uart_puthex(jedec >> 16);
        uart_puts(" ID=");
        uart_puthex(jedec);
        uart_puts("\r\n");
        led_blink(2, 50);                  /* Flash OK */
    } else {
        uart_puts("SPI Flash: FAIL (retries exhausted)\r\n");
        led_blink(5, 20);                  /* Flash FAIL */
        delay(640000);                     /* 10ms settle */
    }

    /* OTA Status Check */
    uart_puts("OTA Status: ");
    if(ota_status_read() == 0) {
        uart_puts("magic=OK, status=");
        uart_puthex(ota_status.boot_status);
        uart_puts("\r\n");

        if(ota_status.boot_status == OTA_STATUS_DONE) {
            uart_puts("OTA: Pending swap from Slot B\r\n");
            led_blink(4, 30);              /* OTA pending */

            /* TODO Phase B: verify signature, swap B?A */
            /* For now: just boot Slot A */

        } else if(ota_status.boot_status == OTA_STATUS_COPYING ||
                  ota_status.boot_status == OTA_STATUS_VERIFYING) {
            /* Incomplete swap - recover */
            uart_puts("OTA: Incomplete, clearing...\r\n");
            /* Clear magic to force Slot A boot */
            /* (Would need Flash write - skip for now) */
        }
    } else {
        uart_puts("none (clean boot)\r\n");
    }

    /* Boot APP from Slot A */
    uint32_t sp = *(volatile uint32_t*)SLOT_A_BASE;
    uint32_t pc = *(volatile uint32_t*)(SLOT_A_BASE + 4);

    uart_puts("Slot A: SP=");
    uart_puthex(sp);
    uart_puts(" PC=");
    uart_puthex(pc);
    uart_puts("\r\n");

    if(valid_ram(sp) && valid_flash(pc)) {
        /* Wait 500ms for AT command (hold boot) */
        uart_puts("Hold 100ms...\r\n");
        int stay = 0;
        volatile int w = 100 * 64000;  /* ~500ms */
        while(w--) {
            if(uart_rx_ready()) {
                char c = (char)USART1_RDR;
                if(c == '\n' || c == '\r') {
                    if(rx_len > 0) {
                        rx_buf[rx_len] = 0;
                        /* Check for AT+HOLD or similar */
                        uart_puts("RX: ");
                        uart_puts(rx_buf);
                        uart_puts("\r\n");
                        if(rx_buf[0] == 'A' && rx_buf[1] == 'T') {
                            stay = 1;
                            uart_puts("AT cmd: staying in BL\r\n");
                            break;
                        }
                        rx_len = 0;
                    }
                } else if(rx_len < 255) {
                    rx_buf[rx_len++] = c;
                }
            }
        }

        if(stay) {
            /* Stay in BL for AT commands */
            uart_puts("BL console ready.\r\n");
            uart_puts("> ");
            led_on();
            while(1) {
                if(uart_rx_ready()) {
                    char c = (char)USART1_RDR;
                    if(c == '\r' || c == '\n') {
                        if(rx_len > 0) {
                            rx_buf[rx_len] = 0;
                            uart_puts("\r\nCMD: ");
                            uart_puts(rx_buf);
                            uart_puts("\r\n");
                            /* TODO: AT command dispatch */
                            if(rx_buf[0] == 'R' && rx_buf[1] == 0) {
                                uart_puts("Rebooting...\r\n");
                                /* Software reset via AIRCR */
                                volatile uint32_t *aircr = (volatile uint32_t*)0xE000ED0C;
                                *aircr = (0x5FA << 16) | (1 << 2);
                                while(1) {}
                            }
                            rx_len = 0;
                        }
                        uart_puts("> ");
                    } else if(rx_len < 255) {
                        rx_buf[rx_len++] = c;
                        uart_putc(c);  /* echo */
                    }
                }
            }
        }

        /* Normal boot to APP */
        uart_puts("Booting APP @ ");
        uart_puthex(SLOT_A_BASE);
        uart_puts("...\r\n\r\n");
        led_blip(150);
        delay(320000);
        boot_jump(sp, pc);
    }

    /* No valid APP */
    uart_puts("ERROR: No valid APP in Slot A!\r\n");
    while(1) {
        led_on();  delay(6400000);
        led_off(); delay(6400000);
    }
}

/* === Exception Handlers === */
void NMI_Handler(void) { while(1); }
void HardFault_Handler(void) {
    while(1) { led_off(); delay(2000000); led_on(); delay(2000000); }
}
void MemManage_Handler(void) { while(1); }
void BusFault_Handler(void) { while(1); }
void UsageFault_Handler(void) { while(1); }
void SysTick_Handler(void) {}

/* === Vector Table === */
__attribute__((section(".vectors")))
const uint32_t vectors[] = {
    (uint32_t)0x2001FFC0,
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0, 0, 0, 0, 0, 0, 0, 0,
    (uint32_t)SysTick_Handler,
};


