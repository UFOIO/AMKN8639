/************************************************************************************
*  File name: bootloader.h
*  Project  : AMKN8639 self-developed Bootloader
*  Description: Bootloader main header (Phase 3.2)
*                - Run on STM32H743XI, linked at 0x08000000, max 64KB
*                - APP linked at 0x08010000
*
*  Modify History:
*   1. Version: 1.0
*      Date:    2026.6.22
*      Modify:  Phase 3.2 - create file (header)
*
*************************************************************************************/
#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include <stdint.h>

/* 类型定义 (避免依赖 const.h) */
typedef uint8_t  INT8U;
typedef uint16_t INT16U;
typedef uint32_t INT32U;
typedef int32_t  INT32S;

/*============================================================================
 * Flash layout
 *  Bootloader : 0x08000000 - 0x0800FFFF  (64KB)
 *  APP        : 0x08010000 - 0x080FFFFF  (960KB)
 *===========================================================================*/
#define BOOTLOADER_START_ADDR     0x08000000UL
#define BOOTLOADER_SIZE           (64UL * 1024UL)      /* 64KB */
#define APP_START_ADDR            0x08010000UL
#define APP_MAX_SIZE              (960UL * 1024UL)     /* 960KB */

/*============================================================================
 * DI1 (PD11) upgrade trigger - long press 3 seconds
 *  PD11 is the hardware upgrade button (per AMKN8639_IOConfig.h)
 *  Active LOW (NC = normally closed), so press = 0
 *===========================================================================*/
#define DI1_PORT                  PIOD
#define DI1_PIN                   Px11
#define DI1_GPIO_ID               ((((INT32U)DI1_PORT) << 16) | (INT32U)DI1_PIN)
#define DI1_LONG_PRESS_MS         3000

/*============================================================================
 * EEPROM upgrade flag (AT24C64, 8KB, I2C1 @ 0xA0)
 *  Address 0x0000-0x0003: 4 bytes legacy 1-byte flag (kept for backward compat)
 *  Address 0x0010-0x0013: 4 bytes UPGRADE_REQ_MAGIC (大端序)
 *  Address 0x0014-0x0017: 4 bytes UPGRADE_DONE_MAGIC (大端序)
 *  Address 0x0018-0x001B: 4 bytes expected CRC32 of new APP (BE)
 *===========================================================================*/
#define EEPROM_UPGRADE_FLAG_ADDR  0x0000
#define EEPROM_UPGRADE_FLAG_VAL   0xAA
#define EEPROM_NORMAL_FLAG_VAL    0x00

#define EEPROM_REQ_MAGIC_ADDR     0x0010
#define EEPROM_DONE_MAGIC_ADDR    0x0014
#define EEPROM_CRC32_ADDR         0x0018

#define EEPROM_BLANK_VAL          0xFF   /* AT24C64 erased value */

/* 双 UART 升级模式状态机 (Modify 2026.6.23) */
#define BL_UPGRADE_REQ_MAGIC      0x12345678UL   /* 收到 UPGRADE_REQ, 等 UART1 数据 */
#define BL_UPGRADE_DONE_MAGIC     0x87654321UL   /* UART1 数据收完, 准备跳 APP */

/*============================================================================
 * 双 UART 设计 (Modify 2026.6.23 — 用户最终决策)
 *   - UART3 (PB10/PB11) = 命令通道 (短帧, 握手 + ACK)
 *   - UART1 (PA9/PA10)  = 数据通道 (流式, 大数据量)
 *  流程: UART3 收到 UPGRADE_REQ → ACK → 软复位 → UART1 流式接收 → 软复位 → 跳 APP
 *  兼容: DI1 长按 3s 走 UART3 单 UART 全双工 (v1 升级流程)
 *===========================================================================*/

/* UART3 命令通道 (115200 8N1, 复用现状协议)
 *  帧:  [AA 55] [LEN] [CMD] [DATA...] [CRC8]
 *    AA 55   : Header (2 bytes)
 *    LEN     : CMD(1) + DATA(N) + CRC8(1), 1 byte, max 255
 *    CMD     : Command, 1 byte
 *    DATA    : Optional
 *    CRC8    : XOR of [LEN+CMD+DATA], 1 byte
 */
#define UPGRADE_FRAME_HEADER0     0xAA
#define UPGRADE_FRAME_HEADER1     0x55
#define UPGRADE_RX_BUF_SIZE       1024
#define UPGRADE_TIMEOUT_MS        5000   /* 5s no data -> abort */

/* Upgrade commands (DeepSeek 大胆版 v1, 6 条) */
#define UPGRADE_CMD_START         0x01   /* Erase APP flash, prepare to receive */
#define UPGRADE_CMD_DATA          0x02   /* Write data block */
#define UPGRADE_CMD_VERIFY        0x03   /* Verify (CRC32, placeholder v1) */
#define UPGRADE_CMD_COMMIT        0x04   /* Commit, clear EEPROM flag, jump APP */
#define UPGRADE_CMD_ABORT         0x05   /* Abort, fall through to APP */
#define UPGRADE_CMD_STATUS        0x06   /* Query BL status */
#define UPGRADE_CMD_ACK           0x80   /* ACK reply (from BL) */
#define UPGRADE_CMD_NACK          0x81   /* NACK reply (from BL) */

/* Modify 2026.6.23: 双 UART 新增命令 */
#define UPGRADE_CMD_DUAL_REQ      0x10   /* 双 UART 升级请求, DATA=[size:4B LE][crc32:4B LE] */
#define UPGRADE_CMD_DUAL_QUERY    0x11   /* 查询双 UART 状态 */
#define UPGRADE_CMD_DUAL_ABORT    0x12   /* 中止双 UART 升级 (清 EEPROM magic) */

/* UART1 数据通道 (115200 8N1, Modify 2026.6.23)
 *  帧:  [CC DD] [SEQ:2B BE] [LEN:2B BE] [DATA:N] [CRC32:4B LE]
 *    CC DD   : Header (2 bytes)
 *    SEQ     : 帧序号 (0, 1, 2, ...; 0xFFFF = 最后一帧, DATA=空)
 *    LEN     : DATA 长度 (0..1024), 2 bytes BE
 *    DATA    : 0..1024 字节
 *    CRC32   : IEEE CRC32 of [SEQ+LEN+DATA], 4 bytes LE
 */
#define DATA_FRAME_HEADER0        0xCC
#define DATA_FRAME_HEADER1        0xDD
#define DATA_FRAME_LAST_SEQ       0xFFFF
#define DATA_FRAME_MAX_LEN        1024
#define DATA_FRAME_OVERHEAD       10      /* HDR(2) + SEQ(2) + LEN(2) + CRC32(4) */

/*============================================================================
 * Function prototypes
 *===========================================================================*/
void bl_main(void);
void bl_jump_to_app(uint32_t app_addr);
void bl_delay_ms(uint32_t ms);
typedef void (*bl_app_entry_t)(void);

#endif /* __BOOTLOADER_H__ */
