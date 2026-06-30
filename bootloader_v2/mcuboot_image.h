#ifndef MCUBOOT_IMAGE_H
#define MCUBOOT_IMAGE_H
#include <stdint.h>

#define IMAGE_MAGIC_V1          0x96f3b83dUL
#define IMAGE_HEADER_SIZE_MIN   0x0200

#define IMAGE_F_PIC             0x00000001
#define IMAGE_F_NON_BOOTABLE    0x00000002
#define IMAGE_F_ENCRYPTED       0x00000004
#define IMAGE_F_RAM_LOAD        0x00000008

#define TLV_TYPE_KEYHASH        0x01
#define TLV_TYPE_PUBKEY         0x02
#define TLV_TYPE_SHA256         0x10
#define TLV_TYPE_ECDSA256       0x22
#define TLV_TYPE_RSA2048        0x20
#define TLV_TYPE_RSA3072        0x23
#define TLV_TYPE_DEPENDENCY     0x40
#define TLV_TYPE_SEC_CNT        0x50

typedef struct __attribute__((packed)) {
    uint8_t  iv_major;
    uint8_t  iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
} image_version_t;

typedef struct __attribute__((packed)) {
    uint32_t        ih_magic;
    uint32_t        ih_load_addr;
    uint16_t        ih_hdr_size;
    uint16_t        ih_protect_tlv_size;
    uint32_t        ih_img_size;
    uint32_t        ih_flags;
    image_version_t ih_ver;
    uint32_t        ih_pad;
} image_header_t;

typedef struct __attribute__((packed)) {
    uint16_t it_type;
    uint16_t it_len;
} image_tlv_t;

typedef struct {
    image_header_t  hdr;
    uint32_t        total_size;
    const uint8_t  *payload;
    uint32_t        payload_size;
    const uint8_t  *sha256_hash;
    const uint8_t  *ecdsa_sig;
    const uint8_t  *key_hash;
} image_info_t;

static inline int image_check_magic(const uint8_t *addr) {
    return ((const image_header_t *)addr)->ih_magic == IMAGE_MAGIC_V1 ? 0 : -1;
}
static inline uint32_t image_total_size(const image_header_t *hdr) {
    return hdr->ih_hdr_size + hdr->ih_img_size;
}

int image_parse(const uint8_t *addr, uint32_t len, image_info_t *info);
int image_verify_ecdsa256(const image_info_t *info, const uint8_t *pubkey, uint32_t key_len);
int image_verify_sha256(const uint8_t *addr, uint32_t len);

#endif