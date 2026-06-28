/*****************************************************************************
 * AMKN8639 Custom Bootloader V2.0
 * Target:  STM32H743XIH6 @ 0x08000000 (128KB), APP @ 0x08020000
 * UART1:   PA9(TX) PA10(RX) 115200 8N1
 *
 * Flow: wait 3s for 'C' -> YMODEM receive -> program Flash -> jump APP
 *****************************************************************************/
#include <stdint.h>

/* ---- Memory ---- */
#define APP_BASE             0x08020000
#define BOOT_TIMEOUT_MS      3000
#define FLASH_SECTOR_SIZE    (128 * 1024)

/* ---- STM32H743 Registers ---- */
typedef struct { volatile uint32_t CR, ICSCR, CFGR, CIER, CIFR, CICR, R0, R1, AHB3RSTR, AHB3ENR; } RCC_Type;
typedef struct { volatile uint32_t ACR, KEYR, OPTKEYR, CR, SR, CCR, OPTCR, OPTCR1; } FLASH_Type;
typedef struct { volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH; } GPIO_Type;
typedef struct { volatile uint32_t CR1, CR2, CR3, BRR, GTPR, RTOR, RQR, ISR, ICR, RDR, TDR; } USART_Type;
typedef struct { volatile uint32_t CSR, RVR, CVR, CALIB; } SYSTICK_Type;
typedef struct { uint32_t R0[8]; volatile uint32_t ICSR, VTOR; } SCB_Type;

#define RCC     ((RCC_Type     *)0x58024400)
#define FLASH   ((FLASH_Type   *)0x52002000)
#define GPIOA   ((GPIO_Type    *)0x58020000)
#define USART1  ((USART_Type   *)0x40011000)
#define SYST    ((SYSTICK_Type *)0xE000E010)
#define SCB     ((SCB_Type     *)0xE000ED00)

/* ---- Globals ---- */
static volatile uint32_t g_msTick;
static uint8_t g_pktBuf[1030];

/* ---- UART helpers ---- */
static void uart_putc(char c) {
    while (!(USART1->ISR & (1 << 7)));
    USART1->TDR = c;
}
static void uart_puts(const char *s) { while (*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v, int n) {
    int i; static const char h[] = "0123456789ABCDEF";
    for (i = n - 1; i >= 0; i--) uart_putc(h[(v >> (i * 4)) & 0xF]);
}
static int uart_readable(void) { return (USART1->ISR & (1 << 5)) ? 1 : 0; }
static uint8_t uart_getc(void) { while (!uart_readable()); return (uint8_t)USART1->RDR; }
static void uart_flush(void) { while (uart_readable()) (void)USART1->RDR; }

/* ---- SysTick 1ms ---- */
void SysTick_Handler(void) { g_msTick++; }
static void systick_init(void) {
    SYST->RVR = 63999; SYST->CVR = 0; SYST->CSR = 7;
}
static uint32_t millis(void) { return g_msTick; }
static void delay_ms(uint32_t ms) {
    uint32_t s = millis(); while ((millis() - s) < ms) { __asm { nop } }
}

/* ---- UART init ---- */
static void uart_init(void) {
    int i;
    RCC->AHB3ENR |= (1 << 0) | (1 << 4);
    /* PA9 AF7 TX (AFRH bits 4-7), PA10 AF7 RX (AFRH bits 8-11) */
    GPIOA->MODER  &= ~((3 << 18) | (3 << 20));
    GPIOA->MODER  |=  ((2 << 18) | (2 << 20));
    GPIOA->AFRH   &= ~((0xF << 4) | (0xF << 8));
    GPIOA->AFRH   |=  ((7 << 4) | (7 << 8));
    USART1->BRR = 64000000 / 115200;
    USART1->CR1 = (1 << 3) | (1 << 2) | (1 << 0);
    for (i = 0; i < 10000; i++) __asm { nop }
}

/* ---- CRC16-XMODEM ---- */
static uint16_t crc16(const uint8_t *d, int len) {
    uint16_t crc = 0; int i, j;
    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)d[i] << 8;
        for (j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
    }
    return crc;
}

/* ---- Flash ops ---- */
static void flash_unlock(void) {
    if (FLASH->CR & 1) { FLASH->KEYR = 0x45670123; FLASH->KEYR = 0xCDEF89AB; }
}
static void flash_wait(void) { while (FLASH->SR & (1 << 0)); }

static void flash_erase_sector(int sec) {
    FLASH->CR &= ~(0xFF << 8);
    FLASH->CR |= (sec << 8) | (2 << 0) | (1 << 16);
    flash_wait();
    FLASH->CR &= ~((2 << 0) | (1 << 16));
}

static void flash_write32(uint32_t addr, const uint8_t *d) {
    int i; volatile uint32_t *dst = (volatile uint32_t *)addr;
    FLASH->CR |= (1 << 1);
    flash_wait();
    for (i = 0; i < 8; i++) {
        dst[i] = ((uint32_t)d[0]) | ((uint32_t)d[1] << 8) |
                 ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24);
        d += 4;
    }
    __asm { dsb 0xF }
    FLASH->CR |= (1 << 16);
    flash_wait();
    FLASH->CR &= ~((1 << 16) | (1 << 1));
}

__asm void _set_msp(uint32_t sp_val) {
    MSR MSP, r0
    BX lr
}
/* ---- Jump to APP ---- */
static void jump_to_app(uint32_t addr) {
    uint32_t *vt = (uint32_t *)addr;
    uint32_t sp = vt[0], pc = vt[1];
    if (sp < 0x20000000 || sp > 0x24080000 || pc < 0x08020000 || pc > 0x08200000) {
        uart_puts("BAD IMG\r\n"); return;
    }
    USART1->CR1 = 0; SYST->CSR = 0;
    SCB->ICSR = (1 << 2);
    SCB->VTOR = addr;
    { uint32_t _sp = sp; __asm { MSR MSP, _sp } }
    ((void(*)(void))pc)();
}

/* ---- YMODEM receive ---- */
static int ymodem_receive(uint32_t *out_sz) {
    uint32_t fw_addr = APP_BASE, total = 0;
    int block = 0, sector = -1, retry;
    int i, j, hdr, pkt_size;
    uint8_t blk, blk_inv;
    uint16_t calc, recv;
    uint32_t fsize;

    uart_puts("YMODEM_READY\r\n");
    uart_flush(); delay_ms(100); uart_putc('C');

    for (;;) {
        hdr = -1; { uint32_t t0 = millis();
        while ((millis() - t0) < 10000) { if (uart_readable()) { hdr = uart_getc(); break; } }
        }
        if (hdr < 0) { uart_puts("TIMEOUT\r\n"); return -1; }
        if (hdr == 0x04) { uart_putc(0x06); uart_puts("EOT\r\n"); break; }
        if (hdr == 0x18) { uart_puts("CAN\r\n"); return -1; }

        pkt_size = (hdr == 0x01) ? 128 : ((hdr == 0x02) ? 1024 : 0);
        if (!pkt_size) { uart_putc(0x15); continue; }

        g_pktBuf[0] = (uint8_t)hdr;
        for (i = 1; i < 3 + pkt_size + 2; i++) {
            uint32_t t0 = millis();
            while (!uart_readable()) { if ((millis() - t0) > 5000) { uart_putc(0x15); goto skip; } }
            g_pktBuf[i] = uart_getc();
        }

        blk = g_pktBuf[1]; blk_inv = g_pktBuf[2];
        if (blk != (block & 0xFF) || (blk ^ blk_inv) != 0xFF) { uart_putc(0x15); continue; }

        calc = crc16(g_pktBuf + 3, pkt_size);
        recv = ((uint16_t)g_pktBuf[3 + pkt_size] << 8) | g_pktBuf[3 + pkt_size + 1];
        if (calc != recv) { uart_putc(0x15); continue; }

        /* Block 0: header */
        if (block == 0) {
            fsize = 0;
            for (i = 0; i < pkt_size && g_pktBuf[3 + i] != 0; i++);
            if (i < pkt_size) {
                char *sz = (char *)(g_pktBuf + 3 + i + 1);
                for (j = 0; sz[j] >= '0' && sz[j] <= '9'; j++)
                    fsize = fsize * 10 + (sz[j] - '0');
            }
            *out_sz = fsize;
            uart_puts("SIZE="); uart_puthex(fsize, 8); uart_puts("\r\n");
        }

        /* Erase sector on first data block crossing boundary */
        if (block > 0) {
            int sec = (fw_addr - APP_BASE) / FLASH_SECTOR_SIZE + 1;
            if (sec != sector) {
                sector = sec;
                uart_puts("ERASE "); uart_puthex(sec, 2); uart_puts("\r\n");
                flash_erase_sector(sec);
            }
            /* Write in 32-byte chunks */
            { const uint8_t *p = g_pktBuf + 3; int rem = pkt_size;
            while (rem >= 32) {
                flash_write32(fw_addr, p);
                fw_addr += 32; p += 32; rem -= 32; total += 32;
            }}
        }

        uart_putc(0x06);
        block++;
skip: ;
    }

    uart_puts("DONE="); uart_puthex(total, 8); uart_puts("\r\n");
    return total;
}

/* ====== RESET Handler ====== */
__attribute__((noreturn))
void Reset_Handler(void) {
    uint32_t t0, fw_size;

    /* Enable HSI 64MHz */
    RCC->CR |= (1 << 0);
    while (!(RCC->CR & (1 << 2)));
    RCC->CFGR = 0;

    /* PWR VOS scale 1 */
    *(volatile uint32_t *)(0x58024800) |= (1 << 14);

    FLASH->ACR = 0x04;

    systick_init();
    uart_init();
    delay_ms(50);

    uart_puts("\r\nAMKN8639 Boot V2.0\r\n");

    /* Wait for 'C' */
    t0 = millis(); { int got = 0;
    while ((millis() - t0) < BOOT_TIMEOUT_MS) {
        if (uart_readable()) { uint8_t c = uart_getc(); if (c == 'C' || c == 'y' || c == 'Y') { got = 1; break; } }
    }
    if (got) {
        uart_puts("YMODEM\r\n");
        flash_unlock();
        if (ymodem_receive(&fw_size) > 0) {
            uart_puts("BOOT APP...\r\n");
            delay_ms(200);
            jump_to_app(APP_BASE);
        }
        uart_puts("FAIL\r\n");
    }}

    uart_puts("JUMP APP\r\n");
    jump_to_app(APP_BASE);
    while (1) { __asm { wfi } }
}

/* ---- Vector table ---- */
__attribute__((section(".vectors")))
const uint32_t vectors[] = {
    (uint32_t)0x20020000,
    (uint32_t)Reset_Handler,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    (uint32_t)SysTick_Handler,
};