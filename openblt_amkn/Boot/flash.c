#include "boot.h"
#include "boot.h"

/* === ZQLY STM32H7 Register-level Flash Ops (with fixes) === */
typedef struct { volatile unsigned int ACR, KEYR, OPTKEYR, CR, SR, CCR, OPTCR, OPTCR1; } ZFLASH_Type;
#define ZFLASH   ((ZFLASH_Type *)0x52002000u)

#define FLASH_INVALID_SECTOR_IDX        (0xff)
#define FLASH_INVALID_ADDRESS           (0xffffffffu)
#define FLASH_WRITE_BLOCK_SIZE          (1024)
#define FLASH_TOTAL_SECTORS             (sizeof(flashLayout)/sizeof(flashLayout[0]))
#define FLASH_END_ADDRESS               (flashLayout[FLASH_TOTAL_SECTORS-1].sector_start + \
                                         flashLayout[FLASH_TOTAL_SECTORS-1].sector_size - 1)
#ifndef BOOT_FLASH_VECTOR_TABLE_CS_OFFSET
#define BOOT_FLASH_VECTOR_TABLE_CS_OFFSET    (0x298)
#endif

typedef struct {
  unsigned int sector_start;
  unsigned int sector_size;
  unsigned char sector_num;
  unsigned char bank_num;
} tFlashSector;

typedef struct {
  unsigned int base_addr;
  unsigned char data[FLASH_WRITE_BLOCK_SIZE];
} tFlashBlockInfo;

/* Flash layout: sector0=BL reserved, sectors1-7=Bank1, sectors8-15=Bank2 */
static const tFlashSector flashLayout[] = {
    { 0x08020000, 0x20000, 1, 1},
    { 0x08040000, 0x20000, 2, 1},
    { 0x08060000, 0x20000, 3, 1},
    { 0x08080000, 0x20000, 4, 1},
    { 0x080A0000, 0x20000, 5, 1},
    { 0x080C0000, 0x20000, 6, 1},
    { 0x080E0000, 0x20000, 7, 1},
    { 0x08100000, 0x20000, 0, 2},
    { 0x08120000, 0x20000, 1, 2},
    { 0x08140000, 0x20000, 2, 2},
    { 0x08160000, 0x20000, 3, 2},
    { 0x08180000, 0x20000, 4, 2},
    { 0x081A0000, 0x20000, 5, 2},
    { 0x081C0000, 0x20000, 6, 2},
    { 0x081E0000, 0x20000, 7, 2},
};

static tFlashBlockInfo blockInfo;
static tFlashBlockInfo bootBlockInfo;

/* ---- Low-level FLASH ops ---- */

/* Get bank-specific flash register base */
static unsigned int zflash_get_base(unsigned int addr) {
    if (addr >= 0x08100000u) return 0x52002100u;  /* Bank 2 */
    return 0x52002000u;                             /* Bank 1 */
}

#define ZF(base) ((ZFLASH_Type *)(base))

static void zflash_unlock(unsigned int base) {
    if (ZF(base)->CR & (1u << 31)) {
        ZF(base)->KEYR = 0x45670123u;
        ZF(base)->KEYR = 0xCDEF89ABu;
    }
}

static void zflash_clear_errors(unsigned int base) {
    ZF(base)->CCR = 0xFFFFFFFFu;
}

static int zflash_wait(unsigned int base, unsigned int timeout_us) {
    volatile unsigned int t = timeout_us * 10;  /* rough ~100ns per loop at 64MHz */
    while ((ZF(base)->SR & 1u) && --t) {}
    return (t > 0) ? 1 : 0;  /* 1=ok, 0=timeout */
}

static void zflash_erase_sector(int sec, int bank) {
    unsigned int base = (bank == 2) ? 0x52002100u : 0x52002000u;
    zflash_unlock(base);
    zflash_clear_errors(base);
    ZF(base)->CR &= ~(0xFFu << 8);
    ZF(base)->CR |= ((unsigned int)sec << 8) | (2u << 0) | (1u << 16);
    if (!zflash_wait(base, 1000000)) { /* timeout */ }
    ZF(base)->CR &= ~((2u << 0) | (1u << 16));
    zflash_clear_errors(base);
}

static int zflash_write32_checked(unsigned int addr, const unsigned char *d) {
    int i;
    unsigned int base = zflash_get_base(addr);
    volatile unsigned int *dst = (volatile unsigned int *)addr;
    
    zflash_unlock(base);
    zflash_clear_errors(base);
    
    /* Set PG bit */
    ZF(base)->CR |= (1u << 0);  /* PG = bit 0 */
    
    /* Write 8 words (256 bits = one flash word) */
    for (i = 0; i < 8; i++) {
        dst[i] = ((unsigned int)d[0]) | ((unsigned int)d[1] << 8) |
                 ((unsigned int)d[2] << 16) | ((unsigned int)d[3] << 24);
        d += 4;
    }
    
    /* Memory barrier before starting operation */
    __asm("dsb 0xF");
    
    /* Start programming operation */
    ZF(base)->CR |= (1u << 16);  /* START */
    
    /* Wait for completion */
    if (!zflash_wait(base, 100000)) {
        /* Timeout: clear and return error */
        ZF(base)->CR &= ~((1u << 16) | (1u << 0));
        return 0;
    }
    
    /* Check for programming error */
    if (ZF(base)->SR & (1u << 7)) {  /* PROGERR */
        ZF(base)->CR &= ~((1u << 16) | (1u << 0));
        zflash_clear_errors(base);
        return 0;
    }
    
    /* Check for EOP (end of programming) success */
    if (!(ZF(base)->SR & (1u << 16))) {  /* EOP */
        ZF(base)->CR &= ~((1u << 16) | (1u << 0));
        return 0;
    }
    
    /* Clear EOP flag */
    ZF(base)->CCR = (1u << 16);
    
    /* Clear PG + START */
    ZF(base)->CR &= ~((1u << 16) | (1u << 0));
    zflash_clear_errors(base);
    
    /* Verify written data */
    for (i = 0; i < 8; i++) {
        unsigned int expected = ((unsigned int)d[-32 + i*4]) | 
                               ((unsigned int)d[-31 + i*4] << 8) |
                               ((unsigned int)d[-30 + i*4] << 16) | 
                               ((unsigned int)d[-29 + i*4] << 24);
        if (dst[i] != expected) {
            return 0;  /* Verify failed */
        }
    }
    
    return 1;  /* Success */
}

/* ---- Forward declarations ---- */
static blt_bool FlashInitBlock(tFlashBlockInfo *block, blt_addr address);
static blt_bool FlashWriteBlock(tFlashBlockInfo *block);
static blt_bool FlashAddToBlock(tFlashBlockInfo *block, blt_addr address, blt_int8u *data, blt_int32u len);
static blt_int8u FlashGetSectorIdx(blt_addr address);
static blt_bool FlashEraseSectors(blt_int8u first, blt_int8u last);
static blt_bool FlashEmptyCheckSector(blt_int8u sectorIdx);
static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr);

/* ---- Public API ---- */
void FlashInit(void) {
    FlashInitBlock(&blockInfo, FLASH_INVALID_ADDRESS);
    bootBlockInfo.base_addr = FLASH_INVALID_ADDRESS;
}

blt_bool FlashWrite(blt_addr addr, blt_int32u len, blt_int8u *data) {
    blt_addr base_addr;
    if ((len - 1) > (FLASH_END_ADDRESS - addr)) return BLT_FALSE;
    if ((FlashGetSectorIdx(addr) == FLASH_INVALID_SECTOR_IDX) ||
        (FlashGetSectorIdx(addr+len-1) == FLASH_INVALID_SECTOR_IDX))
        return BLT_FALSE;
    base_addr = (addr/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;
    if (base_addr == flashLayout[0].sector_start)
        return FlashAddToBlock(&bootBlockInfo, addr, data, len);
    else
        return FlashAddToBlock(&blockInfo, addr, data, len);
}

blt_bool FlashErase(blt_addr addr, blt_int32u len) {
    blt_int8u first_sector_idx, last_sector_idx;
    if ((len - 1) > (FLASH_END_ADDRESS - addr)) return BLT_FALSE;
    first_sector_idx = FlashGetSectorIdx(addr);
    last_sector_idx  = FlashGetSectorIdx(addr+len-1);
    if ((first_sector_idx == FLASH_INVALID_SECTOR_IDX) ||
        (last_sector_idx == FLASH_INVALID_SECTOR_IDX))
        return BLT_FALSE;
    return FlashEraseSectors(first_sector_idx, last_sector_idx);
}

/* Forward declarations */
static blt_bool FlashWriteBlock(tFlashBlockInfo *block);

/* ---- Sector helpers ---- */
static blt_int8u FlashGetSectorIdx(blt_addr address) {
    blt_int8u result = FLASH_INVALID_SECTOR_IDX;
    blt_int8u i;
    for (i = 0; i < FLASH_TOTAL_SECTORS; i++) {
        CopService();
        if ((address >= flashLayout[i].sector_start) &&
            (address < (flashLayout[i].sector_start + flashLayout[i].sector_size))) {
            result = i; break;
        }
    }
    return result;
}

static blt_bool FlashEmptyCheckSector(blt_int8u sectorIdx) {
    unsigned int sectorAddr = flashLayout[sectorIdx].sector_start;
    unsigned int sectorSize = flashLayout[sectorIdx].sector_size;
    volatile const unsigned int *wordPtr = (volatile const unsigned int *)sectorAddr;
    unsigned int wordCnt;
    for (wordCnt = 0; wordCnt < (sectorSize/4); wordCnt++) {
        if ((wordCnt % 256) == 0) CopService();
        if (*wordPtr != 0xFFFFFFFFu) return BLT_FALSE;
        wordPtr++;
    }
    return BLT_TRUE;
}

static blt_bool FlashEraseSectors(blt_int8u first, blt_int8u last) {
    blt_int8u i;
    if (first > last) return BLT_FALSE;
    if (last >= FLASH_TOTAL_SECTORS) return BLT_FALSE;
    for (i = first; i <= last; i++) {
        if (FlashEmptyCheckSector(i) == BLT_FALSE) {
            CopService();
            zflash_erase_sector(flashLayout[i].sector_num, flashLayout[i].bank_num);
        }
    }
    return BLT_TRUE;
}

/* ---- Block management ---- */
static blt_bool FlashInitBlock(tFlashBlockInfo *block, blt_addr address) {
    if ((address % FLASH_WRITE_BLOCK_SIZE) != 0) return BLT_FALSE;
    if (block->base_addr != address) {
        block->base_addr = address;
        CpuMemCopy((blt_addr)block->data, address, FLASH_WRITE_BLOCK_SIZE);
    }
    return BLT_TRUE;
}

static tFlashBlockInfo *FlashSwitchBlock(tFlashBlockInfo *block, blt_addr base_addr) {
    if (block->base_addr != FLASH_INVALID_ADDRESS) {
        if (FlashWriteBlock(block) == BLT_FALSE) return BLT_NULL;
    }
    if (FlashInitBlock(block, base_addr) == BLT_FALSE) return BLT_NULL;
    return block;
}

static blt_bool FlashAddToBlock(tFlashBlockInfo *block, blt_addr address, blt_int8u *data, blt_int32u len) { blt_addr current_base_addr = (address/FLASH_WRITE_BLOCK_SIZE)*FLASH_WRITE_BLOCK_SIZE;
    blt_int8u *dst, *src;
    if (block->base_addr == FLASH_INVALID_ADDRESS) {
        if (FlashInitBlock(block, current_base_addr) == BLT_FALSE) return BLT_FALSE;
    }
    if (block->base_addr != current_base_addr) {
        block = FlashSwitchBlock(block, current_base_addr);
        if (block == BLT_NULL) return BLT_FALSE;
    }
    dst = &(block->data[address - block->base_addr]);
    src = data;
    do {
        CopService();
        if ((blt_addr)(dst - &(block->data[0])) >= FLASH_WRITE_BLOCK_SIZE) {
            block = FlashSwitchBlock(block, block->base_addr + FLASH_WRITE_BLOCK_SIZE);
            if (block == BLT_NULL) return BLT_FALSE;
            dst = &(block->data[0]);
        }
        *dst++ = *src++;
        len--;
    } while (len > 0);
    return BLT_TRUE;
}

static blt_bool FlashWriteBlock(tFlashBlockInfo *block) {
    blt_addr prog_addr = block->base_addr;
    unsigned int wordCnt;
    if (prog_addr == FLASH_INVALID_ADDRESS) return BLT_FALSE;
    for (wordCnt = 0; wordCnt < (FLASH_WRITE_BLOCK_SIZE/32); wordCnt++) {
        CopService();
        if (!zflash_write32_checked(prog_addr, (const unsigned char*)&block->data[wordCnt * 32])) {
            return BLT_FALSE;
        }
        prog_addr += 32;
    }
    return BLT_TRUE;
}

blt_bool FlashWriteChecksum(void) {
    blt_int32u signature_checksum = 0;
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x00));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x04));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x08));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x0C));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x10));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x14));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + 0x18));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start + BOOT_FLASH_VECTOR_TABLE_CS_OFFSET));
    return FlashWrite(flashLayout[0].sector_start + BOOT_FLASH_VECTOR_TABLE_CS_OFFSET,
                      sizeof(signature_checksum), (blt_int8u*)&signature_checksum);
}

blt_bool FlashVerifyChecksum(void) {
    blt_int32u signature_checksum = 0;
    blt_int32u prog_checksum;
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x04));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x08));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x0C));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x10));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x14));
    signature_checksum += *((blt_int32u *)(flashLayout[0].sector_start+0x18));
    prog_checksum = *((blt_int32u *)(flashLayout[0].sector_start + BOOT_FLASH_VECTOR_TABLE_CS_OFFSET));
    return (blt_bool)(signature_checksum == prog_checksum);
}

blt_bool FlashDone(void) {
    blt_bool result = BLT_TRUE;
    if (bootBlockInfo.base_addr != FLASH_INVALID_ADDRESS) {
        if (FlashWriteBlock(&bootBlockInfo) == BLT_FALSE) result = BLT_FALSE;
    }
    if (result == BLT_TRUE) {
        if (blockInfo.base_addr != FLASH_INVALID_ADDRESS) {
            if (FlashWriteBlock(&blockInfo) == BLT_FALSE) result = BLT_FALSE;
        }
    }
    return result;
}

blt_addr FlashGetUserProgBaseAddress(void) {
    return flashLayout[0].sector_start;
}