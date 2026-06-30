/************************************************************************************
*  File name: w25q64.h
*  Project  : AMKN8639 self-developed Bootloader
*  Description: W25Q64 SPI Flash 精简驱动头文件 (Phase 4 P1)
*                - 寄存器直写 H743 QUADSPI (单线 SPI 模式)
*                - 零依赖 (不引厂商 .lib / .c)
*                - 只用 JEDEC 基础命令集 (读 / 写 / 擦)
*                - 三段保险布局: temp + backup + factory 各 256KB
*
*  Modify History:
*   1. Version: 1.0
*      Date:    2026.6.23
*      Modify:  Phase 4 P1 - create file (W25Q64 + 三段保险)
*
*************************************************************************************/
#ifndef __W25Q64_H__
#define __W25Q64_H__

#include <stdint.h>

/* Modify 2026.6.24 v18 P1.5: 调试开关
 *  1 = 打印 SR/CR/DCR 等寄存器, 用于定位 BUSY 卡死根因
 *  0 = 静默 (生产) */
#ifndef W25Q64_DBG
#define W25Q64_DBG  1
#endif

/* BL 调试输出 (bootloader.c 提供) */
extern void bl_puts(const char *s);
extern void bl_put_hex(uint8_t b);   /* 1 字节 hex (2 字符) */

/* Modify 2026.6.24 v18: w25q64.c 内部 32-bit hex 打印 helper (ARMCC 5.05 不支持 inline) */
extern void bl_put_hex32(uint32_t v);

/*============================================================================
 * W25Q64 (板载 8MB SPI Flash) 基础参数
 *  - 总容量 8MB (2^23 = 8388608 bytes)
 *  - 4KB sector erase (0x20)
 *  - 32KB / 64KB block erase (0x52 / 0xD8)
 *  - 256 byte page program (0x02)
 *  - JEDEC ID = 0xEF 0x40 0x17
 *===========================================================================*/
#define W25Q64_JEDEC_ID_MFG      0xEF    /* Winbond */
#define W25Q64_JEDEC_ID_MEM_MSB  0x40    /* SPI Flash */
#define W25Q64_JEDEC_ID_MEM_LSB  0x17    /* 64Mbit = 8MB */
#define W25Q64_TOTAL_SIZE        (8UL * 1024UL * 1024UL)    /* 8MB */
#define W25Q64_SECTOR_SIZE       (4UL * 1024UL)             /* 4KB */
#define W25Q64_BLOCK64_SIZE      (64UL * 1024UL)            /* 64KB */
#define W25Q64_PAGE_SIZE         256                        /* 256B */

/* W25Q64 命令集 (Modify 2026.6.23: 只用基础 8 条命令) */
#define W25Q64_CMD_READ_STATUS1  0x05    /* 读状态寄存器 1 (BUSY + WEL) */
#define W25Q64_CMD_READ_JEDEC_ID 0x9F    /* 读 JEDEC ID (3 字节) */
#define W25Q64_CMD_READ_DATA     0x03    /* 读数据 (低速, 1-line) */
#define W25Q64_CMD_FAST_READ     0x0B    /* 快读 (8 dummy cycles) */
#define W25Q64_CMD_DUAL_IO_READ  0xBB    /* Modify 2026.6.24 v15: Dual I/O Read (2-line addr+data) */
#define W25Q64_CMD_WRITE_ENABLE  0x06    /* 写使能 (擦/写之前必发) */
#define W25Q64_CMD_PAGE_PROGRAM  0x02    /* 页编程 (≤256B) */
#define W25Q64_CMD_SECTOR_ERASE  0x20    /* 扇区擦 (4KB) */
#define W25Q64_CMD_BLOCK64_ERASE 0xD8    /* 块擦 (64KB) */

/* 状态寄存器 1 位定义 */
#define W25Q64_SR1_BUSY          0x01    /* BUSY = 1 表示内部操作进行中 */
#define W25Q64_SR1_WEL           0x02    /* WEL = 1 表示写使能 */

/*============================================================================
 * 三段保险 (三段备份) 布局
 *  设计思想 (用户 2026.6.23 拍板, P1 阶段):
 *    - temp   区: 本次升级临时存放 (网络/RK3568 推过来的新固件)
 *    - backup 区: 上一次成功运行的固件 (回滚目标)
 *    - factory 区: 出厂固件 (最终回滚目标, 永不覆盖)
 *
 *  升级流程:
 *    1) 新固件 → temp 区 (W25Q64 写入 + 校验)
 *    2) 校验通过 → 从当前 APP 区复制到 backup 区
 *    3) temp 区 → 烧 MCU APP 区域
 *    4) 烧写成功 → 标记 temp 区为 "新出厂" (下次启动作为 factory)
 *
 *  回滚流程 (启动检测):
 *    1) 读 MCU APP 头 (含版本 / CRC32)
 *    2) 校验失败 → 从 backup 恢复
 *    3) backup 也失败 → 从 factory 恢复
 *===========================================================================*/
#define W25Q64_REGION_SIZE       (256UL * 1024UL)           /* 每段 256KB */

/* 区域偏移 (24-bit 地址空间, 0x000000 ~ 0x7FFFFF) */
#define W25Q64_TEMP_OFFSET       0x000000UL                /* temp 段:   0x000000 - 0x03FFFF */
#define W25Q64_BACKUP_OFFSET     0x040000UL                /* backup 段: 0x040000 - 0x07FFFF */
#define W25Q64_FACTORY_OFFSET    0x080000UL                /* factory 段:0x080000 - 0x0BFFFF */
#define W25Q64_USERDATA_OFFSET   0x0C0000UL                /* 用户数据:  0x0C0000 - 0x7FFFFF (7.5MB) */

/* 三段区域头 (每段开头 64 字节, 用于元数据) */
#define W25Q64_REGION_HEADER_SIZE  64
#define W25Q64_HDR_MAGIC_OFFSET    0x00    /* 4B Magic (大端) */
#define W25Q64_HDR_SIZE_OFFSET     0x04    /* 4B 数据长度 (大端, 0 表示空段) */
#define W25Q64_HDR_CRC32_OFFSET    0x08    /* 4B CRC32 (大端) */
#define W25Q64_HDR_VERSION_OFFSET  0x0C    /* 16B 版本字符串 (UTF-8, 0 填充) */
#define W25Q64_HDR_TIMESTAMP_OFFSET 0x1C   /* 4B Unix 时间戳 (大端, 备用) */

/* 三段 MAGIC 值 */
#define W25Q64_MAGIC_TEMP         0x57494E54UL   /* "TEMP" */
#define W25Q64_MAGIC_BACKUP       0x42434B55UL   /* "BCKU" */
#define W25Q64_MAGIC_FACTORY      0x46414354UL   /* "FACT" */
#define W25Q64_MAGIC_EMPTY        0xFFFFFFFFUL   /* 空白 (Flash 默认值) */

/*============================================================================
 * W25Q64 函数 API (BL 内部调用, 都阻塞)
 *  返回: 0 = 成功, -1 = 失败
 *===========================================================================*/

/* 初始化 QUADSPI + W25Q64
 *  - 配 PG6=CS, PF10=CLK, PF8=IO0(MOSI), PF9=IO1(MISO)
 *  - 单线 SPI 模式 (因 IO2/IO3 未接)
 *  - 时钟: APB2 (假设 100MHz) / 4 = 25MHz (W25Q64 上限 104MHz, 安全) */
int  w25q64_init(void);

/* 读 JEDEC ID (3 字节: mfg, mem_msb, mem_lsb) */
int  w25q64_read_jedec_id(uint8_t *mfg, uint8_t *mem_msb, uint8_t *mem_lsb);

/* Modify 2026.6.24 v16 P1.5: 返回 init 时缓存的真实 JEDEC (bootloader 打印用) */
int  w25q64_get_jedec(uint8_t *mfg, uint8_t *mem_msb, uint8_t *mem_lsb);

/* 读数据 (低速, 无 dummy cycle) */
int  w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len);

/* 页编程 (≤256B, 跨页自动分多次) */
int  w25q64_page_program(uint32_t addr, const uint8_t *buf, uint32_t len);

/* 扇区擦 (4KB) — addr 必须 4KB 对齐 */
int  w25q64_sector_erase(uint32_t addr);

/* 块擦 (64KB) — addr 必须 64KB 对齐 */
int  w25q64_block64_erase(uint32_t addr);

/*============================================================================
 * 三段保险高层 API
 *===========================================================================*/

/* 写固件到指定段 (自动擦 + 写 + 写头)
 *  offset: 段起始地址 (W25Q64_TEMP_OFFSET / BACKUP_OFFSET / FACTORY_OFFSET)
 *  magic:  段标识 (W25Q64_MAGIC_TEMP / BACKUP / FACTORY)
 *  data:   固件 bin 数据
 *  size:   固件大小 (≤ W25Q64_REGION_SIZE - 64)
 *  返回: 0 = 成功, -1 = 擦失败, -2 = 写失败, -3 = 校验失败 */
int  w25q64_region_write(uint32_t offset, uint32_t magic,
                          const uint8_t *data, uint32_t size,
                          const char *version_str);

/* 从指定段读出固件 (跳过 64B 头)
 *  返回: 读出字节数, -1 = 段空 / magic 不匹配, -2 = CRC 校验失败 */
int  w25q64_region_read(uint32_t offset, uint32_t expected_magic,
                         uint8_t *buf, uint32_t buf_size);

/*============================================================================
 * 内部辅助 (调试用, 一般不直接调)
 *===========================================================================*/

/* 等 W25Q64 BUSY 位清零 (擦/写操作完成后必须调)
 *  timeout_ms: 超时时间 (毫秒), 0 = 默认 5000ms */
int  w25q64_wait_busy(uint32_t timeout_ms);

/* 写使能 (每次擦/写操作前必须调) */
int  w25q64_write_enable(void);

/* 读状态寄存器 1 */
int  w25q64_read_status1(uint8_t *sr);

#endif /* __W25Q64_H__ */