/*****************************************************************************
 * bootloader_v2.c -- AMKN8639 MCUBoot-style Bootloader V2
 * Target:  STM32H743XIH6, BL @ 0x08000000 (128KB), APP @ 0x08020000
 * SPI1:    PA5(SCK) PA6(MISO) PA7(MOSI) PB0(CS) -- W25Q64
 * UART1:   PA9(TX) PA10(RX) 115200 8N1 (debug output)
 *****************************************************************************/
#include <stdint.h>
#include <string.h>
#include "mcuboot_image.h"
#include "flash_map.h"
#include "crypto_port.h"

/* ========== Runtime Support ========== */
void *memcpy(void *dst, const void *src, unsigned int n) {
    unsigned int i; char *d=(char*)dst; const char *s=(const char*)src;
    for(i=0;i<n;i++) d[i]=s[i]; return dst;
}
void *memset(void *s, int c, unsigned int n) {
    unsigned int i; char *p=(char*)s;
    for(i=0;i<n;i++) p[i]=(char)c; return s;
}
int memcmp(const void *a, const void *b, unsigned int n) {
    unsigned int i; const unsigned char *pa=(const unsigned char*)a,*pb=(const unsigned char*)b;
    for(i=0;i<n;i++){if(pa[i]!=pb[i])return pa[i]-pb[i];} return 0;
}

void __aeabi_memcpy(void *d,const void *s,unsigned int n){memcpy(d,s,n);}
void __aeabi_memset(void *d,int c,unsigned int n){memset(d,c,n);}
void __aeabi_memcpy4(void *d,const void *s,unsigned int n){memcpy(d,s,n);}
void __aeabi_memcpy8(void *d,const void *s,unsigned int n){memcpy(d,s,n);}
void __aeabi_memclr(void *d,unsigned int n){memset(d,0,n);}
void __aeabi_memclr4(void *d,unsigned int n){memset(d,0,n);}
void __aeabi_memclr8(void *d,unsigned int n){memset(d,0,n);}

/* ========== SHA-256 Implementation ========== */
static const uint32_t sha256_k[64]={
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cd3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

static uint32_t sha256_rotR(uint32_t v,int n){return(v>>n)|(v<<(32-n));}
#define CH(x,y,z)  (((x)&(y))^(~(x)&(z)))
#define MAJ(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))
#define EP0(x) (sha256_rotR(x,2)^sha256_rotR(x,13)^sha256_rotR(x,22))
#define EP1(x) (sha256_rotR(x,6)^sha256_rotR(x,11)^sha256_rotR(x,25))
#define SIG0(x) (sha256_rotR(x,7)^sha256_rotR(x,18)^(x>>3))
#define SIG1(x) (sha256_rotR(x,17)^sha256_rotR(x,19)^(x>>10))

static void sha256_transform(sha256_ctx_t *ctx,const uint8_t *data){
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for(i=0,j=0;i<16;i++,j+=4)
        m[i]=(uint32_t)data[j]<<24|(uint32_t)data[j+1]<<16|(uint32_t)data[j+2]<<8|(uint32_t)data[j+3];
    for(;i<64;i++)
        m[i]=SIG1(m[i-2])+m[i-7]+SIG0(m[i-15])+m[i-16];
    a=ctx->state[0];b=ctx->state[1];c=ctx->state[2];d=ctx->state[3];
    e=ctx->state[4];f=ctx->state[5];g=ctx->state[6];h=ctx->state[7];
    for(i=0;i<64;i++){
        t1=h+EP1(e)+CH(e,f,g)+sha256_k[i]+m[i];
        t2=EP0(a)+MAJ(a,b,c);h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
    }
    ctx->state[0]+=a;ctx->state[1]+=b;ctx->state[2]+=c;ctx->state[3]+=d;
    ctx->state[4]+=e;ctx->state[5]+=f;ctx->state[6]+=g;ctx->state[7]+=h;
}
void sha256_init(sha256_ctx_t *ctx){
    ctx->count=0;
    ctx->state[0]=0x6a09e667;ctx->state[1]=0xbb67ae85;ctx->state[2]=0x3c6ef372;ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f;ctx->state[5]=0x9b05688c;ctx->state[6]=0x1f83d9ab;ctx->state[7]=0x5be0cd19;
}
void sha256_update(sha256_ctx_t *ctx,const uint8_t *data,uint32_t len){
    uint32_t i=ctx->count&63;
    ctx->count+=len;
    while(len--){
        ctx->buf[i++]=*data++;
        if(i==64){sha256_transform(ctx,ctx->buf);i=0;}
    }
}
void sha256_final(sha256_ctx_t *ctx,uint8_t digest[32]){
    uint64_t bc=ctx->count*8;
    uint32_t i=ctx->count&63;
    ctx->buf[i++]=0x80;
    if(i>56){while(i<64)ctx->buf[i++]=0;sha256_transform(ctx,ctx->buf);i=0;}
    while(i<56)ctx->buf[i++]=0;
    ctx->buf[56]=(uint8_t)(bc>>56);ctx->buf[57]=(uint8_t)(bc>>48);
    ctx->buf[58]=(uint8_t)(bc>>40);ctx->buf[59]=(uint8_t)(bc>>32);
    ctx->buf[60]=(uint8_t)(bc>>24);ctx->buf[61]=(uint8_t)(bc>>16);
    ctx->buf[62]=(uint8_t)(bc>>8);ctx->buf[63]=(uint8_t)bc;
    sha256_transform(ctx,ctx->buf);
    for(i=0;i<4;i++){digest[i]=(uint8_t)(ctx->state[0]>>(24-i*8));digest[i+4]=(uint8_t)(ctx->state[1]>>(24-i*8));digest[i+8]=(uint8_t)(ctx->state[2]>>(24-i*8));digest[i+12]=(uint8_t)(ctx->state[3]>>(24-i*8));digest[i+16]=(uint8_t)(ctx->state[4]>>(24-i*8));digest[i+20]=(uint8_t)(ctx->state[5]>>(24-i*8));digest[i+24]=(uint8_t)(ctx->state[6]>>(24-i*8));digest[i+28]=(uint8_t)(ctx->state[7]>>(24-i*8));}
}
void sha256_hash(const uint8_t *data,uint32_t len,uint8_t digest[32]){
    sha256_ctx_t ctx;sha256_init(&ctx);sha256_update(&ctx,data,len);sha256_final(&ctx,digest);
}

/* ========== CRC32 ========== */
static const uint32_t crc32_table[256]={
    0x00000000,0x77073096,0xee0e612c,0x990951ba,0x076dc419,0x706af48f,
    0xe963a535,0x9e6495a3,0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,
    0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,0x1db71064,0x6ab020f2,
    0xf3b97148,0x84be41de,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
    0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,
    0xfa0f3d63,0x8d080df5,0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,
    0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,0x35b5a8fa,0x42b2986c,
    0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
    0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,
    0xcfba9599,0xb8bda50f,0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,
    0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,0x76dc4190,0x01db7106,
    0x98d220bc,0xefd5102a,0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
    0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x086d3d2d,
    0x91646c97,0xe6635c01,0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,
    0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,0x65b0d9c6,0x12b7e950,
    0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
    0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,
    0xa4d1c46d,0xd3d6f4fb,0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,
    0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,0x5005713c,0x270241aa,
    0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
    0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,
    0xb7bd5c3b,0xc0ba6cad,0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,
    0xead54739,0x9dd277af,0x04db2615,0x73dc1683,0xe3630b12,0x94643b84,
    0x0d6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x93047f9d,0x0a00ae27,0x7d079eb1,
    0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,
    0x196c3671,0x6e6b06e7,0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,
    0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,0xd6d6a3e8,0xa1d1937e,
    0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
    0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,
    0x316e8eef,0x4669be79,0xcb61b38c,0xbc66831a,0x256fd2a0,0x52568e36,
    0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,0xc5ba3bbe,0xb2bd0b28,
    0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
    0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,0x9c0906a9,0xeb0e363f,
    0x72076785,0x05005713,0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,
    0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,0x86d3d2d4,0xf1d4e242,
    0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
    0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,
    0x616bffd3,0x166ccf45,0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,
    0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,0xaed16a4a,0xd9d65adc,
    0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
    0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,
    0x54de5729,0x23d967bf,0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,
    0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d};
uint32_t crc32_calc(const uint8_t *data,uint32_t len){
    uint32_t i,crc=0xFFFFFFFF;
    for(i=0;i<len;i++) crc=crc32_table[(crc^data[i])&0xFF]^(crc>>8);
    return crc^0xFFFFFFFF;
}

/* ========== Image Parse ========== */
int image_parse(const uint8_t *addr, uint32_t len, image_info_t *info) {
    const image_header_t *hdr = (const image_header_t *)addr;
    uint32_t off;
    if (len < sizeof(image_header_t)) return -1;
    if (hdr->ih_magic != IMAGE_MAGIC_V1) return -2;
    if (hdr->ih_hdr_size < IMAGE_HEADER_SIZE_MIN || hdr->ih_hdr_size > len) return -3;
    
    memcpy(&info->hdr, hdr, sizeof(image_header_t));
    info->total_size = hdr->ih_hdr_size + hdr->ih_img_size;
    info->payload = addr + hdr->ih_hdr_size;
    info->payload_size = hdr->ih_img_size;
    info->sha256_hash = 0;
    info->ecdsa_sig = 0;
    info->key_hash = 0;
    
    /* Parse TLVs */
    off = sizeof(image_header_t);
    while (off + 4 <= hdr->ih_hdr_size) {
        const image_tlv_t *tlv = (const image_tlv_t *)(addr + off);
        uint32_t tlv_end = off + 4 + tlv->it_len;
        if (tlv_end > hdr->ih_hdr_size) break;
        if (tlv->it_type == 0xFF || tlv->it_len == 0) break;
        switch (tlv->it_type) {
        case TLV_TYPE_SHA256:    info->sha256_hash = addr + off + 4; break;
        case TLV_TYPE_ECDSA256:  info->ecdsa_sig = addr + off + 4; break;
        case TLV_TYPE_KEYHASH:   info->key_hash = addr + off + 4; break;
        }
        off = tlv_end;
    }
    return 0;
}

/* ========== MCU Registers ========== */
#define RCC_BASE       0x58024400
#define RCC_CR         (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_AHB4ENR    (*(volatile uint32_t *)(RCC_BASE + 0xE0))
#define RCC_APB2ENR    (*(volatile uint32_t *)(RCC_BASE + 0xF8))
#define RCC_APB1LENR   (*(volatile uint32_t *)(RCC_BASE + 0xE8))

typedef struct { volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH; } GPIO_Reg;
#define GPIOA  ((GPIO_Reg *)0x58020000)
#define GPIOB  ((GPIO_Reg *)0x58020400)
#define GPIOI  ((GPIO_Reg *)0x58022000)

typedef struct { volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR; } SPI_Reg;
#define SPI1   ((SPI_Reg *)0x40013000)

typedef struct { volatile uint32_t CR1, CR2, CR3, BRR, GTPR, RTOR, RQR, ISR, ICR, RDR, TDR; } USART_Reg;
#define USART1 ((USART_Reg *)0x40011000)

typedef struct { volatile uint32_t CSR, RVR, CVR, CALIB; } SYST_Reg;
#define SYST   ((SYST_Reg *)0xE000E010)

typedef struct { uint32_t _r0[8]; volatile uint32_t ICSR, VTOR; } SCB_Reg;
#define SCB    ((SCB_Reg *)0xE000ED00)

#define W25Q_CMD_READ         0x03
#define W25Q_CMD_WREN         0x06
#define W25Q_CMD_SECTOR_ERASE 0x20
#define W25Q_CMD_PAGE_PROG    0x02
#define W25Q_CMD_RDSR1        0x05
#define W25Q_CMD_RDID         0x9F

/* ========== Globals ========== */
volatile uint32_t g_msTick;
ota_state_t g_otaState;

/* ========== UART Debug ========== */
static void uart_putc(char c) {
    while (!(USART1->ISR & (1<<7)));
    USART1->TDR = c;
}
static void uart_puts(const char *s) { while (*s) uart_putc(*s++); }
static void uart_puthex(uint32_t v, int n) {
    static const char h[]="0123456789ABCDEF";
    int i; for(i=n-1;i>=0;i--) uart_putc(h[(v>>(i*4))&0xF]);
}
static void uart_puthex8(const uint8_t *d, int n) {
    int i; for(i=0;i<n;i++){uart_puthex(d[i],2);uart_putc(0x20);}
}

/* ========== Systick ========== */
void SysTick_Handler(void) { g_msTick++; }
static void systick_init(void) { SYST->RVR=63999; SYST->CVR=0; SYST->CSR=7; }
static uint32_t millis(void) { return g_msTick; }
static void delay_ms(uint32_t ms) {
    uint32_t s=millis(); while((millis()-s)<ms){__asm("nop");}
}

/* ========== UART Init ========== */
static void uart_init(void) {
    RCC_AHB4ENR|=(1<<0); RCC_APB2ENR|=(1<<4);
    GPIOA->MODER&=~((3<<18)|(3<<20)); GPIOA->MODER|=((2<<18)|(2<<20));
    GPIOA->AFRH&=~((0xF<<4)|(0xF<<8)); GPIOA->AFRH|=((7<<4)|(7<<8));
    USART1->BRR=64000000/115200;
    USART1->CR1=(1<<3)|(1<<2)|(1<<0);
    int i; for(i=0;i<10000;i++)__asm("nop");
}

/* ========== LED ========== */
static void led_init(void){RCC_AHB4ENR|=(1<<8);GPIOI->MODER&=~(3<<16);GPIOI->MODER|=(1<<16);}
static void led_on(void){GPIOI->BSRR=(1<<8);}
static void led_off(void){GPIOI->BSRR=(1<<24);}
static void led_blink(int n,int ms){int i;for(i=0;i<n;i++){led_on();delay_ms(ms);led_off();delay_ms(ms);}}

/* ========== SPI + W25QXX ========== */
static void spi_init(void) {
    RCC_AHB4ENR|=(1<<0)|(1<<1); RCC_APB2ENR|=(1<<12);
    GPIOA->MODER&=~((3<<10)|(3<<12)|(3<<14));
    GPIOA->MODER|=((2<<10)|(2<<12)|(2<<14));
    GPIOA->AFRL&=~((0xF<<20)|(0xF<<24));
    GPIOA->AFRL|=((5<<20)|(5<<24));
    GPIOA->AFRH&=~(0xF<<0); GPIOA->AFRH|=(5<<0);
    GPIOB->MODER&=~(3<<0); GPIOB->MODER|=(1<<0);
    GPIOB->BSRR=(1<<0);
    SPI1->CR1=(1<<2)|(1<<1)|(3<<3)|(1<<6);
}

static uint8_t spi_xfer(uint8_t tx) {
    while(!(SPI1->SR&(1<<1)));
    *(volatile uint8_t*)&SPI1->DR=tx;
    while(!(SPI1->SR&(1<<0)));
    return (uint8_t)SPI1->DR;
}

static void w25q_cs_low(void){GPIOB->BSRR=(1<<16);}
static void w25q_cs_high(void){GPIOB->BSRR=(1<<0);}

static void w25q_write_cmd(uint8_t cmd){w25q_cs_low();spi_xfer(cmd);}
static void w25q_write_addr24(uint32_t addr){spi_xfer((uint8_t)(addr>>16));spi_xfer((uint8_t)(addr>>8));spi_xfer((uint8_t)addr);}

static int w25q_wait_busy(void) {
    uint32_t t0=millis();
    w25q_write_cmd(W25Q_CMD_RDSR1);
    while(spi_xfer(0xFF)&1){if((millis()-t0)>5000){w25q_cs_high();return -1;}}
    w25q_cs_high(); return 0;
}

static int w25q_init(void) {
    w25q_write_cmd(W25Q_CMD_RDID);
    uint8_t mfg=spi_xfer(0xFF), dev=spi_xfer(0xFF);
    w25q_cs_high();
    if(mfg==0x00||mfg==0xFF)return -1;
    uart_puts("SPIFLASH: ");uart_puthex(mfg,2);uart_puthex(dev,2);uart_puts("\n");
    return 0;
}

static int w25q_read(uint32_t addr,uint8_t*buf,uint32_t len){
    w25q_write_cmd(W25Q_CMD_READ); w25q_write_addr24(addr);
    uint32_t i;for(i=0;i<len;i++)buf[i]=spi_xfer(0xFF);
    w25q_cs_high(); return 0;
}

static int w25q_write_page(uint32_t addr,const uint8_t*data,uint32_t len){
    w25q_write_cmd(W25Q_CMD_WREN);w25q_cs_high();
    w25q_write_cmd(W25Q_CMD_PAGE_PROG);w25q_write_addr24(addr);
    uint32_t i;for(i=0;i<len;i++)spi_xfer(data[i]);
    w25q_cs_high(); return w25q_wait_busy();
}

static int w25q_erase_sector(uint32_t addr){
    w25q_write_cmd(W25Q_CMD_WREN);w25q_cs_high();
    w25q_write_cmd(W25Q_CMD_SECTOR_ERASE);w25q_write_addr24(addr);
    w25q_cs_high(); return w25q_wait_busy();
}

/* ========== Internal Flash ========== */
static int flash_unlock_regs(void) {
    
    if(FLASH_REGS->CR&1)return 0;
    FLASH_REGS->KEYR=0x45670123;FLASH_REGS->KEYR=0xCDEF89AB;
    return (FLASH_REGS->CR&1)?0:-1;
}
static void flash_wait(void){while(FLASH_REGS->SR&1);}

static int flash_erase_sector(uint32_t sec){
    
    FLASH_REGS->CR&=~(0xFF<<8);FLASH_REGS->CR|=(sec<<8)|(2<<0)|(1<<16);
    flash_wait();FLASH_REGS->CR&=~((2<<0)|(1<<16));
    return 0;
}

static int flash_write256(uint32_t addr,const uint8_t*data){
    int i; 
    volatile uint32_t*dst=(volatile uint32_t*)addr;
    FLASH_REGS->CR|=(1<<1); flash_wait();
    for(i=0;i<8;i++){dst[i]=((uint32_t)data[0])|((uint32_t)data[1]<<8)|((uint32_t)data[2]<<16)|((uint32_t)data[3]<<24);data+=4;}
    __asm("dsb 0xF");FLASH_REGS->CR|=(1<<16);flash_wait();FLASH_REGS->CR&=~((1<<16)|(1<<1));
    return 0;
}

/* ========== OTA State ========== */
static void ota_state_clear_internal(void) {
    ota_state_t clr;
    memset(&clr,0xFF,sizeof(clr));
    clr.magic=OTA_MAGIC_NONE;
    clr.boot_status=OTA_STATUS_IDLE;
    clr.crc32=crc32_calc((const uint8_t*)&clr,sizeof(clr)-4);
    flash_erase_sector(OTA_STATUS_SECTOR);
    flash_write256(OTA_STATUS_BASE,(const uint8_t*)&clr);
    memcpy(&g_otaState,&clr,sizeof(clr));
}

static int ota_state_init(void) {
    const ota_state_t*p=(const ota_state_t*)OTA_STATUS_BASE;
    memcpy(&g_otaState,p,sizeof(ota_state_t));
    uint32_t calc=crc32_calc((const uint8_t*)&g_otaState,sizeof(ota_state_t)-4);
    if(calc!=g_otaState.crc32 && g_otaState.magic!=OTA_MAGIC_NONE){
        uart_puts("OTA STATE CRC FAIL\n");
        g_otaState.magic=OTA_MAGIC_NONE;
        g_otaState.boot_status=OTA_STATUS_IDLE;
        ota_state_clear_internal();
    }
    uart_puts("OTA STATE: magic=");uart_puthex(g_otaState.magic,8);
    uart_puts(" status=");uart_puthex(g_otaState.boot_status,8);
    uart_puts(" sec=");uart_puthex(g_otaState.sec_counter,8);
    uart_puts("\n");
    return (g_otaState.magic==OTA_MAGIC_FLAG)?1:0;
}

static int ota_state_save_status(ota_status_t status){
    g_otaState.boot_status=status;
    g_otaState.crc32=crc32_calc((const uint8_t*)&g_otaState,sizeof(ota_state_t)-4);
    flash_erase_sector(OTA_STATUS_SECTOR);
    flash_write256(OTA_STATUS_BASE,(const uint8_t*)&g_otaState);
    uart_puts("STATUS SAVED: ");uart_puthex(status,8);uart_puts("\n");
    return 0;
}

/* ========== Image Verify ========== */
static int verify_slot_b(image_info_t*info){
    uint8_t header_buf[IMAGE_HEADER_SIZE_MIN];
    int rc;
    w25q_read(SLOT_B_BASE,header_buf,sizeof(header_buf));
    if(image_check_magic(header_buf)!=0){uart_puts("BAD MAGIC\n");return -1;}
    rc=image_parse(header_buf,sizeof(header_buf),info);
    if(rc!=0){uart_puts("BAD HEADER\n");return -2;}
    uart_puts("IMAGE: ver=");uart_puthex(info->hdr.ih_ver.iv_major,2);
    uart_putc(0x2E);uart_puthex(info->hdr.ih_ver.iv_minor,2);
    uart_putc(0x2E);uart_puthex(info->hdr.ih_ver.iv_revision,4);
    uart_puts(" size=");uart_puthex(info->total_size,8);uart_puts("\n");
    
    uint32_t img_ver=((uint32_t)info->hdr.ih_ver.iv_major<<24)|((uint32_t)info->hdr.ih_ver.iv_minor<<16)|info->hdr.ih_ver.iv_revision;
    if(img_ver<g_otaState.sec_counter){
        uart_puts("ROLLBACK BLOCKED: img=");uart_puthex(img_ver,8);
        uart_puts(" sec=");uart_puthex(g_otaState.sec_counter,8);uart_puts("\n");
        return -3;
    }

    /* SHA256 streaming check */
    {
        sha256_ctx_t ctx;
        sha256_init(&ctx);
        sha256_update(&ctx,header_buf,info->hdr.ih_hdr_size);
        uint32_t off=0,remaining=info->hdr.ih_img_size;
        uint8_t chunk[1024];
        while(remaining>0){
            uint32_t cs=(remaining>sizeof(chunk))?sizeof(chunk):remaining;
            w25q_read(SLOT_B_BASE+info->hdr.ih_hdr_size+off,chunk,cs);
            sha256_update(&ctx,chunk,cs);
            off+=cs;remaining-=cs;
        }
        uint8_t digest[32];
        sha256_final(&ctx,digest);
        if(info->sha256_hash&&memcmp(digest,info->sha256_hash,32)!=0){
            uart_puts("SHA256 MISMATCH\n");return -4;
        }
        uart_puts("SHA256 OK\n");
    }
    return 0;
}

/* ========== Swap B->A ========== */
static int swap_b_to_a(const image_info_t*info){
    uint32_t total=info->total_size;
    uint32_t sectors=(total+FLASH_SECTOR_SIZE-1)/FLASH_SECTOR_SIZE;
    uart_puts("SWAP: ");uart_puthex(total,8);uart_puts(" bytes, ");
    uart_puthex(sectors,2);uart_puts(" sectors\n");

    ota_state_save_status(OTA_STATUS_COPYING);
    uint32_t i;
    for(i=0;i<sectors;i++){
        uint32_t sec=SLOT_A_SECTOR_START+i;
        uart_puts("ERASE S");uart_puthex(sec,2);uart_puts("\n");
        flash_erase_sector(sec);
    }

    uint32_t remaining=total, offset=0;
    uint8_t buf[256];
    while(remaining>0){
        uint32_t chunk=(remaining>sizeof(buf))?sizeof(buf):remaining;
        w25q_read(SLOT_B_BASE+offset,buf,chunk);
        flash_write256(SLOT_A_BASE+offset,buf);
        offset+=32;remaining-=32;
        if((offset&0x7FFF)==0){led_on();delay_ms(10);led_off();}
    }

    ota_state_save_status(OTA_STATUS_VERIFYING);
    uart_puts("VERIFY...\n");
    {
        sha256_ctx_t ctx;
        sha256_init(&ctx);
        remaining=total;offset=0;
        while(remaining>0){
            uint32_t chunk=(remaining>sizeof(buf))?sizeof(buf):remaining;
            const uint8_t*src=(const uint8_t*)(SLOT_A_BASE+offset);
            sha256_update(&ctx,src,chunk);
            offset+=chunk;remaining-=chunk;
        }
        uint8_t digest[32];
        sha256_final(&ctx,digest);
        uart_puts("SLOT A SHA256: ");uart_puthex8(digest,32);uart_puts("\n");
    }

    ota_state_save_status(OTA_STATUS_DONE);
    uint32_t new_sec=((uint32_t)info->hdr.ih_ver.iv_major<<24)|((uint32_t)info->hdr.ih_ver.iv_minor<<16)|info->hdr.ih_ver.iv_revision;
    g_otaState.sec_counter=new_sec;g_otaState.boot_attempts=0;
    ota_state_clear_internal();
    uart_puts("SWAP DONE\n");
    return 0;
}

/* ========== Boot Slot A ========== */
__attribute__((noreturn))
static void boot_slot_a(void) {
    uint32_t*vt=(uint32_t*)SLOT_A_BASE;
    uint32_t sp=vt[0],pc=vt[1];
    uart_puts("BOOT SLOT A: SP=");uart_puthex(sp,8);
    uart_puts(" PC=");uart_puthex(pc,8);uart_puts("\n");
    if(sp<0x20000000||sp>0x24080000||pc<0x08020000||pc>0x08200000){
        uart_puts("BAD VECTOR!\n");
        /* try golden recovery */
        while(1){led_blink(3,200);delay_ms(1000);}
    }
    USART1->CR1=0;SYST->CSR=0;
    SCB->ICSR=(1<<2);SCB->VTOR=SLOT_A_BASE;
    { uint32_t _sp = sp; __asm { MSR MSP, _sp } }
    ((void(*)(void))pc)();
}

/* ========== Golden Recovery ========== */
static int try_golden_recovery(void){
    image_info_t info;
    uint8_t header[IMAGE_HEADER_SIZE_MIN];
    w25q_read(GOLDEN_IMAGE_BASE,header,sizeof(header));
    if(image_check_magic(header)!=0){uart_puts("NO GOLDEN IMAGE\n");return -1;}
    if(image_parse(header,sizeof(header),&info)!=0){uart_puts("BAD GOLDEN HEADER\n");return -2;}
    uart_puts("RECOVER GOLDEN...\n");
    uint32_t total=info.total_size,remaining=total,offset=0;
    uint8_t buf[256];
    while(remaining>0){
        uint32_t chunk=(remaining>sizeof(buf))?sizeof(buf):remaining;
        w25q_read(GOLDEN_IMAGE_BASE+offset,buf,chunk);
        if((offset&(FLASH_SECTOR_SIZE-1))==0){
            uint32_t sec=SLOT_A_SECTOR_START+(offset/FLASH_SECTOR_SIZE);
            if(sec<SLOT_A_SECTOR_START+SLOT_A_SECTOR_COUNT)flash_erase_sector(sec);
        }
        flash_write256(SLOT_A_BASE+offset,buf);offset+=32;remaining-=32;
    }
    uart_puts("GOLDEN RECOVERY DONE\n");
    return 0;
}

/* ========== HW Init ========== */
static void hw_init(void){
    RCC_CR|=(1<<0);while(!(RCC_CR&(1<<2)));
    /* Enable FPU */
    *(volatile uint32_t*)(0xE000ED88)|=(0xF<<20);
    *(volatile uint32_t*)(0x58024800)|=(1<<14);
     FLASH_REGS->ACR=0x04;
    SCB->VTOR=BOOTLOADER_BASE;
    systick_init();led_init();uart_init();spi_init();
    uart_puts("\nAMKN8639 BL V2.0\n");
}


/* ========== BSS Init ========== */
static void bss_init(void) {
    extern unsigned int Image$RW_IRAM1$ZI$Base[];
    extern unsigned int Image$RW_IRAM1$ZI$Limit[];
    unsigned int *p = Image$RW_IRAM1$ZI$Base;
    while (p < Image$RW_IRAM1$ZI$Limit) *p++ = 0;
}

/* ========== Entry ========== */
__attribute__((noreturn))
void Reset_Handler(void){
    /* Immediate LED on to confirm CPU execution */
    { volatile uint32_t *RCC_AHB4 = (volatile uint32_t*)0x580244E0;
      *RCC_AHB4 |= (1<<8);  /* GPIOI clock */
      ((volatile uint32_t*)0x58022000)[0] &= ~(3<<16);
      ((volatile uint32_t*)0x58022000)[0] |= (1<<16);  /* PI8 output */
      ((volatile uint32_t*)0x58022000)[5] = (1<<8); }  /* LED ON */
    g_msTick = 0; memset(&g_otaState, 0, sizeof(g_otaState)); /* BSS init */
    image_info_t imgInfo;
    int has_upgrade;
    hw_init();
    led_blink(2,150);

    if(w25q_init()!=0){
        uart_puts("NO EXT FLASH\n");led_blink(5,100);boot_slot_a();
    }
    if(flash_unlock_regs()!=0){
        uart_puts("FLASH LOCK FAIL\n");led_blink(10,50);boot_slot_a();
    }

    has_upgrade=ota_state_init();
    if(!has_upgrade){
        if(image_check_magic((const uint8_t*)SLOT_A_BASE)!=0){
            uart_puts("SLOT A INVALID\n");
            if(try_golden_recovery()!=0){
                uart_puts("FATAL: NO FIRMWARE\n");
                while(1){led_blink(1,500);}
            }
        }
        uart_puts("BOOT APP...\n");led_off();boot_slot_a();
    }

    uart_puts("OTA PENDING\n");
    if(verify_slot_b(&imgInfo)!=0){
        uart_puts("SLOT B INVALID\n");ota_state_clear_internal();boot_slot_a();
    }

    switch(g_otaState.boot_status){
    case OTA_STATUS_IDLE: break;
    case OTA_STATUS_COPYING: uart_puts("RESUME: COPYING\n");break;
    case OTA_STATUS_VERIFYING:
        uart_puts("RESUME: VERIFYING\n");ota_state_save_status(OTA_STATUS_DONE);
        ota_state_clear_internal();uart_puts("RESUME DONE\n");boot_slot_a();
    case OTA_STATUS_DONE:
        uart_puts("RESUME: DONE\n");ota_state_clear_internal();boot_slot_a();
    }

    if(swap_b_to_a(&imgInfo)!=0){
        uart_puts("SWAP FAIL\n");ota_state_clear_internal();boot_slot_a();
    }
    boot_slot_a();
}

/* ========== Fault Handlers ========== */
void NMI_Handler(void){while(1);}
void HardFault_Handler(void){while(1);}
void MemManage_Handler(void){while(1);}
void BusFault_Handler(void){while(1);}
void UsageFault_Handler(void){while(1);}

/* ========== Vector Table ========== */
__attribute__((section(".vectors")))
const uint32_t vectors[]={
    (uint32_t)0x2001FFF0,
    (uint32_t)Reset_Handler,
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,0,0,0,0,0,0,0,
    (uint32_t)SysTick_Handler,
};