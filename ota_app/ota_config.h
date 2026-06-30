#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H
/*****************************************************************************
 * ota_config.h — OTA upgrade parameters for AMKN8639 APP side
 *****************************************************************************/
#include <stdint.h>

/* ========== Network ========== */
#define OTA_HTTP_TIMEOUT_MS     30000   /* HTTP connect/read timeout */
#define OTA_HTTP_RETRY_MAX      3       /* Max retries on failure */
#define OTA_HTTP_CHUNK_SIZE     4096    /* Download chunk size */

/* ========== External Flash (W25Q64 via SPI) ========== */
#define OTA_EXT_FLASH_OFFSET    0x000000  /* Slot B start address */
#define OTA_EXT_FLASH_SECTOR    0x1000    /* 4KB sector size */

/* ========== Internal Flash ========== */
#define OTA_STATUS_BASE         0x081A0000
#define OTA_MAGIC_FLAG          0x4F544100  /* "OTA\0" */

/* ========== Startup ========== */
#define OTA_CONFIRM_TIMEOUT_MS  60000   /* APP must confirm within 60s */
#define OTA_MAX_BOOT_ATTEMPTS   3       /* Max retry before fallback */

/* ========== AT Command ========== */
#define OTA_AT_PREFIX           "AT+OTA"

/* ========== Phase enum ========== */
typedef enum {
    OTA_PHASE_IDLE = 0,
    OTA_PHASE_DOWNLOADING,
    OTA_PHASE_VERIFYING,
    OTA_PHASE_READY,
    OTA_PHASE_ERROR,
} ota_phase_t;

/* ========== Progress callback ========== */
typedef void (*ota_progress_cb_t)(ota_phase_t phase, uint32_t progress_pct, const char *msg);

/* ========== OTA download request ========== */
typedef struct {
    const char         *url;           /* HTTP URL to firmware bin */
    uint32_t            expected_size; /* Expected file size (0 = unknown) */
    const uint8_t      *expected_sha256; /* Expected SHA256 (NULL = skip check) */
    ota_progress_cb_t   progress_cb;   /* Progress callback */
} ota_request_t;

#endif
