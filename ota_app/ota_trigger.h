#ifndef OTA_TRIGGER_H
#define OTA_TRIGGER_H
/*
 * APP-side OTA trigger for AMKN8639
 * Include this header in your APP project to enable OTA firmware update.
 *
 * Usage:
 *   1. Call ota_request_update() to reboot into bootloader XCP mode.
 *   2. BL detects BKP magic and stays in XCP mode (doesn't auto-boot APP).
 *   3. Host tool connects via UART and flashes new firmware.
 *
 * Hardware dependency:
 *   - RTC backup register BKP0R at 0x58004000 (survives system reset).
 */

#include <stdint.h>

/* RTC Backup Register 0 */
#define RTC_BKP0R  (*(volatile uint32_t*)0x58004000)

/* Magic value: APP requests bootloader to stay in XCP mode */
#define BKP_MAGIC_OTA  0x424C4F54  /* "BLOT" */

/* System reset via NVIC */
#define SCB_AIRCR  (*(volatile uint32_t*)0xE000ED0C)
#define AIRCR_SYSRESET  (0x05FA0000 | (1<<2))

/**
 * @brief Request OTA firmware update.
 *        Writes magic to backup register then triggers system reset.
 *        On next boot, BL detects the magic and enters XCP mode
 *        instead of auto-booting APP.
 *
 * @note  This function does NOT return (system reset).
 */
static inline void ota_request_update(void)
{
    /* Disable interrupts during critical section */
    __disable_irq();

    /* Write magic to backup register (survives reset) */
    RTC_BKP0R = BKP_MAGIC_OTA;

    /* Memory barrier to ensure write completes before reset */
    __DSB();
    __ISB();

    /* Trigger system reset */
    SCB_AIRCR = AIRCR_SYSRESET;
    __DSB();

    /* Wait for reset */
    while(1) { __WFI(); }
}

/**
 * @brief Request bootloader self-upgrade mode.
 *        On next boot, BL enters self-upgrade mode to replace itself.
 */
#define BKP_MAGIC_BL_UPGRADE  0x424C5550  /* "BLUP" */

static inline void ota_request_bl_upgrade(void)
{
    __disable_irq();
    RTC_BKP0R = BKP_MAGIC_BL_UPGRADE;
    __DSB(); __ISB();
    SCB_AIRCR = AIRCR_SYSRESET;
    __DSB();
    while(1) { __WFI(); }
}

/**
 * @brief Call this from your AT command handler:
 *        AT+OTA   -> triggers firmware update mode
 *        AT+OTA=BL -> triggers BL update mode
 */
static inline void ota_at_handler(const char *cmd)
{
    /* Simple AT command parsing */
    if (cmd[0] == 'O' && cmd[1] == 'T' && cmd[2] == 'A') {
        if (cmd[3] == '=' && cmd[4] == 'B' && cmd[5] == 'L') {
            ota_request_bl_upgrade();
        } else {
            ota_request_update();
        }
    }
}

#endif /* OTA_TRIGGER_H */
