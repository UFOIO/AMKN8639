/*****************************************************************************
 * ota_client.c — OTA firmware download & upgrade client (APP side)
 * 
 * Integration notes:
 *   - Replace W25QXX_xxx() calls with project's spiflash_app.c API
 *   - Replace lwip_xxx() with project's LWIP socket API
 *   - Replace FLASH_xxx() with project's internal flash API
 *   - Runs in FreeRTOS task: call ota_poll() in task loop
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include "ota_client.h"
#include "ota_config.h"

/* ========== External API stubs — REPLACE with project headers ========== */
/* W25QXX SPI Flash (from spiflash_app.c) */
extern int  W25QXX_Init(void);
extern int  W25QXX_Read(uint32_t addr, uint8_t *buf, uint32_t len);
extern int  W25QXX_Write_Page(uint32_t addr, const uint8_t *buf, uint32_t len);
extern int  W25QXX_Erase_Sector(uint32_t addr);
extern int  W25QXX_Erase_Chip(void);

/* Internal Flash */
extern int  FLASH_EraseSector(uint32_t sector);
extern int  FLASH_ProgramWord(uint32_t addr, uint32_t data);

/* LWIP Sockets */
extern int  lwip_socket(int domain, int type, int protocol);
extern int  lwip_connect(int s, const void *addr, int addrlen);
extern int  lwip_send(int s, const void *data, int len, int flags);
extern int  lwip_recv(int s, void *buf, int len, int flags);
extern int  lwip_close(int s);
extern int  lwip_gethostbyname(const char *name, uint32_t *ip);

/* FreeRTOS */
extern void vTaskDelay(uint32_t ticks);
extern uint32_t xTaskGetTickCount(void);

/* UART debug */
extern void uart_printf(const char *fmt, ...);

/* ========== Constants ========== */
#define OTA_STATE_IDLE          0
#define OTA_STATE_DOWNLOADING   1
#define OTA_STATE_VERIFYING     2
#define OTA_STATE_READY         3
#define OTA_STATE_COMMIT        4
#define OTA_STATE_ERROR         5

#define HTTP_PORT               80
#define HTTP_RECV_BUF_SIZE      4096
#define HTTP_REQ_TEMPLATE       "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n"

/* OTA Status Area Flags */
#define OTA_STATUS_BASE_ADDR    0x081A0000UL
#define OTA_FLAG_MAGIC          0x4F544100UL

/* ========== Internal State ========== */
typedef struct {
    /* Request */
    ota_request_t   req;

    /* State */
    uint8_t         state;          /* OTA_STATE_xxx */
    uint8_t         is_busy;
    uint32_t        total_bytes;
    uint32_t        received_bytes;
    uint32_t        flash_offset;
    uint32_t        last_sector;

    /* HTTP */
    int             sock;
    uint8_t         recv_buf[HTTP_RECV_BUF_SIZE];
    uint32_t        recv_off;
    uint8_t         header_done;

    /* Verification */
    uint8_t         sha256_ctx[256];    /* Placeholder for SHA256 context */
    uint32_t        crc32_accum;

    /* Error */
    int             error_code;
    char            error_msg[64];
} ota_ctx_t;

static ota_ctx_t g_ota;

/* ========== CRC32 ========== */
static uint32_t crc32_table[256];
static int crc32_table_init_done = 0;

static void crc32_init_table(void) {
    uint32_t i, j, crc;
    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320UL : 0);
        crc32_table[i] = crc;
    }
    crc32_table_init_done = 1;
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *data, uint32_t len) {
    uint32_t i;
    if (!crc32_table_init_done) crc32_init_table();
    crc ^= 0xFFFFFFFFUL;
    for (i = 0; i < len; i++)
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFUL;
}

/* ========== URL Parsing ========== */
static int parse_url(const char *url, char *host, int host_len,
                     char *path, int path_len, uint16_t *port) {
    const char *p, *start;
    int len;

    /* Skip http:// */
    if (strncmp(url, "http://", 7) == 0) url += 7;

    /* Host */
    p = url;
    while (*p && *p != ':' && *p != '/') p++;
    len = (int)(p - url);
    if (len >= host_len) return -1;
    memcpy(host, url, len);
    host[len] = '\0';

    /* Port */
    if (*p == ':') {
        p++;
        *port = 0;
        while (*p >= '0' && *p <= '9') { *port = *port * 10 + (*p - '0'); p++; }
    } else {
        *port = HTTP_PORT;
    }

    /* Path */
    if (*p == '\0') {
        strcpy(path, "/");
    } else {
        len = (int)strlen(p);
        if (len >= path_len) return -1;
        strcpy(path, p);
    }

    return 0;
}

/* ========== HTTP Download ========== */
static int http_connect(const char *host, uint16_t port) {
    uint32_t ip_addr;
    struct sockaddr_in {
        uint8_t  sin_len;
        uint8_t  sin_family;
        uint16_t sin_port;
        uint32_t sin_addr;
        uint8_t  sin_zero[8];
    } addr;

    /* DNS resolve */
    if (lwip_gethostbyname(host, &ip_addr) != 0) {
        strcpy(g_ota.error_msg, "DNS FAIL");
        return -1;
    }

    /* Socket */
    g_ota.sock = lwip_socket(2, 1, 0);  /* AF_INET, SOCK_STREAM */
    if (g_ota.sock < 0) {
        strcpy(g_ota.error_msg, "SOCK FAIL");
        return -2;
    }

    /* Connect */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = 2;  /* AF_INET */
    addr.sin_port = ((port & 0xFF) << 8) | ((port >> 8) & 0xFF);  /* htons */
    addr.sin_addr = ip_addr;

    if (lwip_connect(g_ota.sock, &addr, sizeof(addr)) != 0) {
        lwip_close(g_ota.sock);
        g_ota.sock = -1;
        strcpy(g_ota.error_msg, "CONN FAIL");
        return -3;
    }

    return 0;
}

static void http_disconnect(void) {
    if (g_ota.sock >= 0) {
        lwip_close(g_ota.sock);
        g_ota.sock = -1;
    }
}

static int http_send_request(const char *host, const char *path) {
    char req[512];
    int len, sent;
    len = snprintf(req, sizeof(req), HTTP_REQ_TEMPLATE, path, host);
    sent = lwip_send(g_ota.sock, req, len, 0);
    return (sent == len) ? 0 : -1;
}

static int http_skip_headers(void) {
    /* Find \r\n\r\n in recv buffer */
    uint32_t i;
    for (i = 0; i + 3 < g_ota.recv_off; i++) {
        if (g_ota.recv_buf[i] == '\r' && g_ota.recv_buf[i+1] == '\n' &&
            g_ota.recv_buf[i+2] == '\r' && g_ota.recv_buf[i+3] == '\n') {
            uint32_t header_end = i + 4;
            uint32_t body_len = g_ota.recv_off - header_end;

            /* Check for HTTP 200 */
            if (strncmp((const char *)g_ota.recv_buf, "HTTP/1.", 7) != 0) return -1;

            /* Move body to front */
            if (body_len > 0) memmove(g_ota.recv_buf, g_ota.recv_buf + header_end, body_len);
            g_ota.recv_off = body_len;
            g_ota.header_done = 1;
            return 0;
        }
    }

    /* Header not complete — check for buffer overflow */
    if (g_ota.recv_off >= HTTP_RECV_BUF_SIZE - 1) return -1;
    return -2;  /* Need more data */
}

/* ========== Flash Write (External SPI) ========== */
static int ext_flash_write(uint32_t offset, const uint8_t *data, uint32_t len) {
    /* Erase sector if crossing boundary */
    uint32_t new_sector = offset / OTA_EXT_FLASH_SECTOR;
    if (new_sector != g_ota.last_sector || offset == 0) {
        W25QXX_Erase_Sector(new_sector * OTA_EXT_FLASH_SECTOR);
        g_ota.last_sector = new_sector;
    }

    /* Page program (256 bytes max per W25Q page) */
    while (len > 0) {
        uint32_t page_off = offset & 0xFF;
        uint32_t chunk = 256 - page_off;
        if (chunk > len) chunk = len;
        W25QXX_Write_Page(offset + OTA_EXT_FLASH_OFFSET, data, chunk);
        offset += chunk;
        data += chunk;
        len -= chunk;
    }
    return 0;
}

/* ========== OTA State Machine Steps ========== */
static void ota_step_idle(void) {
    /* Nothing */
}

static int ota_step_start_download(void) {
    char host[128], path[256];
    uint16_t port;

    if (parse_url(g_ota.req.url, host, sizeof(host), path, sizeof(path), &port) != 0) {
        g_ota.state = OTA_STATE_ERROR;
        g_ota.error_code = -1;
        strcpy(g_ota.error_msg, "BAD URL");
        return -1;
    }

    uart_printf("OTA: host=%s path=%s port=%d\r\n", host, path, port);

    if (http_connect(host, port) != 0) {
        g_ota.state = OTA_STATE_ERROR;
        g_ota.error_code = -2;
        return -2;
    }

    if (http_send_request(host, path) != 0) {
        http_disconnect();
        g_ota.state = OTA_STATE_ERROR;
        g_ota.error_code = -3;
        strcpy(g_ota.error_msg, "SEND FAIL");
        return -3;
    }

    g_ota.state = OTA_STATE_DOWNLOADING;
    g_ota.received_bytes = 0;
    g_ota.flash_offset = 0;
    g_ota.recv_off = 0;
    g_ota.header_done = 0;
    g_ota.crc32_accum = 0;
    g_ota.last_sector = 0xFFFFFFFF;

    if (g_ota.req.progress_cb)
        g_ota.req.progress_cb(OTA_PHASE_DOWNLOADING, 0, "Downloading...");

    return 0;
}

static void ota_step_downloading(void) {
    int recv_len;

    /* Receive data */
    recv_len = lwip_recv(g_ota.sock, g_ota.recv_buf + g_ota.recv_off,
                          HTTP_RECV_BUF_SIZE - g_ota.recv_off, 0);

    if (recv_len < 0) {
        http_disconnect();
        g_ota.state = OTA_STATE_ERROR;
        g_ota.error_code = -4;
        strcpy(g_ota.error_msg, "RECV FAIL");
        return;
    }

    g_ota.recv_off += recv_len;

    if (recv_len == 0) {
        /* Connection closed — download complete */
        http_disconnect();
        g_ota.state = OTA_STATE_VERIFYING;
        g_ota.total_bytes = g_ota.received_bytes;
        if (g_ota.req.progress_cb)
            g_ota.req.progress_cb(OTA_PHASE_VERIFYING, 100, "Download complete");
        return;
    }

    /* Parse headers if not done */
    if (!g_ota.header_done) {
        int rc = http_skip_headers();
        if (rc == -1) {
            http_disconnect();
            g_ota.state = OTA_STATE_ERROR;
            g_ota.error_code = -5;
            strcpy(g_ota.error_msg, "HTTP ERR");
            return;
        }
        if (rc == -2) return;  /* Need more header data */
    }

    /* Write received body to external flash */
    if (g_ota.header_done && g_ota.recv_off > 0) {
        /* CRC accumulate */
        g_ota.crc32_accum = crc32_update(g_ota.crc32_accum,
                                          g_ota.recv_buf, g_ota.recv_off);

        /* Write to external flash */
        ext_flash_write(g_ota.flash_offset, g_ota.recv_buf, g_ota.recv_off);
        g_ota.flash_offset += g_ota.recv_off;
        g_ota.received_bytes += g_ota.recv_off;
        g_ota.recv_off = 0;

        /* Progress */
        if (g_ota.req.progress_cb) {
            uint32_t pct = g_ota.req.expected_size > 0 ?
                           (g_ota.received_bytes * 100 / g_ota.req.expected_size) : 0;
            if (pct > 100) pct = 99;
            g_ota.req.progress_cb(OTA_PHASE_DOWNLOADING, pct, 0);
        }
    }
}

static void ota_step_verifying(void) {
    /* Verify expected size */
    if (g_ota.req.expected_size > 0 &&
        g_ota.received_bytes != g_ota.req.expected_size) {
        g_ota.state = OTA_STATE_ERROR;
        g_ota.error_code = -6;
        strcpy(g_ota.error_msg, "SIZE MISMATCH");
        if (g_ota.req.progress_cb)
            g_ota.req.progress_cb(OTA_PHASE_ERROR, 0, "Size mismatch");
        return;
    }

    /* Read back flash and compute CRC32 */
    {
        uint8_t buf[512];
        uint32_t off = 0, remaining = g_ota.received_bytes;
        uint32_t read_crc = 0;

        while (remaining > 0) {
            uint32_t chunk = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
            W25QXX_Read(OTA_EXT_FLASH_OFFSET + off, buf, chunk);
            read_crc = crc32_update(read_crc, buf, chunk);
            off += chunk;
            remaining -= chunk;
        }

        if (read_crc != g_ota.crc32_accum) {
            g_ota.state = OTA_STATE_ERROR;
            g_ota.error_code = -7;
            strcpy(g_ota.error_msg, "CRC FAIL");
            if (g_ota.req.progress_cb)
                g_ota.req.progress_cb(OTA_PHASE_ERROR, 0, "CRC verification failed");
            return;
        }
    }

    g_ota.state = OTA_STATE_READY;
    if (g_ota.req.progress_cb)
        g_ota.req.progress_cb(OTA_PHASE_READY, 100, "Verification OK, ready to commit");
}

static void ota_step_commit(void) {
    /* Write magic flag to OTA status area */
    uint32_t magic = OTA_FLAG_MAGIC;
    uint32_t addr = OTA_STATUS_BASE_ADDR;

    /* Erase OTA status sector */
    FLASH_EraseSector(13);  /* Bank2 Sector 5 */

    /* Write magic flag */
    FLASH_ProgramWord(addr, magic);

    /* Write image size */
    FLASH_ProgramWord(addr + 8, g_ota.received_bytes);

    /* Write CRC32 */
    FLASH_ProgramWord(addr + 0x34, g_ota.crc32_accum);

    g_ota.state = OTA_STATE_IDLE;
    g_ota.is_busy = 0;

    uart_printf("OTA: commit done, reboot now\r\n");
}

/* ========== Public API ========== */
int ota_client_init(void) {
    memset(&g_ota, 0, sizeof(g_ota));
    g_ota.state = OTA_STATE_IDLE;
    g_ota.is_busy = 0;
    g_ota.sock = -1;

    crc32_init_table();
    W25QXX_Init();

    return 0;
}

int ota_start(const ota_request_t *req) {
    if (g_ota.is_busy) return -1;
    if (!req || !req->url) return -2;

    memset(&g_ota, 0, sizeof(g_ota));
    memcpy(&g_ota.req, req, sizeof(ota_request_t));

    g_ota.state = OTA_STATE_DOWNLOADING;
    g_ota.is_busy = 1;
    g_ota.sock = -1;

    /* Initiate connection */
    if (ota_step_start_download() != 0) {
        g_ota.is_busy = 0;
        return g_ota.error_code;
    }

    return 0;
}

int ota_poll(void) {
    if (!g_ota.is_busy) return -1;

    switch (g_ota.state) {
    case OTA_STATE_DOWNLOADING:
        ota_step_downloading();
        break;
    case OTA_STATE_VERIFYING:
        ota_step_verifying();
        break;
    case OTA_STATE_COMMIT:
        ota_step_commit();
        break;
    case OTA_STATE_ERROR:
        if (g_ota.req.progress_cb)
            g_ota.req.progress_cb(OTA_PHASE_ERROR, 0, g_ota.error_msg);
        http_disconnect();
        g_ota.is_busy = 0;
        return g_ota.error_code;
    }

    return (g_ota.state == OTA_STATE_READY) ? 100 : 0;
}

int ota_abort(void) {
    if (!g_ota.is_busy) return -1;
    http_disconnect();
    g_ota.state = OTA_STATE_IDLE;
    g_ota.is_busy = 0;
    return 0;
}

int ota_commit(void) {
    if (g_ota.state != OTA_STATE_READY) return -1;
    g_ota.state = OTA_STATE_COMMIT;
    ota_step_commit();
    return 0;
}

int ota_status(ota_phase_t *phase, uint32_t *pct) {
    if (!phase || !pct) return -1;

    switch (g_ota.state) {
    case OTA_STATE_IDLE:       *phase = OTA_PHASE_IDLE;        *pct = 0;   break;
    case OTA_STATE_DOWNLOADING: *phase = OTA_PHASE_DOWNLOADING; *pct = g_ota.req.expected_size > 0 ? (g_ota.received_bytes * 100 / g_ota.req.expected_size) : 0; break;
    case OTA_STATE_VERIFYING:   *phase = OTA_PHASE_VERIFYING;  *pct = 100; break;
    case OTA_STATE_READY:       *phase = OTA_PHASE_READY;      *pct = 100; break;
    case OTA_STATE_ERROR:       *phase = OTA_PHASE_ERROR;      *pct = 0;   break;
    default: return -1;
    }
    return 0;
}

const char *ota_version(char *buf, uint32_t len) {
    /* Read from compiled-in version — replace with actual version */
    snprintf(buf, len, "%d.%d.%d",
             1, 5, 0);  /* TODO: read from image header */
    return buf;
}

int ota_rollback(void) {
    /* Write a special rollback flag */
    uint32_t rollback_flag = 0x524F4C42UL;  /* "ROLB" */
    FLASH_EraseSector(13);
    FLASH_ProgramWord(OTA_STATUS_BASE_ADDR + 4, rollback_flag);
    return 0;
}

void ota_confirm_boot(void) {
    /* Write confirmed flag — BL checks this to prevent rollback */
    uint32_t confirmed = 0x434F4E46UL;  /* "CONF" */
    FLASH_ProgramWord(OTA_STATUS_BASE_ADDR + 0x38, confirmed);
}
