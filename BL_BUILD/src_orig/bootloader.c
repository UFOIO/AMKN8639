/************************************************************************************
*  File name: bootloader.c
*  Project  : AMKN8639 self-developed Bootloader
*  Description: BL 主程序 (Phase 3.2 + DeepSeek 大胆版 v1)
*                - 全部寄存器直写, 零 .lib 依赖
*                - DI1 长按 / EEPROM 升级标志 触发升级模式
*                - UART1 115200 自定义协议 (6 命令)
*                - 跳 APP: SCB->VTOR = 0x08010000, 设 MSP, 跳 reset
*
*  ARMCC 5.05 是 C89 严格模式, 变量必须块首声明
*
*bootlkoader编译+烧录
*
*编译：先进文件在编译
cd "C:\Users\gjt\Desktop\项目工程\2026内蒙轻型机库项目\软件\机库工控机\机库电设文档\AMKNxxxx_FreeRTOS_V1.31.02_20260501\targets\AMKN8639\Boot"

python compile_bl.py

*编译+烧录：
cd "C:\Users\gjt\Desktop\项目工程\2026内蒙轻型机库项目\软件\机库工控机\机库电设文档\AMKNxxxx_FreeRTOS_V1.31.02_20260501\targets\AMKN8639\Boot"
python build_bl.py

编译烧录：不加--flash就是编译
cd "C:/Users/gjt/Desktop/项目工程/2026内蒙轻型机库项目/软件/机库工控机/机库电设文档/AMKNxxxx_FreeRTOS_V1.31.02_20260501/targets/AMKN8639/Boot"
python build_app.py --skip-build --flash

*************************************************************************************/
#include "bootloader.h"
#include "stm32h7xx_regs.h"
#include "w25q64.h"

/*============================================================================
 * v30: W25Q64 OTA 布局 (与 APP config/AMKN8639_Config.h 对齐, BL 不引 config.h)
 *  BL 单独编译, 必须自包含 W25Q64 OTA 所需常量.
 *===========================================================================*/
#define W25QXX_SECTOR_SIZE             4096UL                /* W25Q64 扇区大小 (4KB) */
#define W25Q64_OTA_MAGIC_ADDR          0x002C0000UL          /* gap sector 704, 避开 FATFS */
#define W25Q64_OTA_MAGIC_VAL           0xAA55AA55UL
#define W25Q64_APP_BUF_OFFSET          0x00200000UL          /* FATFS 2MB 之后 */
#define W25Q64_APP_BUF_SIZE            (64UL * W25QXX_SECTOR_SIZE)  /* 64 sectors = 256KB */
#define W25Q64_APP_BAK_OFFSET          0x00240000UL          /* APP_BUF 之后 */

/*============================================================================
 * v30: CRC32 table for OTA verify (W25Q64 buffered upgrade)
 *  Polynomial 0xEDB88320, 256 entries, 1KB flash cost — much faster than
 *  byte-by-byte computation in the original bl_crc32_update().
 *===========================================================================*/
static const INT32U g_crc32_table[256] = {
    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
    0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
    0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
    0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
    0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
    0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
    0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
    0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
    0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
    0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
    0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
    0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
    0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
    0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031E5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
    0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
    0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
    0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
    0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
    0xFED41B76,0x89D32AE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
    0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
    0xD80D2BDA,0xAF0A4B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
    0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
    0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB30A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
    0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
    0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
    0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
    0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
    0xAED41A4A,0xD9D32ADC,0x40DA7B28,0x37DA3B20,0xA9BCAE73,0xDEBB9A05,0x47B2CF6F,0x30B5FFE9,
    0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD04605,0xCDD766B3,0x54DE5729,0x23D967BF,
    0xB36672EE,0xC46142D8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DFDB,0x2D02EF8D
};

/*============================================================================
 * Modify 2026.6.24 v29: OTA magic 改存 STM32 内部 flash, 不用 W25Q64
 *  - v28 bug: BL 用 QUADSPI 擦 W25Q64 fake success, magic 实际还在 → 死循环
 *  - v29 fix: magic 存 STM32 内部 flash dual bank 模式 Bank 1 sector 6 (0x080FF000)
 *    - vendor HAL FLASH_SECTOR_14 = Bank 1 sector 6 = 0x080FF000 起点 (128KB sector)
 *    - 双 bank 模式每个 bank 1MB, sector 0-7 × 128KB, 跟 vendor HAL 编码对齐
 *    - 之前 sector=8 错位擦的是 Bank 1 sector 0 = 0x08100000, 不对
 *  - W25Q64 只存固件, 不存 magic (避开 QUADSPI vs vendor SPI 不兼容)
 *  - BL 单独编译, 用 vendor HAL (Flash_Write/Flash_Ctrl) 替代自写寄存器操作
 *    (避免 PSIZE=64-bit + ARMCC 32-bit STR 静默丢弃问题)
 *===========================================================================*/
#define INTERNAL_FLASH_MAGIC_ADDR   0x080FF000UL   // Bank 0 sector 7/* Bank 1 sector 6 起点 (128KB sector) */
#define INTERNAL_FLASH_MAGIC_VAL    0xAA55AA55UL   /* 触发升级 */
/* Modify 2026.6.24 v29 fix: dual bank 模式下 vendor HAL sector 14 = Bank 1 sector 6 = 0x080FF000
 * (原 sector 8 = Bank 1 sector 0 = 0x08100000, 错位)
 */
#define INTERNAL_FLASH_MAGIC_SECTOR 7             /* FLASH_SECTOR_14 */

/*============================================================================
 * 前向声明 (C89 严格模式, 函数必须先声明后使用)
 *===========================================================================*/
static int      bl_strlen(const char *s);
static void     bl_memcpy(void *dst, const void *src, uint32_t n);
static void     bl_memset(void *dst, int c, uint32_t n);
void             bl_delay_ms(uint32_t ms);

static void     bl_putc(uint8_t c);
void            bl_puts(const char *s);   /* Modify 2026.6.24 v18: extern 给 w25q64.c 用 */
void            bl_put_hex(uint8_t b);   /* 1 字节 hex (2 字符) — Modify 2026.6.24 v18: extern 给 w25q64.c 用 */

static void     bl_gpio_init(void);

static void     bl_led_buzzer_init(void);
static void     bl_led_set(uint8_t on);
static void     bl_led_toggle(void);
static void     bl_buzzer_beep(uint32_t ms);
/* Modify 2026.6.23: 非阻塞蜂鸣器状态机 (主循环不再被 delay 卡住) */
static void     bl_buzzer_start(uint32_t ticks);
static void     bl_buzzer_tick(void);
/* bl_play_success_melody 已移到 APP 侧 (User_PlayStartupChime), 2026.6.22 */
static void     bl_play_fail_alarm(void);

static void     bl_uart1_init(void);
static int      bl_uart1_getc(uint8_t *c);
static void     bl_uart1_putc(uint8_t c);

static void     bl_uart3_init(void);
static int      bl_uart3_getc(uint8_t *c);
static void     bl_uart3_putc(uint8_t c);

static void     bl_eeprom_init(void);
static int      bl_i2c_wait_flag(uint32_t flag, uint32_t timeout);
static int      bl_eeprom_write_byte(uint16_t addr, uint8_t val);
static int      bl_eeprom_read_byte(uint16_t addr, uint8_t *val);
static int      bl_eeprom_read_u32(uint16_t addr, uint32_t *val);
static int      bl_eeprom_write_u32(uint16_t addr, uint32_t val);

static void     bl_crc32_init(uint32_t *crc);
static void     bl_crc32_update(uint32_t *crc, const uint8_t *data, uint32_t len);
static uint32_t bl_crc32_finalize(uint32_t crc);

static int      bl_flash_unlock(void);
static void     bl_flash_lock(void);
static int      bl_flash_erase_sector(uint32_t sector);
static int      bl_flash_write(uint32_t addr, const uint8_t *data, uint32_t len);
static int      bl_flash_erase_app(void);

/* v30: W25Q64 buffered upgrade from BL */
static int      bl_w25q64_erase_sector(uint32_t sector);
static int      bl_w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len);
int             bl_upgrade_from_w25q64(void);

void             bl_jump_to_app(uint32_t app_addr);
void             bl_main(void);

/*============================================================================
 * startup_boot.s 导入的符号 wrapper
 *  - Bootloader_Main    : startup 调用的复位入口, 转发到 bl_main()
 *  - SpuriousHandler_ISR: 未注册中断的默认 handler, 死循环
 *===========================================================================*/
void Bootloader_Main(void);        /* prototype for startup reference */
void SpuriousHandler_ISR(void);    /* prototype for startup reference */

/* Modify 2026.6.23: __bl_set_msp 定义 (从 stm32h7xx_regs.h 移到这里, 避免多 .o 链接冲突) */
__asm void __bl_set_msp(uint32_t sp)
{
    MSR MSP, r0
    BX LR
}

/*============================================================================
 * 简版基础函数
 *===========================================================================*/
static int bl_strlen(const char *s)        { int n = 0; while (s[n]) n++; return n; }
static void bl_memcpy(void *dst, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;
}
static void bl_memset(void *dst, int c, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    while (n--) *d++ = (uint8_t)c;
}

/*============================================================================
 * 简版 Delay (粗略 ~ms, 假设 64MHz 主时钟)
 *  8000 NOP ≈ 1ms @ 64MHz
 *  C89 strict: 所有变量必须在块首声明
 *===========================================================================*/
void bl_delay_ms(uint32_t ms)
{
    volatile uint32_t j;
    for (; ms > 0; ms--) {
        for (j = 0; j < 8000; j++) {
            __asm("NOP");
        }
    }
}

/*============================================================================
 * GPIO 初始化 (PD11 = DI1 输入)
 *  DSB 屏障: 强制 AHB4ENR 写穿到外设, 避免 GPIOD 时钟没就绪时 MODER 写被丢
 *  GPIOA/GPIOB/USART3EN 宏统一在 stm32h7xx_regs.h 里定义
 *===========================================================================*/
static void bl_gpio_init(void)
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;
    __asm("DSB");
    /* PD11 输入: MODER bit[23:22] = 00 */
    GPIOD->MODER &= ~(0x3UL << (11 * 2));
}

/*============================================================================
 * LED (PI8) + 蜂鸣器 (PG3) 控制 — 用户 2026-06-22 定版
 *   行为规范:
 *     - 上电          : 蜂鸣器短鸣 100ms, LED 灭
 *     - BL 心跳循环   : LED 5Hz toggle (视觉心跳, 同步 UART 心跳), 蜂鸣器静默
 *     - 进入升级模式  : 蜂鸣器短鸣 100ms
 *     - 跳 APP 成功   : LED 闪 100ms + 蜂鸣器播电子音乐 ≤150ms (3 音)
 *     - 跳 APP 失败   : LED 爆闪 (10Hz) + 蜂鸣器 100ms 周期响 50ms 持续报警
 *  BSRR: bit[0:15] 置位, bit[16:31] 复位 (原子操作, 不需要 read-modify-write)
 *===========================================================================*/
#define BL_RUN_LED_PORT         GPIOI
#define BL_RUN_LED_PIN          8
#define BL_BUZZER_PORT          GPIOG
#define BL_BUZZER_PIN           3

static void bl_led_buzzer_init(void)
{
    /* 开 GPIOG / GPIOI 时钟 (AHB4) */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOGEN | RCC_AHB4ENR_GPIOIEN;
    __asm("DSB");

    /* PI8 输出: MODER bit[17:16] = 01 */
    BL_RUN_LED_PORT->MODER &= ~(0x3UL << (BL_RUN_LED_PIN * 2));
    BL_RUN_LED_PORT->MODER |=  (GPIO_MODE_OUTPUT << (BL_RUN_LED_PIN * 2));

    /* PG3 输出: MODER bit[7:6] = 01 */
    BL_BUZZER_PORT->MODER &= ~(0x3UL << (BL_BUZZER_PIN * 2));
    BL_BUZZER_PORT->MODER |=  (GPIO_MODE_OUTPUT << (BL_BUZZER_PIN * 2));

    /* 初始: LED 灭, 蜂鸣器静默 */
    BL_RUN_LED_PORT->BSRR = (1UL << (BL_RUN_LED_PIN + 16));     /* PI8 = 0 */
    BL_BUZZER_PORT->BSRR   = (1UL << (BL_BUZZER_PIN + 16));     /* PG3 = 0 */
}

/* LED 控制 (1=亮, 0=灭) — BSRR 原子操作, 不可重入安全 */
static void bl_led_set(uint8_t on)
{
    if (on) {
        BL_RUN_LED_PORT->BSRR = (1UL << BL_RUN_LED_PIN);        /* PI8 = 1 */
    } else {
        BL_RUN_LED_PORT->BSRR = (1UL << (BL_RUN_LED_PIN + 16)); /* PI8 = 0 */
    }
}

/* LED toggle — 用于心跳 */
static uint8_t bl_led_state = 0;
static void bl_led_toggle(void)
{
    bl_led_state = bl_led_state ? 0 : 1;
    bl_led_set(bl_led_state);
}

/* 蜂鸣器响 ms 毫秒 (方波驱动, BL 短脉冲足够) */
static void bl_buzzer_beep(uint32_t ms)
{
    BL_BUZZER_PORT->BSRR = (1UL << BL_BUZZER_PIN);              /* PG3 = 1 */
    bl_delay_ms(ms);
    BL_BUZZER_PORT->BSRR = (1UL << (BL_BUZZER_PIN + 16));       /* PG3 = 0 */
}

/* Modify 2026.6.23: 非阻塞蜂鸣器 (状态机)
 *   bl_buzzer_start(ticks) 开启蜂鸣器, 不阻塞
 *   bl_buzzer_tick() 每个时基 (10ms) 调一次, 到 0 自动关
 *   主循环期间 LED 翻转 + 串口输出 都不会被蜂鸣器 delay 卡顿 */
static volatile uint32_t bl_buzzer_ticks_left = 0;

/* 蜂鸣器开 (非阻塞, ticks 个 10ms 时基后自动关) */
static void bl_buzzer_start(uint32_t ticks)
{
    bl_buzzer_ticks_left = ticks;
    BL_BUZZER_PORT->BSRR = (1UL << BL_BUZZER_PIN);              /* PG3 = 1 */
}

/* 蜂鸣器 tick 处理: 每 10ms 时基调用, 倒计时到 0 自动关 */
static void bl_buzzer_tick(void)
{
    if (bl_buzzer_ticks_left > 0) {
        bl_buzzer_ticks_left--;
        if (bl_buzzer_ticks_left == 0) {
            BL_BUZZER_PORT->BSRR = (1UL << (BL_BUZZER_PIN + 16));   /* PG3 = 0 */
        }
    }
}

/*============================================================================
 * 跳 APP 成功提示音 — v3.7 用户反馈: v3.6 (340ms) 还是太快
 *   新版 (翻倍 + 明显停顿):
 *     滴(200ms) → 静(80ms) → 嘟(200ms) → 静(80ms) → 滴(120ms) = 680ms
 *   + LED 保持亮 300ms (让用户看清 LED)
 *   + LED 灭后 100ms 静默停顿 (让音乐/视觉余韵结束)
 *   总指示时长 = 1080ms (~1.1秒)
 *   无源蜂鸣器 200ms 足以完整发声, 80ms 停顿听感清晰
 *===========================================================================*/
/* Modify 2026.6.22: bl_play_success_melody 已废弃, 提示音逻辑全部移到 APP 侧
 * (user_app.c 的 User_PlayStartupChime). 这里保留注释作为版本历史参考.
 * 原始实现:
 *   滴 200ms + 静 80ms + 嘟 200ms + 静 80ms + 滴 120ms = 680ms */

/*============================================================================
 * 跳 APP 失败报警 — 用户规范: 100ms 周期响 50ms 持续报警
 *   LED 爆闪 10Hz 同步 (toggle 间隔 50ms)
 *   蜂鸣器 50ms 响 + 50ms 静, 持续 5 个周期 = 500ms (足够引起注意)
 *===========================================================================*/
static void bl_play_fail_alarm(void)
{
    int i;
    for (i = 0; i < 5; i++) {
        bl_led_toggle();                /* LED 爆闪 10Hz */
        BL_BUZZER_PORT->BSRR = (1UL << BL_BUZZER_PIN);     /* 蜂鸣器 ON */
        bl_delay_ms(50);
        BL_BUZZER_PORT->BSRR = (1UL << (BL_BUZZER_PIN + 16)); /* 蜂鸣器 OFF */
        bl_delay_ms(50);
    }
    bl_led_set(1);   /* 报警结束后 LED 保持亮, 提示故障 */
}

/*============================================================================
 * 双 UART 设计 (v30 — W25Q64 buffered upgrade, AA55 协议已废弃)
 *   - USART1 (PA9/PA10) = DEBUG 输出 — 引导阶段独占, 跳 APP 后 APP 重新初始化
 *   - USART3 (PB10/PB11) = DEBUG 输出 (v30 仅作 debug 备份)
 * 两者独立, 不互相影响。
 *===========================================================================*/

/* USART1 寄存器操作 (PA9=TX, PA10=RX, JP8 一般留空不接)
 *  注意: 不碰 PA13/PA14 (SWDIO/SWCLK) — DeepSeek 铁律 */
static void bl_uart1_putc_raw(uint8_t c)
{
    /* 带 timeout, 防止 USART1 异常时 BL 死锁 */
    uint32_t spin = 0;
    while (!(USART1->ISR & USART_ISR_TXE)) {
        if (++spin > 100000) return;   /* 超时直接放弃 */
    }
    USART1->TDR = c;
}

/* USART3 寄存器操作 (PB10=TX, PB11=RX, JP20 RS232 转 USB 接 PC) */
static void bl_uart3_putc_raw(uint8_t c)
{
    uint32_t spin = 0;
    while (!(USART3->ISR & USART_ISR_TXE)) {
        if (++spin > 100000) return;
    }
    USART3->TDR = c;
}

/* bl_putc / bl_puts → DEBUG 输出 → USART1 (PA9) + USART3 (PB10)
 *  banner / 状态信息 / 错误信息都同时输出, 方便双链路验证 */
static void bl_putc(uint8_t c)
{
    bl_uart1_putc_raw(c);
}
void bl_puts(const char *s)   /* Modify 2026.6.24 v18: extern 给 w25q64.c 用 */
{
    while (*s) {
        bl_uart1_putc_raw((uint8_t)*s++);
    }
}

/* 输出 1 字节为 2 位 hex 字符 (大写), 用 UART1+UART3 双输出 */
void bl_put_hex(uint8_t b)
{
    static const char hex[] = "0123456789ABCDEF";
    bl_putc(hex[(b >> 4) & 0x0F]);
    bl_putc(hex[b & 0x0F]);
}

/*============================================================================
 * USART1 初始化 (PA9/PA10, 115200 8N1) — BL DEBUG 通道
 *  DSB 屏障: H743 AHB4/APB 外设时钟使能后必须等时钟就绪再写外设寄存器
 *===========================================================================*/
static void bl_uart1_init(void)
{
    /* 1) 时钟 */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
    __asm("DSB");   /* 等 GPIOA 时钟就绪 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    __asm("DSB");   /* 等 USART1 时钟就绪 */

    /* 2) PA9 (TX) AF7 */
    GPIOA->MODER &= ~(0x3UL << (9 * 2));
    GPIOA->MODER |=  (GPIO_MODE_AF << (9 * 2));
    GPIOA->AFR[1] &= ~(0xFUL << ((9 - 8) * 4));
    GPIOA->AFR[1] |=  (7UL << ((9 - 8) * 4));

    /* 3) PA10 (RX) AF7 — 暂不用, 但配好方便后续做 DEBUG 双向 */
    GPIOA->MODER &= ~(0x3UL << (10 * 2));
    GPIOA->MODER |=  (GPIO_MODE_AF << (10 * 2));
    GPIOA->AFR[1] &= ~(0xFUL << ((10 - 8) * 4));
    GPIOA->AFR[1] |=  (7UL << ((10 - 8) * 4));

    /* 4) 115200 @ 64MHz (H743 HSI default = 64MHz): BRR = 555 */
    USART1->CR1 = 0;
    USART1->BRR = 555;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static int bl_uart1_getc(uint8_t *c)
{
    if (USART1->ISR & USART_ISR_RXNE) {
        *c = (uint8_t)(USART1->RDR & 0xFF);
        return 1;
    }
    return 0;
}
static void bl_uart1_putc(uint8_t c) { bl_uart1_putc_raw(c); }

/*============================================================================
 * USART3 初始化 (PB10/PB11, 115200 8N1) — 升级协议通道
 *  Modify 2026.6.24 v23 大胆版: 加 RXNE 中断 + ring buffer (4KB)
 *   解决: PC fire-and-forget 模式下, 单字节轮询 USART3->ISR 会丢字节
 *   ISR 写 ring head, bl_uart3_getc() 从 ring tail 读
 *===========================================================================*/
/* ring buffer — 4KB, 索引用 32-bit 以减少原子读问题 */
#define UART3_RX_RING_SIZE      4096
static uint8_t  uart3_rx_ring[UART3_RX_RING_SIZE];
static volatile uint32_t uart3_rx_head = 0;  /* ISR 写, 必须 volatile */
static volatile uint32_t uart3_rx_tail = 0;  /* 主循环读 */

void USART3_IRQHandler(void);
void USART3_IRQHandler(void)
{
    /* 一次性把 RX FIFO 里所有字节取尽 (虽然 H743 是 1 字节, 但用 while 更稳) */
    while (USART3->ISR & USART_ISR_RXNE) {
        uint8_t c = (uint8_t)(USART3->RDR & 0xFF);
        uint32_t next = (uart3_rx_head + 1) % UART3_RX_RING_SIZE;
        if (next != uart3_rx_tail) {
            /* 未满, 入队 */
            uart3_rx_ring[uart3_rx_head] = c;
            uart3_rx_head = next;
        } else {
            /* 已满, 丢弃 — 同时拉高 tail 让 BL 不再等这个字节
             * (这里不严格, 实际只丢 1 字节; 下次 CRC 失败会触发 NACK 重传) */
            break;
        }
    }
}

static void bl_uart3_init(void)
{
    /* 1) 时钟 */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
    __asm("DSB");   /* 等 GPIOB 时钟就绪 */
    RCC->APB1LENR |= RCC_APB1LENR_USART3EN;
    __asm("DSB");   /* 等 USART3 时钟就绪 */

    /* 2) PB10 (TX) AF7 */
    GPIOB->MODER &= ~(0x3UL << (10 * 2));
    GPIOB->MODER |=  (GPIO_MODE_AF << (10 * 2));
    GPIOB->AFR[1] &= ~(0xFUL << ((10 - 8) * 4));
    GPIOB->AFR[1] |=  (7UL << ((10 - 8) * 4));

    /* 3) PB11 (RX) AF7 */
    GPIOB->MODER &= ~(0x3UL << (11 * 2));
    GPIOB->MODER |=  (GPIO_MODE_AF << (11 * 2));
    GPIOB->AFR[1] &= ~(0xFUL << ((11 - 8) * 4));
    GPIOB->AFR[1] |=  (7UL << ((11 - 8) * 4));

    /* 4) 115200 @ 64MHz: BRR = 555 */
    USART3->CR1 = 0;
    USART3->BRR = 555;

    /* 5) 清 ring buffer 头尾 (C89 不能在声明里初始化 volatile — 显式赋 0) */
    uart3_rx_head = 0;
    uart3_rx_tail = 0;

    /* 6) 开 RXNE 中断 + 使能 USART + 使能 NVIC (Modify 2026.6.24 v23) */
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART3_IRQn);
}

static int bl_uart3_getc(uint8_t *c)
{
    /* 从 ring buffer 读, 不直接读 USART->ISR (避免和 ISR 抢数据) */
    if (uart3_rx_head == uart3_rx_tail) return 0;  /* 空 */
    *c = uart3_rx_ring[uart3_rx_tail];
    uart3_rx_tail = (uart3_rx_tail + 1) % UART3_RX_RING_SIZE;
    return 1;
}

/* Modify 2026.6.24 v23: ring buffer 占用字节数 (调试 / 心跳显示用) */
static uint32_t bl_uart3_rx_pending(void)
{
    return (uart3_rx_head - uart3_rx_tail) % UART3_RX_RING_SIZE;
}

static void bl_uart3_putc(uint8_t c) { bl_uart3_putc_raw(c); }

/*============================================================================
 * EEPROM (AT24C64, I2C1 PB8/PB9, 8KB, 16-bit addr)
 *  设备地址: 0xA0 写 / 0xA1 读 (7-bit = 0x50)
 *  Modify 2026.6.23: 之前是占位 return -1, 现在实现真实 I2C 读写
 *  AT24C64 写周期: ≤5ms (写完要等)
 *===========================================================================*/
/* Modify 2026.6.23 P1.5 v13: 删除 init_done 检查 (DEBUG 发现 root cause: startup_boot.s 没做 .data 复制 + .bss 清零,
 * 导致 bl_eeprom_init_done 静态变量在 SRAM 是随机值, 总是非零, 跳过 init, GPIOH/I2C2 时钟没 enable, EEPROM 通信失败).
 * 改成每次都跑 init (代码幂等, 仅多花 2ms 延时) */
static void bl_eeprom_init(void)
{
    int i;
    char hbuf[9];
    int k;
    uint32_t apb1_before, apb1_after;
    static const char hex[] = "0123456789ABCDEF";
    /* P1.5 v13: 移除 if (bl_eeprom_init_done) return; — 见上注释 */

    /* v11: 读 init 前 APB1LENR */
    apb1_before = *REG_APB1LENR;
    bl_puts("AT+BOOT=INIT_BEFORE_APB1LENR=0x");
    for (k = 7; k >= 0; k--) {
        hbuf[7-k] = hex[(apb1_before >> (k*4)) & 0xF];
    }
    hbuf[8] = 0;
    bl_puts(hbuf);
    bl_puts("\r\n");

    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
    __asm("DSB");
    RCC->APB1LENR |= RCC_APB1LENR_I2C2EN;   /* 改用 I2C2 */
    __asm("DSB");

    /* v11: 读 init 后 APB1LENR */
    apb1_after = *REG_APB1LENR;
    bl_puts("AT+BOOT=INIT_AFTER_APB1LENR=0x");
    for (k = 7; k >= 0; k--) {
        hbuf[7-k] = hex[(apb1_after >> (k*4)) & 0xF];
    }
    hbuf[8] = 0;
    bl_puts(hbuf);
    bl_puts("\r\n");

    /* I2C123SEL = 10 (hsi_ker_ck = 16MHz), 不受 PCLK1 分频影响 */
    *REG_D2CCIP2R = (*REG_D2CCIP2R & ~RCC_D2CCIP2R_I2C123SEL_Msk) | RCC_D2CCIP2R_I2C123SEL_HSIKER;
    __asm("DSB");
    bl_delay_ms(2);

    /* Modify 2026.6.23 P1.5 v12: AMKN8639 板 AT24C64 接到 I2C2 PH4/PH5 (AMKN8639_IOConfig.h:328-329)
     * 原 PB10/PB11 错引脚, EEPROM 不在那 — 现改 PH4/PH5 AF4 */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN;
    __asm("DSB");

    /* bus recovery: PH4/PH5 input, 检查 SDA stuck low */
    GPIOH->MODER &= ~((0x3UL << (4 * 2)) | (0x3UL << (5 * 2)));
    GPIOH->PUPDR &= ~((0x3UL << (4 * 2)) | (0x3UL << (5 * 2)));
    GPIOH->PUPDR |=  ((0x1UL << (4 * 2)) | (0x1UL << (5 * 2)));
    bl_delay_ms(1);
    if (!(GPIOH->IDR & (1UL << 5))) {
        GPIOH->MODER &= ~(0x3UL << (4 * 2));
        GPIOH->MODER |=  (GPIO_MODE_OUTPUT << (4 * 2));
        for (i = 0; i < 9; i++) {
            GPIOH->BSRR = (1UL << 4);
            bl_delay_ms(1);
            if (GPIOH->IDR & (1UL << 5)) break;
            GPIOH->BSRR = (1UL << (4 + 16));
            bl_delay_ms(1);
        }
    }

    /* PH4 (SCL) AF4 I2C2 — AFR[0] pin 0-7 */
    GPIOH->MODER &= ~(0x3UL << (4 * 2));
    GPIOH->MODER |=  (GPIO_MODE_AF << (4 * 2));
    GPIOH->AFR[0] &= ~(0xFUL << (4 * 4));
    GPIOH->AFR[0] |=  (4UL << (4 * 4));

    /* PH5 (SDA) AF4 I2C2 — AFR[0] pin 0-7 */
    GPIOH->MODER &= ~(0x3UL << (5 * 2));
    GPIOH->MODER |=  (GPIO_MODE_AF << (5 * 2));
    GPIOH->AFR[0] &= ~(0xFUL << (5 * 4));
    GPIOH->AFR[0] |=  (4UL << (5 * 4));

    I2C2->CR1 = 0;
    I2C2->TIMINGR = 0x30420F13UL;
    I2C2->CR1 = I2C_CR1_PE;
    bl_delay_ms(1);

    /* P1.5 v13: bl_eeprom_init_done 移除 */
}

/* 等待 I2C2->ISR 某 flag, timeout 是粗略循环计数 */
static int bl_i2c_wait_flag(uint32_t flag, uint32_t timeout)
{
    while (!(I2C2->ISR & flag)) {
        if (I2C2->ISR & (I2C_ISR_NACKF | I2C_ISR_BERR | I2C_ISR_ARLO)) {
            I2C2->ICR = I2C_ISR_NACKF | I2C_ISR_BERR | I2C_ISR_ARLO;
            return -1;
        }
        if (--timeout == 0) return -1;
    }
    return 0;
}

/* 写 1 字节 (addr 16-bit, val 8-bit) — 含 5ms 写周期等待 */
static int bl_eeprom_write_byte(uint16_t addr, uint8_t val)
{
    uint8_t hi = (uint8_t)(addr >> 8);
    uint8_t lo = (uint8_t)(addr & 0xFF);

    bl_eeprom_init();
    /* 等 BUSY 释放 */
    if (bl_i2c_wait_flag(I2C_ISR_BUSY, 0) != 0) {
        /* 已经在忙, 不一定是错 (短时), 直接试 */
    }
    I2C2->CR1 = I2C_CR1_PE;   /* 确保使能 */
    /* 配 CR2: 设备地址 0x50 (写), NBYTES=3 (addr_hi + addr_lo + val), START */
    I2C2->CR2 = (0x50UL << 1) | (3UL << I2C_CR2_NBYTES_SHIFT);
    I2C2->CR2 |= I2C_CR1_START;

    /* 写 addr_hi */
    if (bl_i2c_wait_flag(I2C_ISR_TXIS, 10000) != 0) return -1;
    I2C2->TXDR = hi;
    /* 写 addr_lo */
    if (bl_i2c_wait_flag(I2C_ISR_TXIS, 10000) != 0) return -1;
    I2C2->TXDR = lo;
    /* 写 val */
    if (bl_i2c_wait_flag(I2C_ISR_TXIS, 10000) != 0) return -1;
    I2C2->TXDR = val;
    /* 等 TC + STOP */
    if (bl_i2c_wait_flag(I2C_ISR_TC, 10000) != 0) return -1;
    I2C2->CR2 |= I2C_CR1_STOP;

    bl_delay_ms(5);   /* AT24C64 写周期 */
    return 0;
}

/* Modify 2026.6.23 P1.5 v3: 重置 I2C1 控制器 (用于 APP 软复位后 I2C 总线卡死) */
static void bl_i2c1_reset(void)
{
    /* 关 PE */
    I2C2->CR1 &= ~I2C_CR1_PE;
    bl_delay_ms(1);
    /* 清所有错误标志 */
    I2C2->ICR = 0xFFFFFFFFUL;
    /* 重设 TIMINGR */
    I2C2->TIMINGR = I2C_TIMING_100KHZ;
    /* 再开 PE */
    I2C2->CR1 = I2C_CR1_PE;
    bl_delay_ms(2);
}

/* 读 1 字节 (addr 16-bit) — 含 dummy write 阶段, Modify 2026.6.23 加 retry */
static int bl_eeprom_read_byte(uint16_t addr, uint8_t *val)
{
    uint8_t hi = (uint8_t)(addr >> 8);
    uint8_t lo = (uint8_t)(addr & 0xFF);
    int retry;

    bl_eeprom_init();
    for (retry = 0; retry < 3; retry++) {
        bl_i2c1_reset();    /* 每次重试前先重置 I2C1, 避免 APP RESET 时总线卡死 */
        I2C2->CR1 = I2C_CR1_PE;
        /* Phase 1: dummy write (发地址) */
        I2C2->CR2 = (0x50UL << 1) | (2UL << I2C_CR2_NBYTES_SHIFT);
        I2C2->CR2 |= I2C_CR1_START;

        if (bl_i2c_wait_flag(I2C_ISR_TXIS, 10000) != 0) continue;
        I2C2->TXDR = hi;
        if (bl_i2c_wait_flag(I2C_ISR_TXIS, 10000) != 0) continue;
        I2C2->TXDR = lo;
        if (bl_i2c_wait_flag(I2C_ISR_TC, 10000) != 0) continue;

        /* Phase 2: 读 1 字节 (用 RESTART) */
        I2C2->CR2 = (0x50UL << 1) | (1UL << I2C_CR2_NBYTES_SHIFT) | I2C_CR2_RD_WRN;
        I2C2->CR2 |= I2C_CR1_START;

        if (bl_i2c_wait_flag(I2C_ISR_RXNE, 10000) != 0) continue;
        *val = (uint8_t)(I2C2->RXDR & 0xFF);

        /* STOP + NACK (主机不应答) */
        I2C2->CR2 |= I2C_CR1_STOP;
        bl_delay_ms(1);
        return 0;   /* 成功 */
    }
    return -1;  /* 3 次都失败 */
}

/* 读 4 字节 (大端序) */
static int bl_eeprom_read_u32(uint16_t addr, uint32_t *val)
{
    uint8_t b0, b1, b2, b3;
    if (bl_eeprom_read_byte(addr,     &b0) != 0) return -1;
    if (bl_eeprom_read_byte(addr + 1, &b1) != 0) return -1;
    if (bl_eeprom_read_byte(addr + 2, &b2) != 0) return -1;
    if (bl_eeprom_read_byte(addr + 3, &b3) != 0) return -1;
    *val = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | b3;
    return 0;
}

/* 写 4 字节 (大端序) */
static int bl_eeprom_write_u32(uint16_t addr, uint32_t val)
{
    if (bl_eeprom_write_byte(addr,     (uint8_t)(val >> 24)) != 0) return -1;
    if (bl_eeprom_write_byte(addr + 1, (uint8_t)(val >> 16)) != 0) return -1;
    if (bl_eeprom_write_byte(addr + 2, (uint8_t)(val >>  8)) != 0) return -1;
    if (bl_eeprom_write_byte(addr + 3, (uint8_t)(val      )) != 0) return -1;
    return 0;
}

/*============================================================================
 * CRC32 IEEE 802.3 (多项式 0xEDB88320, 初值 0xFFFFFFFF, 结果 XOR 0xFFFFFFFF)
 *  byte-by-byte 算法, 适合 BL 这种资源紧张场景 (无查表 1KB 开销)
 *  Modify 2026.6.23: 用于双 UART 数据完整性校验
 *===========================================================================*/
static void bl_crc32_init(uint32_t *crc)   { *crc = 0xFFFFFFFFUL; }
static void bl_crc32_update(uint32_t *crc, const uint8_t *data, uint32_t len)
{
    uint32_t c = *crc;
    uint32_t i, j;
    for (i = 0; i < len; i++) {
        c ^= (uint32_t)data[i];
        for (j = 0; j < 8; j++) {
            if (c & 1UL) c = (c >> 1) ^ 0xEDB88320UL;
            else         c =  c >> 1;
        }
    }
    *crc = c;
}
static uint32_t bl_crc32_finalize(uint32_t crc)   { return crc ^ 0xFFFFFFFFUL; }

/*============================================================================
 * Flash 操作 (寄存器直写 H743)
 *===========================================================================*/
static int bl_flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    if (FLASH->CR & FLASH_CR_LOCK) return -1;
    return 0;
}
static void bl_flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}
static int bl_flash_erase_sector(uint32_t sector)
{
    uint32_t cr;
    bl_flash_unlock();
    FLASH->SR = FLASH_ERR_MASK;
    cr = FLASH_CR_SER | (sector << FLASH_CR_SNB_SHIFT) | (2UL << FLASH_CR_PSIZE_SHIFT);
    FLASH->CR = cr;
    FLASH->CR |= FLASH_CR_START;
    while (FLASH->SR & FLASH_SR_BSY) { /* spin */ }
    if (FLASH->SR & FLASH_ERR_MASK) {
        bl_flash_lock();
        return -1;
    }
    FLASH->CR = 0;
    bl_flash_lock();
    return 0;
}
static int bl_flash_write(uint32_t addr, const uint8_t *data, uint32_t len)
{
    bl_flash_unlock();
    FLASH->SR = FLASH_ERR_MASK;
    FLASH->CR = (2UL << FLASH_CR_PSIZE_SHIFT) | FLASH_CR_PG;

    while (len >= 32) {
        volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)addr;
        const uint32_t *src = (const uint32_t *)data;
        int i;
        for (i = 0; i < 8; i++) {
            dst[i] = src[i];
        }
        while (FLASH->SR & FLASH_SR_BSY) { /* spin */ }
        if (FLASH->SR & FLASH_ERR_MASK) {
            FLASH->CR = 0;
            bl_flash_lock();
            return -1;
        }
        FLASH->SR = FLASH_SR_EOP;
        addr += 32;
        data += 32;
        len  -= 32;
    }
    if (len > 0) {
        volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)addr;
        const uint32_t *src = (const uint32_t *)data;
        uint32_t i;
        for (i = 0; i < (len + 3) / 4; i++) {
            dst[i] = src[i];
        }
        while (FLASH->SR & FLASH_SR_BSY) { /* spin */ }
        if (FLASH->SR & FLASH_ERR_MASK) {
            FLASH->CR = 0;
            bl_flash_lock();
            return -1;
        }
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR = 0;
    bl_flash_lock();
    return 0;
}

#define APP_SECTOR_START        2
#define APP_SECTOR_END          7
static int bl_flash_erase_app(void)
{
    uint32_t s;
    bl_puts("AT+BOOT=ERASE APP 0x08010000-0x0807FFFF...\r\n");
    for (s = APP_SECTOR_START; s <= APP_SECTOR_END; s++) {
        if (bl_flash_erase_sector(s) != 0) {
            bl_puts("AT+BOOT=ERASE FAIL sector=");
            bl_putc('0' + s);
            bl_puts("\r\n");
            return -1;
        }
        bl_putc('.');
    }
    bl_puts("\r\nAT+BOOT=ERASE OK\r\n");
    return 0;
}

/*============================================================================
 * 跳转 APP
 *  H743 内存布局 (只列合法栈区):
 *    ITCMRAM  : 0x00000000 - 0x0000FFFF (64KB)
 *    DTCMRAM  : 0x20000000 - 0x2001FFFF (128KB)
 *    AXI SRAM : 0x24000000 - 0x2407FFFF (512KB)
 *    SRAM1/2/3/4: 0x30000000-0x30047FFF / 0x30000000-0x30047FFF ...
 *  SP 校验: 拒绝 0xFFFFFFFF(未烧写) 和 0x08xxxxxx(flash), 其他 RAM 都接受
 *===========================================================================*/
void bl_jump_to_app(uint32_t app_addr)
{
    uint32_t *p_vec = (uint32_t *)app_addr;
    uint32_t app_sp = p_vec[0];
    uint32_t app_reset = p_vec[1];

    if (app_sp == 0xFFFFFFFFUL || (app_sp & 0xFF000000UL) == 0x08000000UL) {
        bl_puts("AT+BOOT=APP SP invalid\r\n");
        bl_play_fail_alarm();    /* 跳失败: LED 爆闪 + 蜂鸣器报警 */
        return;
    }
    if ((app_reset & 0xFF000000UL) != 0x08000000UL) {
        bl_puts("AT+BOOT=APP reset invalid\r\n");
        bl_play_fail_alarm();
        return;
    }

    bl_puts("AT+BOOT=Jump APP 0x08010000\r\n");
    /* Modify 2026.6.22: 跳 APP 成功提示音已移到 APP 侧 (User_PlayStartupChime), 这里只跳不响 */
    bl_led_set(1);                       /* 跳 APP 成功: LED 亮 (持续到跳 APP 前) */
    bl_delay_ms(300);                    /* 等 APP 启动后播放提示音 (300ms 是 APP 启动 + Init 的窗口) */
    bl_led_set(0);                       /* LED 灭, 准备跳 APP */

    __disable_irq();
    SCB->VTOR = app_addr;
    __set_msp(app_sp);  /* ARMCC 函数式内联汇编, 见 stm32h7xx_regs.h */

    {
        void (*app_entry)(void) = (void (*)(void))(uintptr_t)app_reset;
        app_entry();
    }
    while (1) { }
}

/*============================================================================
 * W25Q64 三段保险 — 集成 (Modify 2026.6.23: P1)
 *  设计:
 *    - 启动时软失败初始化 W25Q64 (失败不阻塞, 跳过回滚)
 *    - 检测 MCU APP 头 (SP/reset) 是否合法
 *    - 不合法 → 尝试从 W25Q64 backup 恢复 → 不行从 factory 恢复
 *    - 升级成功后自动把新 APP 备份到 backup 区
 *  RAM 使用: 4KB 块缓冲 (D1 SRAM, 64KB BL 限制外)
 *===========================================================================*/
#define W25Q64_BACKUP_CHUNK     4096    /* 4KB 块缓冲 (Flash 扇区大小) */
static uint8_t g_w25q64_buf[W25Q64_BACKUP_CHUNK];

/* 软失败初始化 W25Q64 (失败不阻塞) */
static int bl_w25q64_safe_init(void)
{
    int ret = w25q64_init();
    if (ret == 0) {
        /* Modify 2026.6.24 v16 P1.5: 打印真实 JEDEC (之前硬编码 "EF40" 是错的) */
        uint8_t mfg, msb, lsb;
        w25q64_get_jedec(&mfg, &msb, &lsb);
        bl_puts("AT+BOOT=W25Q64 OK (JEDEC=");
        bl_put_hex(mfg);
        bl_put_hex(msb);
        bl_put_hex(lsb);
        bl_puts(" expected EF4017)\r\n");
        return 0;
    } else if (ret == -2) {
        bl_puts("AT+BOOT=W25Q64 FAIL: JEDEC MFG mismatch\r\n");
    } else {
        bl_puts("AT+BOOT=W25Q64 FAIL: init err\r\n");
    }
    return -1;
}

/* 从 W25Q64 某段恢复 APP 到 MCU flash
 *  offset:  W25Q64_BACKUP_OFFSET / W25Q64_FACTORY_OFFSET
 *  magic:   期望的 magic (BACKUP / FACTORY)
 *  返回: 0 = 成功, -1 = 段空/magic 不匹配, -2 = 读失败, -3 = 写失败 */
static int bl_restore_from_region(uint32_t offset, uint32_t magic)
{
    uint8_t  hdr[W25Q64_REGION_HEADER_SIZE];
    uint32_t size;
    uint32_t write_addr = APP_START_ADDR;
    uint32_t read_pos;
    uint32_t chunk;
    uint32_t i;

    /* 1) 读区域头 */
    if (w25q64_read(offset, hdr, W25Q64_REGION_HEADER_SIZE) != 0) return -2;
    {
        uint32_t hdr_magic = ((uint32_t)hdr[0] << 24) |
                             ((uint32_t)hdr[1] << 16) |
                             ((uint32_t)hdr[2] <<  8) |
                             ((uint32_t)hdr[3] >>  0);
        if (hdr_magic == W25Q64_MAGIC_EMPTY || hdr_magic != magic) return -1;
    }

    size = ((uint32_t)hdr[4] << 24) |
           ((uint32_t)hdr[5] << 16) |
           ((uint32_t)hdr[6] <<  8) |
           ((uint32_t)hdr[7] >>  0);
    if (size == 0 || size > APP_MAX_SIZE) return -1;

    bl_puts("AT+BOOT=Restore from region size=");
    bl_put_hex((uint8_t)(size >> 24)); bl_put_hex((uint8_t)(size >> 16));
    bl_put_hex((uint8_t)(size >>  8)); bl_put_hex((uint8_t)(size      ));
    bl_puts("\r\n");

    /* 2) 擦 MCU APP 区 (扇区 2-7, 共 384KB 但实际只写 size 字节) */
    if (bl_flash_erase_app() != 0) return -3;

    /* 3) 分块读 W25Q64 + 写 MCU flash (4KB 块) */
    read_pos = W25Q64_REGION_HEADER_SIZE;
    while (size > 0) {
        chunk = (size > W25Q64_BACKUP_CHUNK) ? W25Q64_BACKUP_CHUNK : size;
        if (w25q64_read(offset + read_pos, g_w25q64_buf, chunk) != 0) return -2;
        if (bl_flash_write(write_addr, g_w25q64_buf, chunk) != 0) return -3;
        read_pos   += chunk;
        write_addr += chunk;
        size       -= chunk;
    }
    (void)i;
    return 0;
}

/* 从 backup 恢复 */
static int bl_restore_from_backup(void)
{
    bl_puts("AT+BOOT=Try restore from BACKUP...\r\n");
    return bl_restore_from_region(W25Q64_BACKUP_OFFSET, W25Q64_MAGIC_BACKUP);
}

/* 从 factory 恢复 (最终回滚) */
static int bl_restore_from_factory(void)
{
    bl_puts("AT+BOOT=Try restore from FACTORY...\r\n");
    return bl_restore_from_region(W25Q64_FACTORY_OFFSET, W25Q64_MAGIC_FACTORY);
}

/* 把当前 MCU APP 备份到 W25Q64 backup 区
 *  流程: 读 MCU APP 头校验 → 读 size 字节 → w25q64_region_write 到 BACKUP
 *  返回: 0 = 成功, -1 = APP 头不合法, -2 = 写失败 */
static int bl_backup_current_app(void)
{
    uint32_t *p_vec = (uint32_t *)APP_START_ADDR;
    uint32_t app_sp = p_vec[0];
    uint32_t size = 0;
    uint32_t read_addr = APP_START_ADDR;
    uint32_t w25_offset = 0;

    /* 1) 校验 APP 头 */
    if (app_sp == 0xFFFFFFFFUL || (app_sp & 0xFF000000UL) == 0x08000000UL) return -1;

    /* 2) 用 size = 第一扇区 0xFF 检测法估算 APP 长度 (从 0x08010000 开始扫 0xFF) */
    {
        uint32_t scan;
        for (scan = APP_MAX_SIZE / 4 - 1; scan > 0; scan--) {
            if (p_vec[scan] != 0xFFFFFFFFUL) {
                size = (scan + 1) * 4;
                /* 对齐到 256 字节 (page) */
                size = (size + 255) & ~255UL;
                break;
            }
        }
    }
    if (size == 0 || size > APP_MAX_SIZE) return -1;
    bl_puts("AT+BOOT=Backup current APP size=");
    bl_put_hex((uint8_t)(size >> 24)); bl_put_hex((uint8_t)(size >> 16));
    bl_put_hex((uint8_t)(size >>  8)); bl_put_hex((uint8_t)(size      ));
    bl_puts("\r\n");

    /* 3) 写到 W25Q64 backup 区 (直接用 w25q64_region_write) */
    /*    但 region_write 会先擦整个 region, 我们需要先把数据缓存到 RAM */
    /*    简化: 因为 size 可能很大 (几百 KB), 没法一次性缓存, 改为直接用 */
    /*    w25q64_sector_erase + w25q64_page_program 增量写 */
    {
        /* 擦 4 个 64KB 块 (共 256KB) */
        for (w25_offset = 0; w25_offset < 4; w25_offset++) {
            if (w25q64_block64_erase(W25Q64_BACKUP_OFFSET + w25_offset * W25Q64_BLOCK64_SIZE) != 0) return -2;
        }
        /* 写 64B 头 */
        {
            uint8_t hdr[W25Q64_REGION_HEADER_SIZE];
            uint32_t i;
            for (i = 0; i < W25Q64_REGION_HEADER_SIZE; i++) hdr[i] = 0xFF;
            hdr[0] = (uint8_t)(W25Q64_MAGIC_BACKUP >> 24);
            hdr[1] = (uint8_t)(W25Q64_MAGIC_BACKUP >> 16);
            hdr[2] = (uint8_t)(W25Q64_MAGIC_BACKUP >>  8);
            hdr[3] = (uint8_t)(W25Q64_MAGIC_BACKUP >>  0);
            hdr[4] = (uint8_t)(size >> 24);
            hdr[5] = (uint8_t)(size >> 16);
            hdr[6] = (uint8_t)(size >>  8);
            hdr[7] = (uint8_t)(size >>  0);
            /* 写到一个 256B 临时页 (前 64B 头, 后 192B 0xFF) */
            for (i = 0; i < W25Q64_PAGE_SIZE; i++) g_w25q64_buf[i] = (i < W25Q64_REGION_HEADER_SIZE) ? hdr[i] : 0xFF;
            if (w25q64_page_program(W25Q64_BACKUP_OFFSET, g_w25q64_buf, W25Q64_PAGE_SIZE) != 0) return -2;
        }
        /* 分块读 MCU + 写 W25Q64 (4KB 块) */
        w25_offset = W25Q64_REGION_HEADER_SIZE;
        read_addr = APP_START_ADDR;
        while (size > 0) {
            uint32_t chunk = (size > W25Q64_BACKUP_CHUNK) ? W25Q64_BACKUP_CHUNK : size;
            /* 读 MCU flash (直接 memcopy) */
            {
                uint8_t *src = (uint8_t *)read_addr;
                uint8_t *dst = g_w25q64_buf;
                uint32_t j;
                for (j = 0; j < chunk; j++) *dst++ = *src++;
            }
            if (w25q64_page_program(W25Q64_BACKUP_OFFSET + w25_offset, g_w25q64_buf, chunk) != 0) return -2;
            read_addr  += chunk;
            w25_offset += chunk;
            size       -= chunk;
        }
    }
    return 0;
}

/*============================================================================
 * v30 BL: W25Q64 buffered upgrade
 *  流程:
 *    1) 读 W25Q64 magic @ W25Q64_OTA_MAGIC_ADDR
 *    2) 读头 (size + crc32) @ W25Q64_APP_BUF_OFFSET
 *    3) 算 W25Q64 固件 CRC32 (头后 8B 起)
 *    4) 擦 flash sector 1-7 (0x08010000-0x080FFFFF)
 *    5) W25Q64 APP_BUF → flash 0x08010000 (256B 分块, 喂 IWDG)
 *    6) 校验 flash CRC32
 *    7) 失败 → 回滚 from APP_BAK
 *    8) 成功 → 清 internal flash magic + W25Q64 magic
 *===========================================================================*/
static int bl_w25q64_erase_sector(uint32_t sector)
{
    /* sector 是 W25Q64 物理 sector 编号 (0,1,2,...)
     * W25Q64 sector size = 4KB, addr = sector * 4096 */
    return w25q64_sector_erase(sector * W25QXX_SECTOR_SIZE);
}

static int bl_w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return w25q64_read(addr, buf, len);
}

int bl_upgrade_from_w25q64(void)
{
    uint32_t w25_magic, fw_size, fw_crc, calc_crc;
    uint8_t  buf[256];
    uint32_t off;
    uint32_t chunk;
    uint32_t j;
    int      b;
    int      i;

    /* Step 1: 读 W25Q64 magic (sector 704) */
    if (bl_w25q64_read(W25Q64_OTA_MAGIC_ADDR, (uint8_t*)&w25_magic, 4) != 0) {
        bl_puts("AT+BOOT=W25Q64 read error\r\n");
        return -1;
    }
    if (w25_magic != W25Q64_OTA_MAGIC_VAL) {
        bl_puts("AT+BOOT=W25Q64 magic NOT found, fall through APP\r\n");
        return -1;
    }
    bl_puts("AT+BOOT=W25Q64 magic found, start OTA\r\n");

    /* Step 2: 读头 (size + crc32) - 头 8B 在 APP_BUF 起始 */
    bl_w25q64_read(W25Q64_APP_BUF_OFFSET, (uint8_t*)&fw_size, 4);
    bl_w25q64_read(W25Q64_APP_BUF_OFFSET + 4, (uint8_t*)&fw_crc, 4);
    if (fw_size == 0 || fw_size > W25Q64_APP_BUF_SIZE || (fw_size & 3) != 0) {
        bl_puts("AT+BOOT=invalid FW size\r\n");
        return -2;
    }

    /* Step 3: 算 W25Q64 固件 CRC32 (头后 8B 起, 即 +8 偏移) */
    calc_crc = 0xFFFFFFFF;
    for (off = 0; off < fw_size; off += 512) {
        chunk = (fw_size - off > 512) ? 512 : (fw_size - off);
        bl_w25q64_read(W25Q64_APP_BUF_OFFSET + 8 + off, buf, chunk);
        for (j = 0; j < chunk; j++) {
            calc_crc = g_crc32_table[(calc_crc ^ buf[j]) & 0xFF] ^ (calc_crc >> 8);
        }
        *(volatile uint32_t *)0x58004800 = 0xAAAA;  /* 喂 IWDG */
    }
    if (calc_crc != fw_crc) {
        bl_puts("AT+BOOT=W25Q64 CRC FAIL\r\n");
        bl_w25q64_erase_sector(W25Q64_OTA_MAGIC_ADDR / W25QXX_SECTOR_SIZE);
        return -3;
    }
    bl_puts("AT+BOOT=CRC OK, erasing APP sectors...\r\n");

    /* Step 4: 擦 flash sector 1-7 (0x08010000-0x080FFFFF) */
    for (i = 1; i <= 7; i++) {
        bl_flash_erase_sector(i);
        *(volatile uint32_t *)0x58004800 = 0xAAAA;
    }

    /* Step 5: W25Q64 APP_BUF → flash 0x08010000 (256B 分块, 喂 IWDG) */
    for (off = 0; off < fw_size; off += 256) {
        chunk = (fw_size - off > 256) ? 256 : (fw_size - off);
        bl_w25q64_read(W25Q64_APP_BUF_OFFSET + 8 + off, buf, chunk);
        bl_flash_write(0x08010000 + off, buf, chunk);
        if ((off & 0x3FFF) == 0) *(volatile uint32_t *)0x58004800 = 0xAAAA;
    }
    bl_puts("AT+BOOT=FLASH write OK, verifying...\r\n");

    /* Step 6: 校验 flash CRC32 */
    calc_crc = 0xFFFFFFFF;
    for (off = 0; off < fw_size; off += 4) {
        uint32_t w = *(volatile uint32_t *)(0x08010000 + off);
        for (b = 0; b < 4; b++) {
            calc_crc = g_crc32_table[(calc_crc ^ ((w >> (b*8)) & 0xFF)) & 0xFF] ^ (calc_crc >> 8);
        }
        if ((off & 0x3FFF) == 0) *(volatile uint32_t *)0x58004800 = 0xAAAA;
    }
    if (calc_crc != fw_crc) {
        bl_puts("AT+BOOT=FLASH verify FAIL, rolling back from backup\r\n");
        /* Step 7: 回滚 - 读 W25Q64 APP_BAK → 写回 flash */
        for (i = 1; i <= 7; i++) bl_flash_erase_sector(i);
        for (off = 0; off < fw_size; off += 256) {
            chunk = (fw_size - off > 256) ? 256 : (fw_size - off);
            bl_w25q64_read(W25Q64_APP_BAK_OFFSET + off, buf, chunk);
            bl_flash_write(0x08010000 + off, buf, chunk);
        }
        return -6;
    }

    /* Step 8: 成功, 清 magic */
    bl_flash_erase_sector(INTERNAL_FLASH_MAGIC_SECTOR);
    bl_w25q64_erase_sector(W25Q64_OTA_MAGIC_ADDR / W25QXX_SECTOR_SIZE);
    bl_puts("AT+BOOT=UPGRADE OK, jumping to APP\r\n");
    return 0;
}

/*============================================================================
 * BL 入口
 *===========================================================================*/
/*============================================================================
 * W25Q64 upgrade: read fw from ext flash, write to internal flash
 *============================================================================*/
#define W25Q64_FW_OFFSET  0x1000UL
void bl_main(void)
{
    int need_upgrade;

    *(volatile uint32_t *)0x58004800 = 0xAAAA; /* feed IWDG on entry */

    bl_gpio_init();
    bl_led_buzzer_init();        /* LED PI8 + 蜂鸣器 PG3 — 行业标准状态指示 */
    bl_uart1_init();   /* UART1 PA9/PA10 = DEBUG 通道 */
    bl_uart3_init();   /* UART3 PB10/PB11 = DEBUG 通道 */
    bl_w25q64_safe_init();   /* Modify 2026.6.23: P1 W25Q64 软失败初始化 (失败不阻塞) */

    bl_puts("\r\n=== AMKN8639 BL v1.0 (Phase 3.2 / W25Q64 buffered) ===\r\n");
    bl_puts("AT+BOOT=Build 2026.6.25 (v30: W25Q64 buffered upgrade, no AA55)\r\n");
    bl_buzzer_beep(100);         /* 第1声 */
    bl_delay_ms(120);
    bl_buzzer_beep(100);         /* 第2声 */

    /* v30: 检查 internal flash magic (0x080FF000), 决定是否走升级流程
     *  - magic == 0xAA55AA55 → 进入升级 (从 W25Q64 APP_BUF 读固件)
     *  - magic 其他/空        → 跳 APP */
    need_upgrade = 0;
    {
        volatile uint32_t *p_magic = (volatile uint32_t *)INTERNAL_FLASH_MAGIC_ADDR;
        uint32_t m = *p_magic;
        if (m == INTERNAL_FLASH_MAGIC_VAL) {
            bl_puts("AT+BOOT=InternalFlash magic match, UPGRADE MODE\r\n");
            need_upgrade = 1;
        } else {
            bl_puts("AT+BOOT=InternalFlash magic NOT match, jump APP\r\n");
        }
    }

    if (need_upgrade) {
        bl_buzzer_beep(100);
        bl_upgrade_from_w25q64();  /* v30: 直接从 W25Q64 读, 无 AA55 协议 */
        goto jump_app;
    }

    /* Modify 2026.6.23: P1 — 跳 APP 前检测有效性, 失败则尝试 W25Q64 回滚 */
    {
        uint32_t *p_vec = (uint32_t *)APP_START_ADDR;
        uint32_t app_sp = p_vec[0];
        uint32_t app_reset = p_vec[1];
        int app_valid = 1;

        if (app_sp == 0xFFFFFFFFUL || (app_sp & 0xFF000000UL) == 0x08000000UL) app_valid = 0;
        if ((app_reset & 0xFF000000UL) != 0x08000000UL) app_valid = 0;

        if (!app_valid) {
            bl_puts("AT+BOOT=APP invalid, try rollback...\r\n");
            /* 尝试从 backup 恢复 */
            if (bl_restore_from_backup() == 0) {
                bl_puts("AT+BOOT=Restore from BACKUP OK, retry jump APP\r\n");
            } else if (bl_restore_from_factory() == 0) {
                bl_puts("AT+BOOT=Restore from FACTORY OK, retry jump APP\r\n");
            } else {
                bl_puts("AT+BOOT=Restore FAIL, no valid APP\r\n");
            }
        }
    }

jump_app:
    /* v30.2: 跳 APP 前擦 W25Q64 magic sector 704
     * 让 APP 端 PREPARE 时 W25QXX_Write 4B 不触发 auto-erase (vendor HAL 阻塞) */
    *(volatile uint32_t *)0x58004800 = 0xAAAA;  /* 喂狗 */
    if (bl_w25q64_erase_sector(W25Q64_OTA_MAGIC_ADDR / W25QXX_SECTOR_SIZE) == 0) {
        bl_puts("AT+BOOT=W25Q64 magic sector (704) erased for next upgrade\r\n");
    } else {
        bl_puts("AT+BOOT=W25Q64 magic sector erase FAIL (non-fatal)\r\n");
    }
    *(volatile uint32_t *)0x58004800 = 0xAAAA;

    /* 上电成功提示音: 滴 滴滴滴 滴滴 (6音, ~150ms) */
    bl_buzzer_beep(12); bl_delay_ms(13);
    bl_buzzer_beep(12); bl_delay_ms(13);
    bl_buzzer_beep(12); bl_delay_ms(13);
    bl_buzzer_beep(12); bl_delay_ms(13);
    bl_buzzer_beep(12); bl_delay_ms(13);
    bl_buzzer_beep(12);

    bl_jump_to_app(APP_START_ADDR);

    /* Modify 2026.6.22 v5: 跳 APP 失败 → 持续报警 (三路节奏真正独立可调)
     *   用户规范: 蜂鸣器节奏 / LED 频率 / 串口频率 三者独立控制
     *   用 10ms 时基 + 三个独立计数器分频 (10 是最小分度)
     *   bl_jump_to_app() 内部已跑过 bl_play_fail_alarm() (5周期=500ms),
     *   这里接管的 while(1) 必须继续响, 不能只闪 LED. */
#define FAIL_TICK_MS         10    /* 基础时基 (最小分度 10ms) */
#define FAIL_LED_TICKS       10    /* LED toggle 间隔 = 10×10ms = 100ms */
#define FAIL_HEART_TICKS     10    /* 串口输出间隔 = 10×10ms = 100ms */
#define FAIL_BUZZER_TICKS    500    /* 蜂鸣器周期 = 50×10ms = 500ms */
#define FAIL_BUZZER_ON_TICKS 100    /* 蜂鸣器响声 = 10×10ms = 100ms */
    {
        uint32_t tick = 0;
        while (1) {
            bl_delay_ms(FAIL_TICK_MS);
            tick++;

            /* LED toggle: 每 FAIL_LED_TICKS 个 tick 翻一次 */
            if ((tick % FAIL_LED_TICKS) == 0) {
                bl_led_toggle();
            }

            /* 串口心跳: 每 FAIL_HEART_TICKS 个 tick 输出一次 */
            if ((tick % FAIL_HEART_TICKS) == 0) {
                bl_puts("AT+HEART=1\r\n");
            }

            /* 蜂鸣器: 每 FAIL_BUZZER_TICKS 个 tick 启动响声 (非阻塞, 不卡主循环) */
            if ((tick % FAIL_BUZZER_TICKS) == 0) {
                bl_buzzer_start(FAIL_BUZZER_ON_TICKS);
            }
            /* 蜂鸣器 tick 处理 (持续跑, 自动倒计时关) */
            bl_buzzer_tick();
        }
    }
#undef FAIL_TICK_MS
#undef FAIL_LED_TICKS
#undef FAIL_HEART_TICKS
#undef FAIL_BUZZER_TICKS
#undef FAIL_BUZZER_ON_TICKS
}

/*============================================================================
 * startup_boot.s 符号 wrapper
 *===========================================================================*/
void Bootloader_Main(void)
{
    bl_main();
}

void SpuriousHandler_ISR(void)
{
    /* 默认 spurious handler, 死循环. BL 阶段不上 OS, 不需要特殊处理 */
    while (1) { }
}
