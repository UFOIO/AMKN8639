#ifndef OTA_CLIENT_H
#define OTA_CLIENT_H
/*****************************************************************************
 * ota_client.h — OTA firmware download & upgrade client (APP side)
 * 
 * Runs in FreeRTOS task context. Uses LWIP sockets for HTTP download.
 *****************************************************************************/
#include <stdint.h>
#include "ota_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== Public API ========== */

/*
 * Initialize OTA client. Must be called once from FreeRTOS task init.
 * @return 0 on success, -1 on failure
 */
int ota_client_init(void);

/*
 * Start OTA download from HTTP URL.
 * Blocking: downloads in background via state machine, progress via callback.
 * Call ota_client_poll() periodically to drive the state machine.
 * 
 * @param req  Download request (URL, expected size, SHA256, callback)
 * @return 0 on queued, -1 if busy
 */
int ota_start(const ota_request_t *req);

/*
 * Poll OTA state machine. Call from FreeRTOS task loop.
 * Returns current progress percentage (0-100), or -1 on idle.
 */
int ota_poll(void);

/*
 * Abort ongoing download.
 * @return 0 on success, -1 if not downloading
 */
int ota_abort(void);

/*
 * Commit upgrade: write magic flag → prepare for reboot.
 * Call after VERIFY_OK and user sends AT+OTA=COMMIT.
 * @return 0 on success, -1 on error
 */
int ota_commit(void);

/*
 * Get current phase and progress.
 * @param phase  Output: current phase
 * @param pct    Output: 0-100 progress
 * @return 0 on success
 */
int ota_status(ota_phase_t *phase, uint32_t *pct);

/*
 * Get current firmware version string.
 * @param buf   Output buffer
 * @param len   Buffer size
 * @return version string pointer (=buf)
 */
const char *ota_version(char *buf, uint32_t len);

/*
 * Rollback to factory golden image.
 * Writes rollback flag → reboots → BL handles recovery.
 * @return 0 on flag written, -1 on error
 */
int ota_rollback(void);

/*
 * Confirm successful boot (call after APP runs normally for OTA_CONFIRM_TIMEOUT_MS).
 * Marks Slot A as permanent, prevents rollback.
 */
void ota_confirm_boot(void);

#ifdef __cplusplus
}
#endif

#endif
