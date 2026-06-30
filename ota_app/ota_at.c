/*****************************************************************************
 * ota_at.c — AT+OTA command handler implementation
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ota_at.h"
#include "ota_client.h"
#include "ota_config.h"

/* ========== External UART send ========== */
extern void at_send_response(const char *resp);

/* ========== Progress tracking for AT ========== */
static ota_phase_t g_at_phase = OTA_PHASE_IDLE;
static uint32_t    g_at_pct = 0;
static char        g_at_msg[64];

void ota_at_progress(ota_phase_t phase, uint32_t pct, const char *msg) {
    g_at_phase = phase;
    g_at_pct = pct;
    if (msg) {
        strncpy(g_at_msg, msg, sizeof(g_at_msg) - 1);
        g_at_msg[sizeof(g_at_msg) - 1] = '\0';
    } else {
        g_at_msg[0] = '\0';
    }
}

/* ========== Command Parsing Helpers ========== */
static const char *skip_prefix(const char *cmd, const char *prefix) {
    int len = (int)strlen(prefix);
    if (strncmp(cmd, prefix, len) != 0) return 0;
    return cmd + len;
}

/*
 * Parse comma-separated argument.
 * Returns pointer to start of value, sets *next to next char after value.
 * Value is NOT null-terminated; *out_len gives length.
 */
static const char *parse_arg(const char *p, const char **next, int *out_len) {
    *out_len = 0;
    while (*p == ' ') p++;

    if (*p == '\0' || *p == '\r' || *p == '\n') { *next = p; return p; }

    const char *start = p;
    while (*p && *p != ',' && *p != '\r' && *p != '\n') { p++; (*out_len)++; }

    *next = (*p == ',') ? p + 1 : p;
    return start;
}

/*
 * Extract URL from AT+OTA=START,<url>,<size>,<sha256>
 * URL may contain commas in query strings, but we assume simple URLs.
 */
static int parse_start_args(const char *args, char *url, int url_len,
                            uint32_t *size, char *sha256_hex, int sha256_len) {
    const char *p = args, *next;
    int len;

    /* URL */
    p = parse_arg(args, &next, &len);
    if (len == 0 || len >= url_len) return -1;
    memcpy(url, p, len); url[len] = '\0';
    p = next;

    /* Size */
    p = parse_arg(p, &next, &len);
    if (len == 0) { *size = 0; }  /* Optional */
    else { char tmp[16]; memcpy(tmp, p, len); tmp[len] = '\0'; *size = (uint32_t)atol(tmp); }
    p = next;

    /* SHA256 hex (optional) */
    p = parse_arg(p, &next, &len);
    if (len > 0 && len < sha256_len) {
        memcpy(sha256_hex, p, len); sha256_hex[len] = '\0';
    } else {
        sha256_hex[0] = '\0';
    }

    return 0;
}

/* ========== Command Dispatch ========== */
int ota_at_dispatch(const char *cmd, char *resp, int len) {
    const char *p;

    /* AT+OTA=START,<url>,<size>,<sha256> */
    p = skip_prefix(cmd, "AT+OTA=START,");
    if (p) {
        char url[256], sha256_hex[128];
        uint32_t size;

        if (g_at_phase == OTA_PHASE_DOWNLOADING) {
            snprintf(resp, len, "+OTA:START,ERR,BUSY");
            at_send_response(resp);
            return 0;
        }

        if (parse_start_args(p, url, sizeof(url), &size, sha256_hex, sizeof(sha256_hex)) != 0) {
            snprintf(resp, len, "+OTA:START,ERR,PARSE");
            at_send_response(resp);
            return 0;
        }

        ota_request_t req;
        memset(&req, 0, sizeof(req));
        req.url = url;
        req.expected_size = size;
        req.progress_cb = ota_at_progress;

        int rc = ota_start(&req);
        if (rc == 0) {
            snprintf(resp, len, "+OTA:START,OK");
        } else {
            snprintf(resp, len, "+OTA:START,ERR,%d", rc);
        }
        at_send_response(resp);
        return 0;
    }

    /* AT+OTA=STATUS? */
    p = skip_prefix(cmd, "AT+OTA=STATUS?");
    if (p) {
        ota_phase_t phase;
        uint32_t pct;
        ota_status(&phase, &pct);

        const char *phase_str = "IDLE";
        switch (phase) {
        case OTA_PHASE_DOWNLOADING: phase_str = "DOWNLOADING"; break;
        case OTA_PHASE_VERIFYING:   phase_str = "VERIFYING";   break;
        case OTA_PHASE_READY:       phase_str = "READY";       break;
        case OTA_PHASE_ERROR:       phase_str = "ERROR";       break;
        default: break;
        }
        snprintf(resp, len, "+OTA:STATUS,%s,%lu", phase_str, (unsigned long)pct);
        at_send_response(resp);
        return 0;
    }

    /* AT+OTA=ABORT */
    p = skip_prefix(cmd, "AT+OTA=ABORT");
    if (p) {
        int rc = ota_abort();
        snprintf(resp, len, "+OTA:ABORT,%s", (rc == 0) ? "OK" : "ERR");
        at_send_response(resp);
        return 0;
    }

    /* AT+OTA=COMMIT */
    p = skip_prefix(cmd, "AT+OTA=COMMIT");
    if (p) {
        int rc = ota_commit();
        if (rc == 0) {
            snprintf(resp, len, "+OTA:COMMIT,OK");
            at_send_response(resp);
            /* Reboot after response sent */
            /* TODO: NVIC_SystemReset(); */
        } else {
            snprintf(resp, len, "+OTA:COMMIT,ERR");
            at_send_response(resp);
        }
        return 0;
    }

    /* AT+OTA=ROLLBACK */
    p = skip_prefix(cmd, "AT+OTA=ROLLBACK");
    if (p) {
        ota_rollback();
        snprintf(resp, len, "+OTA:ROLLBACK,OK");
        at_send_response(resp);
        /* TODO: NVIC_SystemReset(); */
        return 0;
    }

    /* AT+OTA=VERSION? */
    p = skip_prefix(cmd, "AT+OTA=VERSION?");
    if (p) {
        char ver[32];
        ota_version(ver, sizeof(ver));
        snprintf(resp, len, "+OTA:VERSION,%s", ver);
        at_send_response(resp);
        return 0;
    }

    return -1;  /* Not an OTA command */
}
