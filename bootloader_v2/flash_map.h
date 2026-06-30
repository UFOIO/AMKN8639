/* AMKN8639 Flash Map - MCUBoot compatible
 * STM32H743XIH6: 2MB internal Flash
 * W25Q64: 8MB external SPI Flash
 */
#ifndef FLASH_MAP_H
#define FLASH_MAP_H

#include <stdint.h>

/* === Internal Flash (STM32H743, 2MB) === */
#define FLASH_BASE              0x08000000
#define FLASH_SIZE              (2048 * 1024)
#define FLASH_SECTOR_SIZE       (128 * 1024)
#define FLASH_PAGE_SIZE         256

/* Bootloader: 128KB, Bank1 Sector 0 */
#define BL_BASE                 0x08000000
#define BL_SIZE                 (128 * 1024)

/* Slot A (APP): 1536KB, Bank1 S1-S7 + Bank2 S0-S4 */
#define SLOT_A_BASE             0x08020000
#define SLOT_A_SIZE             (1536 * 1024)

/* OTA Status Area: 128KB, Bank2 Sector 5 */
#define OTA_STATUS_BASE         0x081A0000
#define OTA_STATUS_SIZE         (128 * 1024)

/* User Params (EEPROM emulation): 128KB, Bank2 Sector 6 */
#define USER_PARAM_BASE         0x081C0000
#define USER_PARAM_SIZE         (128 * 1024)

/* Reserved: 128KB, Bank2 Sector 7 */
#define RESERVED_BASE           0x081E0000
#define RESERVED_SIZE           (128 * 1024)

/* === External SPI Flash (W25Q64, 8MB) === */
#define EXT_FLASH_SIZE          (8 * 1024 * 1024)
#define EXT_SECTOR_SIZE         (4 * 1024)

/* Slot B (Download buffer): 2MB */
#define SLOT_B_OFFSET           0x000000
#define SLOT_B_SIZE             (2 * 1024 * 1024)

/* Golden Image (Factory backup): 2MB */
#define GOLDEN_OFFSET           0x200000
#define GOLDEN_SIZE             (2 * 1024 * 1024)

/* FatFS area: 4MB */
#define FATFS_OFFSET            0x400000
#define FATFS_SIZE              (4 * 1024 * 1024)

/* === OTA Status Structure (at OTA_STATUS_BASE) === */
typedef struct __attribute__((packed)) {
    uint32_t magic;          /* 0x4F544148 = "OTA\0" ... actually "OTAH" */
    uint32_t boot_status;    /* 0=IDLE, 1=COPYING, 2=VERIFYING, 3=DONE */
    uint32_t slot_b_size;    /* Image size in Slot B */
    uint32_t sec_counter;    /* Monotonic anti-rollback counter */
    uint8_t  new_sha256[32]; /* New firmware SHA256 */
    uint32_t boot_attempts;  /* Retry counter */
    uint32_t crc32;          /* Structure self-check */
} ota_status_t;

#define OTA_MAGIC               0x4F544148  /* "OTAH" */
#define OTA_STATUS_IDLE         0
#define OTA_STATUS_COPYING      1
#define OTA_STATUS_VERIFYING    2
#define OTA_STATUS_DONE         3

/* === Boot Decision === */
typedef enum {
    BOOT_SLOT_A = 0,
    BOOT_SWAP_B_TO_A,
    BOOT_NO_APP,
} boot_decision_t;

#endif /* FLASH_MAP_H */
