#ifndef OTA_AT_H
#define OTA_AT_H
/*****************************************************************************
 * ota_at.h — AT+OTA command handler for AMKN8639
 * 
 * Integrate into libapp/at.c: add calls to ota_at_dispatch() in AT parser.
 * 
 * Commands:
 *   AT+OTA=START,<url>,<size>,<sha256>   → start upgrade
 *   AT+OTA=STATUS?                       → query progress
 *   AT+OTA=ABORT                         → cancel upgrade
 *   AT+OTA=COMMIT                        → confirm & reboot
 *   AT+OTA=ROLLBACK                      → factory reset
 *   AT+OTA=VERSION?                      → query firmware version
 * 
 * Responses:
 *   +OTA:START,OK
 *   +OTA:START,ERR,<code>
 *   +OTA:STATUS,<phase>,<pct>
 *   +OTA:ABORT,OK
 *   +OTA:COMMIT,OK
 *   +OTA:ROLLBACK,OK
 *   +OTA:VERSION,<ver>
 *   +OTA:ERROR,<msg>
 *   +OTA:PROGRESS,<pct>
 *****************************************************************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Dispatch AT+OTA command.
 * @param cmd   Full AT command string (e.g. "AT+OTA=START,http://...")
 * @param resp  Output response buffer (must be pre-allocated, >= 128 bytes)
 * @param len   Response buffer size
 * @return 0 = handled, -1 = not an OTA command
 */
int ota_at_dispatch(const char *cmd, char *resp, int len);

/*
 * Progress callback from OTA client → formats AT response.
 * Called by ota_client during download/verify.
 */
void ota_at_progress(ota_phase_t phase, uint32_t pct, const char *msg);

#ifdef __cplusplus
}
#endif

#endif
