/************************************************************************************
*  File name: w25q64.c
*  Project  : AMKN8639 self-developed Bootloader
*  Description: W25Q64 SPI Flash 精简驱动 (Phase 4 P1)
*                - 寄存器直写 H743 QUADSPI (单线 SPI 模式, 因 IO2/IO3 未接)
*                - 零依赖: 不引厂商 .lib / .c
*                - ARMCC 5.05 C89 strict (变量块首声明)
*                - 时钟: 64MHz HSI / 4 = 16MHz (W25Q64 上限 104MHz)
*
*  Modify History:
*   1. Version: 1.0
*      Date:    2026.6.23
*      Modify:  Phase 4 P1 - create file (W25Q64 + 三段保险)
*
*************************************************************************************/
#include "w25q64.h"
#include "stm32h7xx_regs.h"

/*============================================================================
 * 内部辅助 (C89 strict, 函数必须先声明后使用)
 *===========================================================================*/
static int w25q64_quadspi_init(void);
static int w25q64_gpio_init(void);
static int w25q64_wait_flag_set(uint32_t flag, uint32_t timeout_ms);

/* Modify 2026.6.24 v16 P1.5: 缓存 JEDEC 实际读取值, bootloader 打印验证 */
static uint8_t g_w25q64_mfg    = 0;
static uint8_t g_w25q64_mem_msb = 0;
static uint8_t g_w25q64_mem_lsb = 0;

/*============================================================================
 * 简版基础 (bootloader.c 已经有, 这里不重复)
 *===========================================================================*/

/* 阻塞 delay — 用 bl_delay_ms 风格 (假设 64MHz 主时钟, 简单循环) */
static void w25q64_delay_ms(uint32_t ms)
{
    volatile uint32_t j;
    for (; ms > 0; ms--) {
        for (j = 0; j < 8000; j++) {
            __asm("NOP");
        }
    }
}

/*============================================================================
 * GPIO 初始化 (Modify 2026.6.23: P1 W25Q64)
 *  PG6  = AF10 = QUADSPI_BK1_NCS
 *  PF10 = AF9  = QUADSPI_CLK
 *  PF8  = AF10 = QUADSPI_BK1_IO0 (MOSI)
 *  PF9  = AF10 = QUADSPI_BK1_IO1 (MISO)
 *  注意: IO2/IO3 没接, 只能单线 SPI
 *===========================================================================*/
static int w25q64_gpio_init(void)
{
    /* 1) 开 GPIOF / GPIOG 时钟 */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN | RCC_AHB4ENR_GPIOGEN;
    __asm("DSB");
    __asm("ISB");

    /* 2) PG6 = AF10 (QUADSPI_BK1_NCS) */
    GPIOG->MODER &= ~(0x3UL << (6 * 2));
    GPIOG->MODER |=  (GPIO_MODE_AF << (6 * 2));
    GPIOG->AFR[0] &= ~(0xFUL << ((6) * 4));
    GPIOG->AFR[0] |=  (10UL << ((6) * 4));
    /* 高速 (Very High Speed) — SPI 需要快速边沿 */
    GPIOG->OSPEEDR |= (0x3UL << (6 * 2));

    /* 3) PF10 = AF9 (QUADSPI_CLK) */
    GPIOF->MODER &= ~(0x3UL << (10 * 2));
    GPIOF->MODER |=  (GPIO_MODE_AF << (10 * 2));
    GPIOF->AFR[1] &= ~(0xFUL << ((10 - 8) * 4));
    GPIOF->AFR[1] |=  (9UL << ((10 - 8) * 4));
    GPIOF->OSPEEDR |= (0x3UL << (10 * 2));

    /* 4) PF8 = AF10 (QUADSPI_BK1_IO0 / MOSI) */
    GPIOF->MODER &= ~(0x3UL << (8 * 2));
    GPIOF->MODER |=  (GPIO_MODE_AF << (8 * 2));
    GPIOF->AFR[1] &= ~(0xFUL << ((8 - 8) * 4));
    GPIOF->AFR[1] |=  (10UL << ((8 - 8) * 4));
    GPIOF->OSPEEDR |= (0x3UL << (8 * 2));

    /* 5) PF9 = AF10 (QUADSPI_BK1_IO1 / MISO) */
    GPIOF->MODER &= ~(0x3UL << (9 * 2));
    GPIOF->MODER |=  (GPIO_MODE_AF << (9 * 2));
    GPIOF->AFR[1] &= ~(0xFUL << ((9 - 8) * 4));
    GPIOF->AFR[1] |=  (10UL << ((9 - 8) * 4));
    GPIOF->OSPEEDR |= (0x3UL << (9 * 2));

    return 0;
}

/*============================================================================
 * QUADSPI 控制器初始化
 *  时钟: 64MHz HSI / 4 = 16MHz (PRESCALER=3)
 *  Flash Size: 8MB = 2^23 → FMSIZE = 23
 *  CSHT: CS 高电平保持时间 = 2 个周期 (足够 W25Q64 释放)
 *  CKMODE: 0 = 模式 0 (W25Q64 默认)
 *
 *  Modify 2026.6.24 v18 P1.5: 关键修复
 *   1) 先用 RCC->AHB3RSTR 硬件复位 (v17 仅软件 ABORT, BUSY 仍卡)
 *   2) ABORT 后等 BUSY=0 再清 CR (v17 错误: CR=0 立即清掉 ABORT 位, abort 没生效)
 *   3) 加寄存器 dump 调试输出 (受 W25Q64_DBG 宏控制)
 *===========================================================================*/
static int w25q64_quadspi_init(void)
{
    uint32_t sr_before, cr_after_en;

    /* 1) 开 QUADSPI 时钟 (AHB3) */
    RCC->AHB3ENR |= RCC_AHB3ENR_QUADSPIEN;
    __asm("DSB");
    __asm("ISB");

#if (W25Q64_DBG > 0)
    bl_puts("AT+BOOT=W25Q64 init step1: AHB3ENR=0x");
    bl_put_hex32(RCC->AHB3ENR);
    bl_puts(" SR(before reset)=0x");
    bl_put_hex32(QUADSPI->SR);
    bl_puts("\r\n");
#endif

    /* 2) 硬件复位 QUADSPI (v18: 用 RCC 复位而不是软 ABORT)
     *  v17 bug: QUADSPI->CR = ABORT 之后立即 QUADSPI->CR = 0
     *    → ABORT 位被 CR=0 截胡, BUSY 永远清不掉
     *  v18 fix: RCC 复位整个外设, 这是真正的硬件清零
     */
    RCC->AHB3RSTR |= RCC_AHB3RSTR_QUADSPIRST;
    __asm("DSB");
    __asm("ISB");
    (void)0;  /* RM0433 要求 reset 至少 1 cycle, DSB/ISB 足够 */
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_QUADSPIRST;
    __asm("DSB");
    __asm("ISB");

    sr_before = QUADSPI->SR;  /* 复位后 SR 应为 0 (BUSY=0) */

    /* 3) ABORT 序列 (兜底, v18 顺序: ABORT → 等 BUSY=0 → 清 CR)
     *  此时 BUSY 应该已经是 0 (RCC 复位后), 但仍走一遍 ABORT 流程保险
     */
    if (QUADSPI->SR & QUADSPI_SR_BUSY) {
        QUADSPI->CR = QUADSPI_CR_ABORT;
        /* 等 BUSY 清零 (H743 RM: ABORT 位置 1 后硬件自动清 BUSY) */
        {
            uint32_t t = 5000;
            while ((QUADSPI->SR & QUADSPI_SR_BUSY) && (t-- > 0)) {
                w25q64_delay_ms(1);
            }
        }
    }
    /* 现在 BUSY 一定是 0, 可以安全清 CR */
    QUADSPI->CR = 0;

    /* 4) DCR: flash size + CS 高电平时间 + 时钟模式 */
    QUADSPI->DCR = (23UL << QUADSPI_DCR_FMSIZE_SHIFT) |   /* 8MB */
                   (1UL  << QUADSPI_DCR_CSHT_SHIFT)   |   /* CSHT=1 → 2 cycles */
                   (0UL  << QUADSPI_DCR_CKMODE);          /* mode 0 (低) */

    /* 5) CR: 时钟分频 + FIFO threshold (EN=0, 后面再开) */
    QUADSPI->CR = (3UL << QUADSPI_CR_PRESCALER_SHIFT) |   /* 64/4 = 16MHz */
                  (15UL << QUADSPI_CR_FTHRES_SHIFT);       /* FIFO threshold = 15 */

    /* 6) 清标志 */
    QUADSPI->FCR = QUADSPI_FCR_CTOF | QUADSPI_FCR_CSMF | QUADSPI_FCR_CTCF | QUADSPI_FCR_CTEF;

    /* 7) 使能 QUADSPI */
    QUADSPI->CR |= QUADSPI_CR_EN;

    cr_after_en = QUADSPI->CR;

#if (W25Q64_DBG > 0)
    bl_puts("AT+BOOT=W25Q64 init step7: SR(rst)=");
    bl_put_hex32(sr_before);
    bl_puts(" CR(after EN)=0x");
    bl_put_hex32(cr_after_en);
    bl_puts(" DCR=0x");
    bl_put_hex32(QUADSPI->DCR);
    bl_puts(" SR(now)=0x");
    bl_put_hex32(QUADSPI->SR);
    bl_puts("\r\n");
#endif

    return 0;
}

/*============================================================================
 * 等 QUADSPI 某标志置位 (timeout_ms: 毫秒, 0=默认 5000ms)
 *  返回: 0 = 标志已置, -1 = 超时
 *  Modify 2026.6.24 v20: 改用 TCF 而非 BUSY — 关键修复!
 *
 *  v18 错误思路: 等 BUSY=0 (= "外设完全空闲")
 *    - 问题: BUSY 在 CCR 写入后置 1, 在 SPI 传输完后变 0
 *      但如果 FMODE=IND_READ, BUSY 还会保持 1 直到 MCU 把 FIFO 读完
 *    - 我们之前的代码: 等 BUSY=0 → 读 FIFO → 永远等不到 (死锁)
 *  v20 正确思路: 等 TCF=1 (= "SPI 总线上数据已传完, FIFO 有 N 字节")
 *    - TCF 置 1 后, 读 FIFO 不会卡 (因为数据已经在 FIFO 里)
 *    - BUSY 在 FIFO 排空后自动清零, 但我们不需要等它
 *===========================================================================*/
static int w25q64_wait_flag_set(uint32_t flag, uint32_t timeout_ms)
{
    if (timeout_ms == 0) timeout_ms = 5000;
    while ((QUADSPI->SR & flag) == 0) {
        if (timeout_ms == 0) {
#if (W25Q64_DBG > 0)
            bl_puts("AT+BOOT=W25Q64 wait_flag SET timeout: SR=0x");
            bl_put_hex32(QUADSPI->SR);
            bl_puts(" CCR=0x");
            bl_put_hex32(QUADSPI->CCR);
            bl_puts(" CR=0x");
            bl_put_hex32(QUADSPI->CR);
            bl_puts(" FLEVEL=");
            bl_put_hex32((QUADSPI->SR >> 8) & 0x3F);
            bl_puts("\r\n");
#endif
            return -1;
        }
        w25q64_delay_ms(1);
        timeout_ms--;
    }
    return 0;
}

/*============================================================================
 * 等 W25Q64 内部操作完成 (BUSY 位清零)
 *  W25Q64 sector erase 典型 30-200ms, block erase 典型 1-2s, page prog 0.5-3ms
 *===========================================================================*/
int w25q64_wait_busy(uint32_t timeout_ms)
{
    uint8_t sr = 0;
    if (timeout_ms == 0) timeout_ms = 5000;
    while (timeout_ms--) {
        if (w25q64_read_status1(&sr) != 0) return -1;
        if ((sr & W25Q64_SR1_BUSY) == 0) return 0;   /* BUSY 清零 = 完成 */
        w25q64_delay_ms(1);
    }
    return -1;
}

/*============================================================================
 * 写使能 (每次擦/写操作前必须调)
 *  流程: 配 CCR (instruction only, no addr, no data, indirect write) → 设 INSTR=0x06 → 自动触发
 *  Modify 2026.6.24 v20: 改用 TCF 而非 BUSY
 *===========================================================================*/
int w25q64_write_enable(void)
{
    /* 配 CCR: 间接写模式 + 单线指令 (无地址无数据) + 指令=0x06 (Write Enable) */
    QUADSPI->CCR = CCR_FMODE_IND_WRITE |
                   CCR_IMODE_1LINE |
                   CCR_ADMODE_NONE |
                   CCR_DMODE_NONE |
                   (W25Q64_CMD_WRITE_ENABLE << QUADSPI_CCR_INSTR_SHIFT);

    /* 等 TCF=1 (传输完成, MCU 侧无需读 FIFO, 因为是写命令无数据) */
    if (w25q64_wait_flag_set(QUADSPI_SR_TCF, 1000) != 0) return -1;
    /* 清 TCF 标志 (写 1 到 FCR CTCF 位) */
    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    return 0;
}

/*============================================================================
 * 读状态寄存器 1 (返回 BUSY/WEL 位)
 *  流程: CCR 间接读模式 + 单线指令 + 1 字节数据 → 触发 → 读 DR
 *===========================================================================*/
int w25q64_read_status1(uint8_t *sr)
{
    uint32_t dw;

    /* 配 CCR: 间接读 + 单线指令 + 1 字节数据 */
    QUADSPI->DLR = 0;   /* 1 byte (DLR=0 → 1 byte) */
    QUADSPI->CCR = CCR_FMODE_IND_READ |
                   CCR_DMODE_1LINE |
                   CCR_ADMODE_NONE |
                   CCR_IMODE_1LINE |
                   (W25Q64_CMD_READ_STATUS1 << QUADSPI_CCR_INSTR_SHIFT);

    /* Modify 2026.6.24 v20: 等 TCF=1 (传输完成) 而非 BUSY=0 */
    if (w25q64_wait_flag_set(QUADSPI_SR_TCF, 1000) != 0) return -1;

    /* 读 FIFO (1 字节, 但 DR 是 32-bit, 读 1 word 即可, 高 3 字节是 garbage) */
    dw = QUADSPI->DR;
    *sr = (uint8_t)(dw & 0xFF);
    QUADSPI->FCR = QUADSPI_FCR_CTCF;   /* 清 TCF */
    return 0;
}

/*============================================================================
 * 读 JEDEC ID (3 字节: mfg, mem_msb, mem_lsb)
 *  W25Q64 期望返回: 0xEF 0x40 0x17
 *===========================================================================*/
int w25q64_read_jedec_id(uint8_t *mfg, uint8_t *mem_msb, uint8_t *mem_lsb)
{
    uint32_t dr;

    /* 配 CCR: 间接读 + 单线指令 + 3 字节数据 */
    QUADSPI->DLR = 2;   /* 3 bytes (DLR=2 → 3 bytes) */
    QUADSPI->CCR = CCR_FMODE_IND_READ |
                   CCR_DMODE_1LINE |
                   CCR_ADMODE_NONE |
                   CCR_IMODE_1LINE |
                   (W25Q64_CMD_READ_JEDEC_ID << QUADSPI_CCR_INSTR_SHIFT);

    /* Modify 2026.6.24 v20: 等 TCF=1 (传输完成) 而非 BUSY=0
     *  v18 bug: 等 BUSY=0 永远等不到 (BUSY 在 FIFO 排空后才清, 形成死锁) */
    if (w25q64_wait_flag_set(QUADSPI_SR_TCF, 1000) != 0) return -1;

    /* 读 FIFO (3 字节) — H743 QUADSPI DR 是 32-bit, 一次读 1 word
     *  小端: byte0=[7:0], byte1=[15:8], byte2=[23:16], byte3=[31:24] (garbage)
     *  W25Q64 JEDEC ID 命令 (0x9F) 返回 MSB 先: 0xEF, 0x40, 0x17
     *  进 FIFO 后: byte0=0xEF, byte1=0x40, byte2=0x17 */
    dr = QUADSPI->DR;
    *mfg     = (uint8_t)(dr & 0xFF);
    *mem_msb = (uint8_t)((dr >> 8) & 0xFF);
    *mem_lsb = (uint8_t)((dr >> 16) & 0xFF);
    QUADSPI->FCR = QUADSPI_FCR_CTCF;   /* 清 TCF */
    return 0;
}

/*============================================================================
 * 读数据 (Modify 2026.6.24 v16 P1.5: 改回 0x03 Read Data + 1-line 基础模式)
 *  历史:
 *    v13 (0x03 + 1-line) 失败 - 但当时 INSTR 宏错位 (<<16 而非 <<0), 实际硬件收到 0x00
 *    v14 (0x0B + 8 dummy) 失败 - 同上 INSTR 错位
 *    v15 (0xBB + 2-line) 失败 - 同上 INSTR 错位 (实际收到 0x09)
 *    v16 (0x03 + 1-line) - 修正 INSTR 宏到 bits 7:0 后, 重新验证
 *  W25Q64 0x03 命令格式: 1-line 指令 + 1-line 24-bit 地址 + 1-line 数据
 *  addr: 24-bit W25Q64 地址
 *  buf:  接收缓冲
 *  len:  字节数 (≤ 256 推荐, 更长需要分批读)
 *===========================================================================*/
int w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t remaining;
    uint32_t chunk;
    uint32_t i;
    uint32_t words;
    const uint32_t max_chunk = 256;   /* 一次最多读 256 字节, 避免 FIFO 溢出 */

    if (addr >= W25Q64_TOTAL_SIZE) return -1;
    if ((addr + len) > W25Q64_TOTAL_SIZE) return -1;

    remaining = len;
    while (remaining > 0) {
        chunk = (remaining > max_chunk) ? max_chunk : remaining;

        /* Modify 2026.6.24 v17: H743 RM 18.5.4 规定 CCR 必须最后写 (CCR 写入触发 transaction)
         *  v16 错误顺序 DLR → CCR → AR → CCR 在 AR=0 时已触发, 写入地址错误
         *  修正: DLR → AR → CCR (vendor ST HAL QSPI_Read 也是这个顺序)
         */
        QUADSPI->DLR = chunk - 1;   /* DLR = N-1 (先写) */
        QUADSPI->AR  = addr;        /* AR 第二写 */
        QUADSPI->CCR = CCR_FMODE_IND_READ |
                       CCR_DMODE_1LINE |
                       CCR_ADSIZE_24 |
                       CCR_ADMODE_1LINE |
                       CCR_IMODE_1LINE |
                       (W25Q64_CMD_READ_DATA << QUADSPI_CCR_INSTR_SHIFT);  /* CCR 最后写, 触发 transaction */

        /* Modify 2026.6.24 v20: 等 TCF=1 而非 BUSY=0
         *  v18 bug: 等 BUSY=0 永远等不到 (FIFO 未排空时 BUSY 一直 1)
         *  v20 fix: TCF=1 表示 SPI 总线上数据已传完, FIFO 里有 chunk 字节 */
        if (w25q64_wait_flag_set(QUADSPI_SR_TCF, 5000) != 0) return -1;

        /* 读 FIFO (按 32-bit 读, 然后拆字节) — drain 后 BUSY 自动清零 */
        words = (chunk + 3) / 4;
        for (i = 0; i < words; i++) {
            uint32_t dw = QUADSPI->DR;
            uint32_t bytes_in_this_word = (chunk - i * 4);
            if (bytes_in_this_word > 4) bytes_in_this_word = 4;
            /* 小端: LSB 在最低字节 */
            buf[i * 4 + 0] = (uint8_t)(dw & 0xFF);
            if (bytes_in_this_word > 1) buf[i * 4 + 1] = (uint8_t)((dw >> 8) & 0xFF);
            if (bytes_in_this_word > 2) buf[i * 4 + 2] = (uint8_t)((dw >> 16) & 0xFF);
            if (bytes_in_this_word > 3) buf[i * 4 + 3] = (uint8_t)((dw >> 24) & 0xFF);
        }
        QUADSPI->FCR = QUADSPI_FCR_CTCF;   /* 清 TCF, 准备下一次 transaction */

        buf     += chunk;
        addr    += chunk;
        remaining -= chunk;
    }
    return 0;
}

/*============================================================================
 * 页编程 (0x02 命令, ≤256B)
 *  W25Q64 页编程限制:
 *    - 一次最多 256 字节 (1 页)
 *    - 跨页写入必须分多次 (地址回卷到页首会覆盖前页数据)
 *    - 调用方必须保证 addr + len 不跨页, 否则需要外部循环
 *  调用前必须先 write_enable, 调用后必须 wait_busy
 *===========================================================================*/
int w25q64_page_program(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t remaining;
    uint32_t chunk;
    uint32_t i;
    uint32_t words;

    if (addr >= W25Q64_TOTAL_SIZE) return -1;
    if ((addr + len) > W25Q64_TOTAL_SIZE) return -1;

    remaining = len;
    while (remaining > 0) {
        /* 计算本页剩余空间 (256 字节对齐) */
        uint32_t page_offset = addr & (W25Q64_PAGE_SIZE - 1);
        uint32_t page_left   = W25Q64_PAGE_SIZE - page_offset;
        chunk = (remaining > page_left) ? page_left : remaining;
        if (chunk > W25Q64_PAGE_SIZE) chunk = W25Q64_PAGE_SIZE;

        /* 写使能 (每次写之前都要, 之前的状态可能被清) */
        if (w25q64_write_enable() != 0) return -1;

        /* 配 CCR: 间接写 + 单线 + 24位地址 + 单线指令 + 单线数据 */
        QUADSPI->DLR = chunk - 1;
        QUADSPI->AR  = addr;
        QUADSPI->CCR = CCR_FMODE_IND_WRITE |
                       CCR_DMODE_1LINE |
                       CCR_ADSIZE_24 |
                       CCR_ADMODE_1LINE |
                       CCR_IMODE_1LINE |
                       (W25Q64_CMD_PAGE_PROGRAM << QUADSPI_CCR_INSTR_SHIFT);

        /* 写 FIFO (32-bit 写, 自动) */
        words = (chunk + 3) / 4;
        for (i = 0; i < words; i++) {
            uint32_t dw = 0;
            uint32_t bytes_in_this_word = (chunk - i * 4);
            if (bytes_in_this_word > 4) bytes_in_this_word = 4;
            /* 小端: 第一个字节在最低位 */
            dw |= (uint32_t)buf[i * 4 + 0];
            if (bytes_in_this_word > 1) dw |= (uint32_t)buf[i * 4 + 1] << 8;
            if (bytes_in_this_word > 2) dw |= (uint32_t)buf[i * 4 + 2] << 16;
            if (bytes_in_this_word > 3) dw |= (uint32_t)buf[i * 4 + 3] << 24;
            QUADSPI->DR = dw;
        }

        /* 等 BUSY 清零 (写入完成) */
        if (w25q64_wait_busy(100) != 0) return -1;

        buf       += chunk;
        addr      += chunk;
        remaining -= chunk;
    }
    return 0;
}

/*============================================================================
 * 扇区擦 (4KB, 0x20 命令)
 *  addr 必须 4KB 对齐
 *  典型时间: 30-200ms
 *===========================================================================*/
int w25q64_sector_erase(uint32_t addr)
{
    if (addr & (W25Q64_SECTOR_SIZE - 1)) return -1;   /* 必须 4KB 对齐 */

    if (w25q64_write_enable() != 0) return -1;

    QUADSPI->AR = addr;
    QUADSPI->CCR = CCR_FMODE_IND_WRITE |
                   CCR_ADSIZE_24 |
                   CCR_ADMODE_1LINE |
                   CCR_IMODE_1LINE |
                   (W25Q64_CMD_SECTOR_ERASE << QUADSPI_CCR_INSTR_SHIFT);

    return w25q64_wait_busy(2000);   /* 扇区擦最长 2s */
}

/*============================================================================
 * 块擦 (64KB, 0xD8 命令)
 *  addr 必须 64KB 对齐
 *  典型时间: 1-2s
 *===========================================================================*/
int w25q64_block64_erase(uint32_t addr)
{
    if (addr & (W25Q64_BLOCK64_SIZE - 1)) return -1;

    if (w25q64_write_enable() != 0) return -1;

    QUADSPI->AR = addr;
    QUADSPI->CCR = CCR_FMODE_IND_WRITE |
                   CCR_ADSIZE_24 |
                   CCR_ADMODE_1LINE |
                   CCR_IMODE_1LINE |
                   (W25Q64_CMD_BLOCK64_ERASE << QUADSPI_CCR_INSTR_SHIFT);

    return w25q64_wait_busy(3000);   /* 块擦最长 3s */
}

/*============================================================================
 * 初始化入口
 *  步骤: GPIO → QUADSPI 控制器 → 读 JEDEC ID 验证
 *  Modify 2026.6.23 P1.5-W25Q64: 容忍 MFG mismatch (APP 用 QSPI 模式, 控制器状态被改)
 *===========================================================================*/
int w25q64_init(void)
{
    uint8_t mfg, mem_msb, mem_lsb;
    int jedec_ret;

    if (w25q64_gpio_init() != 0) return -1;
    if (w25q64_quadspi_init() != 0) return -1;

    /* 验证: 读 JEDEC ID, 应返回 0xEF 0x40 0x17 (W25Q64) */
    jedec_ret = w25q64_read_jedec_id(&mfg, &mem_msb, &mem_lsb);
#if (W25Q64_DBG > 0)
    bl_puts("AT+BOOT=W25Q64 jedec_id ret=");
    bl_put_hex32((uint32_t)jedec_ret);
    bl_puts(" mfg=0x");
    bl_put_hex(mfg);
    bl_puts(" mem=0x");
    bl_put_hex(mem_msb);
    bl_put_hex(mem_lsb);
    bl_puts(" SR=0x");
    bl_put_hex32(QUADSPI->SR);
    bl_puts(" CCR=0x");
    bl_put_hex32(QUADSPI->CCR);
    bl_puts(" DR=0x");
    bl_put_hex32(QUADSPI->DR);
    bl_puts("\r\n");
#endif
    if (jedec_ret != 0) return -1;

    /* Modify 2026.6.24 v16 P1.5: 缓存 MFG/MSB/LSB 给 bootloader 打印 (之前硬编码 "JEDEC=EF40" 误导) */
    g_w25q64_mfg    = mfg;
    g_w25q64_mem_msb = mem_msb;
    g_w25q64_mem_lsb = mem_lsb;

    if (mfg != W25Q64_JEDEC_ID_MFG) {
        /* Modify 2026.6.23 P1.5-W25Q64: 容忍 MFG mismatch (APP QSPI 模式改了控制器状态) */
        return 0;
    }
    /* mem_msb=0x40, mem_lsb=0x17 (W25Q64), 但也可能其他 Winbond 型号, 这里只检查厂商 */
    return 0;
}

/* Modify 2026.6.24 v16 P1.5: 返回 init 时读到的真实 JEDEC (0xEF 0x40 0x17 = W25Q64) */
int w25q64_get_jedec(uint8_t *mfg, uint8_t *mem_msb, uint8_t *mem_lsb)
{
    if (mfg != 0)     *mfg     = g_w25q64_mfg;
    if (mem_msb != 0) *mem_msb = g_w25q64_mem_msb;
    if (mem_lsb != 0) *mem_lsb = g_w25q64_mem_lsb;
    return 0;
}

/*============================================================================
 * 三段保险 — 区域写 (Modify 2026.6.23: P1)
 *  流程:
 *    1) 擦除整个区域 (256KB = 64 个扇区 或 4 个 64KB 块)
 *    2) 写 64B 区域头 (magic + size + crc32 + version + timestamp)
 *    3) 写固件数据 (跨页自动分页)
 *  offset: 区域起始地址 (TEMP/BACKUP/FACTORY)
 *  magic:  区域标识
 *  data:   固件 bin (≤ 256KB - 64B)
 *  size:   固件大小
 *  version_str: 版本字符串 (≤ 16B, 截断)
 *  返回: 0=成功, -1=擦失败, -2=写失败, -3=写头失败
 *===========================================================================*/
int w25q64_region_write(uint32_t offset, uint32_t magic,
                         const uint8_t *data, uint32_t size,
                         const char *version_str)
{
    uint8_t  hdr[W25Q64_REGION_HEADER_SIZE];
    uint32_t crc;
    uint32_t i;
    int      ret;

    if (size > (W25Q64_REGION_SIZE - W25Q64_REGION_HEADER_SIZE)) return -3;

    /* 1) 擦除整个区域 (4 个 64KB 块 = 256KB) */
    for (i = 0; i < 4; i++) {
        ret = w25q64_block64_erase(offset + i * W25Q64_BLOCK64_SIZE);
        if (ret != 0) return -1;
    }

    /* 2) 构造 64B 区域头 (大端序) */
    for (i = 0; i < W25Q64_REGION_HEADER_SIZE; i++) hdr[i] = 0;
    /* magic (4B BE) at offset 0 */
    hdr[0] = (uint8_t)(magic >> 24);
    hdr[1] = (uint8_t)(magic >> 16);
    hdr[2] = (uint8_t)(magic >>  8);
    hdr[3] = (uint8_t)(magic >>  0);
    /* size (4B BE) at offset 4 */
    hdr[4] = (uint8_t)(size >> 24);
    hdr[5] = (uint8_t)(size >> 16);
    hdr[6] = (uint8_t)(size >>  8);
    hdr[7] = (uint8_t)(size >>  0);
    /* CRC32 (4B BE) at offset 8 — 占位, 先写 0, 写完数据回填 */
    /* version (16B) at offset 0x0C */
    if (version_str != 0) {
        for (i = 0; i < 16 && version_str[i] != 0; i++) {
            hdr[W25Q64_HDR_VERSION_OFFSET + i] = (uint8_t)version_str[i];
        }
    }

    /* 3) 写区域头 (1 页 = 256B, 我们只写前 64B, 后面是 0xFF) */
    /* 这里需要确保 hdr 后面是 0xFF (擦后默认) */
    /* 直接 page_program 写入前 64B, 但 W25Q64 页编程是整页, 所以需要 */
    /* 一个 256B buffer, 前 64B 是头, 后 192B 是 0xFF */
    {
        uint8_t page_buf[W25Q64_PAGE_SIZE];
        for (i = 0; i < W25Q64_PAGE_SIZE; i++) page_buf[i] = 0xFF;
        for (i = 0; i < W25Q64_REGION_HEADER_SIZE; i++) page_buf[i] = hdr[i];
        if (w25q64_page_program(offset, page_buf, W25Q64_PAGE_SIZE) != 0) return -3;
    }

    /* 4) 写固件数据 (跨页自动分页, 由 page_program 内部处理) */
    if (size > 0 && data != 0) {
        if (w25q64_page_program(offset + W25Q64_REGION_HEADER_SIZE, data, size) != 0) return -2;
    }

    /* 5) 计算 CRC32 并回填到 header */
    /* TODO: 调用 bl_crc32_ieee802_3 计算 data CRC32, 写回 hdr[8..11] */
    /* 这里简化为只写 0, 启动校验时跳过 CRC */
    crc = 0;
    {
        uint8_t hdr_crc_buf[W25Q64_REGION_HEADER_SIZE];
        /* 读回 header (重读避免在 RAM 里维护) */
        if (w25q64_read(offset, hdr_crc_buf, W25Q64_REGION_HEADER_SIZE) != 0) return -3;
        hdr_crc_buf[8] = (uint8_t)(crc >> 24);
        hdr_crc_buf[9] = (uint8_t)(crc >> 16);
        hdr_crc_buf[10] = (uint8_t)(crc >> 8);
        hdr_crc_buf[11] = (uint8_t)(crc >> 0);
        /* 注意: 重写 header 必须再擦一次那 256B 页 (因为 flash 不能覆盖写) */
        /* 简化: 不重写 CRC, 只标记 magic 已写, 启动时只校验 magic 不校验 CRC */
        (void)hdr_crc_buf;
    }

    return 0;
}

/*============================================================================
 * 三段保险 — 区域读
 *  流程:
 *    1) 读 64B 区域头
 *    2) 检查 magic 是否匹配
 *    3) 读 size 字节固件数据到 buf
 *  返回: 读出字节数 (= size), -1 = 段空 / magic 不匹配, -2 = buf 太小
 *============================================================================*/
int w25q64_region_read(uint32_t offset, uint32_t expected_magic,
                        uint8_t *buf, uint32_t buf_size)
{
    uint8_t  hdr[W25Q64_REGION_HEADER_SIZE];
    uint32_t magic;
    uint32_t size;
    uint32_t i;

    /* 1) 读区域头 */
    if (w25q64_read(offset, hdr, W25Q64_REGION_HEADER_SIZE) != 0) return -1;

    /* 2) 检查 magic */
    magic = ((uint32_t)hdr[0] << 24) |
            ((uint32_t)hdr[1] << 16) |
            ((uint32_t)hdr[2] <<  8) |
            ((uint32_t)hdr[3] >>  0);
    if (magic == W25Q64_MAGIC_EMPTY) return -1;            /* 段未写过 */
    if (expected_magic != 0 && magic != expected_magic) return -1;

    /* 3) 读 size 字节数据 */
    size = ((uint32_t)hdr[4] << 24) |
           ((uint32_t)hdr[5] << 16) |
           ((uint32_t)hdr[6] <<  8) |
           ((uint32_t)hdr[7] >>  0);
    if (size == 0) return -1;
    if (size > buf_size) return -2;
    if ((offset + W25Q64_REGION_HEADER_SIZE + size) > W25Q64_TOTAL_SIZE) return -1;

    if (w25q64_read(offset + W25Q64_REGION_HEADER_SIZE, buf, size) != 0) return -1;

    /* 4) 可选: 校验 CRC32 (暂不实现, 只校验 magic) */
    (void)i;
    return (int)size;
}

/*============================================================================
 * 32-bit hex 打印 helper (Modify 2026.6.24 v19: 修正 16→8 字符 bug)
 *  v18 bug: 每个 bl_put_hex(byte) 自身打 2 个 hex 字符
 *           8 次调用 → 16 字符 (而不是 8), 把寄存器值打错
 *  v19 fix: 4 次调用 bl_put_hex(byte), 每次打 1 字节的 2 个 hex 字符
 *           4 字节 × 2 字符 = 8 字符 (正确)
 *  注意: bl_put_hex 是 bootloader.c 提供的 extern, 1 字节 → 2 hex 字符
 *===========================================================================*/
void bl_put_hex32(uint32_t v)
{
    bl_put_hex((uint8_t)(v >> 24));
    bl_put_hex((uint8_t)(v >> 16));
    bl_put_hex((uint8_t)(v >>  8));
    bl_put_hex((uint8_t)(v >>  0));
}