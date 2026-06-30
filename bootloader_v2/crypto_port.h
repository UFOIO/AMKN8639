#ifndef CRYPTO_PORT_H
#define CRYPTO_PORT_H
#include <stdint.h>

#define SHA256_BLOCK_SIZE  64
#define SHA256_DIGEST_SIZE 32

typedef struct {
    uint8_t  buf[64];
    uint32_t state[8];
    uint64_t count;
} sha256_ctx_t;

void sha256_init(sha256_ctx_t *ctx);
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, uint32_t len);
void sha256_final(sha256_ctx_t *ctx, uint8_t digest[32]);
void sha256_hash(const uint8_t *data, uint32_t len, uint8_t digest[32]);

#define ECDSA_P256_KEY_SIZE    64
#define ECDSA_P256_SIG_SIZE    64

int ecdsa_p256_verify(const uint8_t hash[32], const uint8_t pubkey[64], const uint8_t sig[64]);

uint32_t sec_counter_read(void);
int      sec_counter_write(uint32_t counter);

uint32_t crc32_calc(const uint8_t *data, uint32_t len);

#endif