/************************************************************************************
*  File name: stm32h7xx_regs.h
*  Project  : AMKN8639 self-developed Bootloader
*  Description: 自定义 STM32H743 寄存器结构 (零依赖, 不引厂商 HAL/CMSIS)
*                - 包含 BL 需要的最小外设: RCC / GPIOD / USART1 / I2C1 / FLASH
*                - 基于 ST 参考手册 RM0433 (H743)
*                - 全部 volatile, 无 .lib 依赖
*
*  Modify History:
*   1. Version: 1.0
*      Date:    2026.6.22
*      Modify:  Phase 3.2 - 自定义寄存器 (零依赖)
*
*************************************************************************************/
#ifndef __STM32H7XX_REGS_H__
#define __STM32H7XX_REGS_H__

#include <stdint.h>

/*============================================================================
 * 基础类型
 *===========================================================================*/
typedef volatile uint32_t  REG32;
typedef volatile uint16_t  REG16;
typedef volatile uint8_t   REG8;

/*============================================================================
 * RCC base
 *===========================================================================*/
#define RCC_BASE                0x58024400UL
typedef struct {
    REG32 CR;           /* 0x00 */
    REG32 HSICFGR;      /* 0x04 */
    REG32 CRRCR;        /* 0x08 */
    REG32 CSICFGR;      /* 0x0C */
    REG32 CFGR;         /* 0x10 */
    REG32 Reserved1;    /* 0x14 */
    REG32 D1CFGR;       /* 0x18 */
    REG32 D2CFGR;       /* 0x1C */
    REG32 D3CFGR;       /* 0x20 */
    REG32 Reserved2;    /* 0x24 */
    REG32 PLLCKSELR;    /* 0x28 */
    REG32 PLLCFGR;      /* 0x2C */
    REG32 PLL1DIVR;     /* 0x30 */
    REG32 PLL1FRACR;    /* 0x34 */
    REG32 PLL2DIVR;     /* 0x38 */
    REG32 PLL2FRACR;    /* 0x3C */
    REG32 PLL3DIVR;     /* 0x40 */
    REG32 PLL3FRACR;    /* 0x44 */
    REG32 Reserved3;    /* 0x48 */
    REG32 CDCCIPR;      /* 0x4C */
    REG32 CDCCIP1R;     /* 0x50 */
    REG32 CDCCIP2R;     /* 0x54 */
    REG32 SRDCFGR;      /* 0x58 */
    REG32 Reserved4;    /* 0x5C */
    REG32 CIER;         /* 0x60 */
    REG32 CIFR;         /* 0x64 */
    REG32 CICR;         /* 0x68 */
    REG32 Reserved5;    /* 0x6C */
    REG32 BDCR;         /* 0x70 */
    REG32 CSR;          /* 0x74 */
    REG32 Reserved6;    /* 0x78 */
    REG32 AHB1RSTR;     /* 0x7C */
    REG32 AHB2RSTR;     /* 0x80 */
    REG32 AHB3RSTR;     /* 0x84 */
    REG32 AHB4RSTR;     /* 0x88 - GPIO D-H reset */
    REG32 APB1LRSTR;    /* 0x8C - I2C1 reset */
    REG32 APB1HRSTR;    /* 0x90 */
    REG32 APB2RSTR;     /* 0x94 - USART1 reset */
    REG32 APB3RSTR;     /* 0x98 */
    REG32 APB4RSTR;     /* 0x9C */
    REG32 GCR;          /* 0xA0 */
    REG32 Reserved7;    /* 0xA4 */
    REG32 D3AMR;        /* 0xA8 */
    REG32 Reserved8[9]; /* 0xAC-0xCC */
    REG32 RSR;          /* 0xD0 */
    REG32 AHB3ENR;      /* 0xD4 */
    REG32 AHB1ENR;      /* 0xD8 */
    REG32 AHB2ENR;      /* 0xDC */
    REG32 AHB4ENR;      /* 0xE0 - GPIO D-H enable */
    REG32 APB3ENR;      /* 0xE4 - APB3 peripheral enable (BL 不访问) */
    REG32 APB1LENR;     /* 0xE8 - I2C1 enable bit 21, USART3EN bit 18 (probe-verified) */
    REG32 APB1HENR;     /* 0xEC */
    REG32 APB2ENR;      /* 0xF0 - USART1 enable, bit 4 (verified empirically) */
    REG32 APB4ENR;      /* 0xF4 */
} RCC_TypeDef;

#define RCC                     ((RCC_TypeDef *)RCC_BASE)

/* RCC bit fields */
#define RCC_AHB4ENR_GPIOAEN     (1UL << 0)    /* GPIOA clock enable */
#define RCC_AHB4ENR_GPIOBEN     (1UL << 1)    /* GPIOB clock enable */
#define RCC_AHB4ENR_GPIODEN     (1UL << 3)    /* GPIOD clock enable */
#define RCC_AHB4ENR_GPIOFEN     (1UL << 5)    /* GPIOF clock enable (PF8/PF9 for W25Q64) */
#define RCC_AHB4ENR_GPIOGEN     (1UL << 6)    /* GPIOG clock enable */
#define RCC_AHB4ENR_GPIOHEN     (1UL << 7)    /* GPIOH clock enable */
#define RCC_AHB4ENR_GPIOIEN     (1UL << 8)    /* GPIOI clock enable */
#define RCC_APB1LENR_USART3EN   (1UL << 18)   /* USART3 clock enable (PB10/PB11) */
#define RCC_APB1LENR_I2C1EN     (1UL << 21)   /* I2C1 clock enable */
#define RCC_APB1LENR_I2C2EN     (1UL << 22)   /* I2C2 clock enable (Modify 2026.6.23 P1.5 v7) */
#define RCC_APB2ENR_USART1EN    (1UL << 4)    /* USART1 clock enable */
/* Modify 2026.6.23: AHB3 时钟 — QUADSPI 在这里 (W25Q64) */
#define RCC_AHB3ENR_QUADSPIEN   (1UL << 14)   /* QUADSPI clock enable */

/* Modify 2026.6.24 v18 P1.5: AHB3 复位 — 解决 BUSY 卡死 */
#define RCC_AHB3RSTR_QUADSPIRST (1UL << 14)   /* QUADSPI peripheral reset (H743 RM0433 8.7.30) */

/* Modify 2026.6.23 P1.5 v4: I2C1 时钟源选择 (RCC->D2CCIP2R bit 13:12 — I2C123SEL 共享) */
#define RCC_BASE_D2CCIP2R       (RCC_BASE + 0x54UL)   /* D2 peripherals clock config 2 */
#define RCC_D2CCIP2R_I2C123SEL_Pos (12U)
#define RCC_D2CCIP2R_I2C123SEL_Msk (3UL << RCC_D2CCIP2R_I2C123SEL_Pos)
#define RCC_D2CCIP2R_I2C123SEL_PCLK1    (0UL << RCC_D2CCIP2R_I2C123SEL_Pos)
#define RCC_D2CCIP2R_I2C123SEL_PLL3R    (1UL << RCC_D2CCIP2R_I2C123SEL_Pos)
#define RCC_D2CCIP2R_I2C123SEL_HSIKER   (2UL << RCC_D2CCIP2R_I2C123SEL_Pos)  /* hsi_ker_ck = 16MHz */
#define RCC_D2CCIP2R_I2C123SEL_CSIKER   (3UL << RCC_D2CCIP2R_I2C123SEL_Pos)
#define REG_D2CCIP2R            ((volatile uint32_t *)RCC_BASE_D2CCIP2R)

/* Modify 2026.6.23 P1.5 v10: 绝对地址访问 APB1LENR (RCC 偏移 0xE8) */
#define RCC_BASE_APB1LENR       (RCC_BASE + 0xE8UL)
#define REG_APB1LENR            ((volatile uint32_t *)RCC_BASE_APB1LENR)

/*============================================================================
 * GPIO base (GPIOD for DI1=PD11)
 *  H743 GPIO 寄存器:
 *   MODER  = 0x00
 *   OTYPER = 0x04
 *   OSPEEDR= 0x08
 *   PUPDR  = 0x0C
 *   IDR    = 0x10
 *   ODR    = 0x14
 *   BSRR   = 0x18
 *   LCKR   = 0x1C
 *   AFR[0] = 0x20
 *   AFR[1] = 0x24
 *===========================================================================*/
#define GPIOA_BASE              0x58020000UL
#define GPIOB_BASE              0x58020400UL
#define GPIOD_BASE              0x58020C00UL
/* Modify 2026.6.23: 加 GPIOF — W25Q64 用 PF8/PF9 (IO0/IO1) */
#define GPIOF_BASE              0x58021400UL
#define GPIOG_BASE              0x58021800UL
#define GPIOH_BASE              0x58021C00UL
#define GPIOI_BASE              0x58022000UL
typedef struct {
    REG32 MODER;        /* 0x00 */
    REG32 OTYPER;       /* 0x04 */
    REG32 OSPEEDR;      /* 0x08 */
    REG32 PUPDR;        /* 0x0C */
    REG32 IDR;          /* 0x10 */
    REG32 ODR;          /* 0x14 */
    REG32 BSRR;         /* 0x18 */
    REG32 LCKR;         /* 0x1C */
    REG32 AFR[2];       /* 0x20, 0x24 */
    REG32 Reserved;     /* 0x28 */
    REG32 BRR;          /* 0x28 (alias) */
    REG32 Reserved2;    /* 0x2C */
} GPIO_TypeDef;

#define GPIOA                   ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                   ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOD                   ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOF                   ((GPIO_TypeDef *)GPIOF_BASE)
#define GPIOG                   ((GPIO_TypeDef *)GPIOG_BASE)
#define GPIOH                   ((GPIO_TypeDef *)GPIOH_BASE)
#define GPIOI                   ((GPIO_TypeDef *)GPIOI_BASE)

/* GPIO mode */
#define GPIO_MODE_INPUT         0x0   /* 00: Input */
#define GPIO_MODE_OUTPUT        0x1   /* 01: General purpose output */
#define GPIO_MODE_AF            0x2   /* 10: Alternate function */
#define GPIO_MODE_ANALOG        0x3   /* 11: Analog */

/*============================================================================
 * USART1 base (PA9/PA10)
 *   0x40011000
 *   CR1  = 0x00
 *   CR2  = 0x04
 *   CR3  = 0x08
 *   BRR  = 0x0C
 *   GTPR = 0x10
 *   RTOR = 0x14
 *   RQR  = 0x18
 *   ISR  = 0x1C
 *   ICR  = 0x20
 *   RDR  = 0x24
 *   TDR  = 0x28
 *===========================================================================*/
#define USART1_BASE             0x40011000UL
#define USART3_BASE             0x40004800UL
typedef struct {
    REG32 CR1;          /* 0x00 */
    REG32 CR2;          /* 0x04 */
    REG32 CR3;          /* 0x08 */
    REG32 BRR;          /* 0x0C */
    REG16 GTPR;         /* 0x10 */
    uint16_t Reserved1;
    REG32 RTOR;         /* 0x14 */
    REG32 RQR;          /* 0x18 */
    REG32 ISR;          /* 0x1C */
    REG32 ICR;          /* 0x20 */
    REG32 RDR;          /* 0x24 */
    REG32 TDR;          /* 0x28 */
} USART_TypeDef;

#define USART1                  ((USART_TypeDef *)USART1_BASE)
#define USART3                  ((USART_TypeDef *)USART3_BASE)

/* USART1 bit fields */
#define USART_CR1_UE            (1UL << 0)
#define USART_CR1_TE            (1UL << 3)
#define USART_CR1_RE            (1UL << 2)
#define USART_CR1_M0            (1UL << 12)   /* 8-bit: M0=0, M1=0 */
#define USART_CR1_M1            (1UL << 28)   /* 7/8/9 bit */
#define USART_CR1_OVER8         (1UL << 15)
#define USART_CR1_RXNEIE        (1UL << 5)    /* Modify 2026.6.24 v23: RXNE interrupt enable (BL ring buf 用) */
#define USART_CR1_TXEIE         (1UL << 7)    /* TXE interrupt enable (未用) */
#define USART_ISR_TXE           (1UL << 7)    /* TX empty */
#define USART_ISR_TC            (1UL << 6)    /* TX complete */
#define USART_ISR_RXNE          (1UL << 5)    /* RX not empty */
#define USART_ISR_ORE           (1UL << 3)    /* Overrun error */

/*============================================================================
 * NVIC (Modify 2026.6.24 v23: BL 用 RXNE 中断, 需要 ISER/ICER + IRQ 编号)
 *  H743 NVIC 基址 0xE000E100, ISER0..7 (set enable), ICER0..7 (clear enable)
 *  USART3_IRQn = 39 (按 H743 中断向量表)
 *===========================================================================*/
#define NVIC_BASE               0xE000E100UL
#define NVIC_ISER0              ((volatile uint32_t *)(NVIC_BASE + 0x000))
#define NVIC_ISER1              ((volatile uint32_t *)(NVIC_BASE + 0x004))
#define NVIC_ISER2              ((volatile uint32_t *)(NVIC_BASE + 0x008))
#define NVIC_ICER0              ((volatile uint32_t *)(NVIC_BASE + 0x080))
#define NVIC_ICER1              ((volatile uint32_t *)(NVIC_BASE + 0x084))
#define NVIC_ICER2              ((volatile uint32_t *)(NVIC_BASE + 0x088))

/* IRQ 编号 (H743, 按 RM0433 表) */
#define USART1_IRQn             37
#define USART2_IRQn             38
#define USART3_IRQn             39

static __inline void NVIC_EnableIRQ(uint32_t irqn)
{
    if (irqn < 32)      *NVIC_ISER0 = (1UL << irqn);
    else if (irqn < 64) *NVIC_ISER1 = (1UL << (irqn - 32));
    else                *NVIC_ISER2 = (1UL << (irqn - 64));
}
static __inline void NVIC_DisableIRQ(uint32_t irqn)
{
    if (irqn < 32)      *NVIC_ICER0 = (1UL << irqn);
    else if (irqn < 64) *NVIC_ICER1 = (1UL << (irqn - 32));
    else                *NVIC_ICER2 = (1UL << (irqn - 64));
}

/*============================================================================
 * I2C1 base (PB8/PB9 - SCL/SDA on AMKN8639)
 *  实际引脚由 product_config.h 决定，但 EEPROM 通常在固定 I2C bus
 *  按 AMKN8639 板默认: I2C1
 *  0x40005400
 *===========================================================================*/
#define I2C1_BASE               0x40005400UL
/* Modify 2026.6.23 P1.5 v7: 加 I2C2 定义 */
#define I2C2_BASE               0x40005800UL
typedef struct {
    REG32 CR1;          /* 0x00 */
    REG32 CR2;          /* 0x04 */
    REG32 OAR1;         /* 0x08 */
    REG32 OAR2;         /* 0x0C */
    REG32 TIMINGR;      /* 0x10 */
    REG32 TIMEOUTR;     /* 0x14 */
    REG32 ISR;          /* 0x18 */
    REG32 ICR;          /* 0x1C */
    REG32 PECR;         /* 0x20 */
    REG32 RXDR;         /* 0x24 */
    REG32 TXDR;         /* 0x28 */
} I2C_TypeDef;

#define I2C1                    ((I2C_TypeDef *)I2C1_BASE)
#define I2C2                    ((I2C_TypeDef *)I2C2_BASE)

#define I2C_CR1_PE              (1UL << 0)
#define I2C_CR1_START           (1UL << 13)
#define I2C_CR1_STOP            (1UL << 14)
#define I2C_CR1_ACK             (1UL << 10)
#define I2C_CR1_SWRST           (1UL << 15)
#define I2C_CR2_NBYTES_SHIFT    16
#define I2C_CR2_RD_WRN          (1UL << 10)
#define I2C_CR2_SADD_SHIFT      0
#define I2C_ISR_TXE             (1UL << 0)
#define I2C_ISR_TXIS            (1UL << 1)
#define I2C_ISR_RXNE            (1UL << 2)
#define I2C_ISR_ADDR            (1UL << 3)
#define I2C_ISR_NACKF           (1UL << 4)
#define I2C_ISR_STOPF           (1UL << 5)
#define I2C_ISR_TC              (1UL << 6)
#define I2C_ISR_TCR             (1UL << 7)
#define I2C_ISR_BERR            (1UL << 8)
#define I2C_ISR_ARLO            (1UL << 9)
#define I2C_ISR_OVR             (1UL << 10)
#define I2C_ISR_PECERR          (1UL << 11)
#define I2C_ISR_TIMEOUT         (1UL << 12)
#define I2C_ISR_ALERT           (1UL << 13)
#define I2C_ISR_BUSY            (1UL << 15)
#define I2C_ISR_DIR             (1UL << 16)
#define I2C_ISR_ADDCODE_SHIFT   17

/* I2C timing for 100kHz @ 64MHz I2CCLK (SYSCLK default after BL) */
#define I2C_TIMING_100KHZ       0x10707DBCUL
#define I2C_TIMING_400KHZ       0x00602173UL

/*============================================================================
 * FLASH base (FLASH controller)
 *  0x52002000
 *  ACR    = 0x00
 *  KEYR   = 0x04
 *  OPTKEYR= 0x08
 *  CR     = 0x0C
 *  SR     = 0x10
 *  CCR    = 0x14
 *  OPTCR  = 0x18
 *  OPTSR  = 0x1C
 *  OPTCCR = 0x24
 *===========================================================================*/
#define FLASH_BASE              0x52002000UL
typedef struct {
    REG32 ACR;          /* 0x00 */
    REG32 KEYR;         /* 0x04 */
    REG32 OPTKEYR;      /* 0x08 */
    REG32 CR;           /* 0x0C */
    REG32 SR;           /* 0x10 */
    REG32 CCR;          /* 0x14 */
    REG32 OPTCR;        /* 0x18 */
    REG32 OPTSR;        /* 0x1C */
    REG32 OPTCCR;       /* 0x20 */
    REG32 PRAR;         /* 0x24 */
    REG32 SCAR;         /* 0x28 */
    REG32 WPSN;         /* 0x2C */
    REG32 CRCR;         /* 0x30 */
    REG32 CRCS;         /* 0x34 */
} FLASH_TypeDef;

#define FLASH                   ((FLASH_TypeDef *)FLASH_BASE)

/* Flash key */
#define FLASH_KEY1              0x45670123UL
#define FLASH_KEY2              0xCDEF89ABUL

/* Flash CR bits */
#define FLASH_CR_PG             (1UL << 0)
#define FLASH_CR_SER            (1UL << 1)
#define FLASH_CR_BER            (1UL << 2)    /* Mass erase (bank 1+2) */
#define FLASH_CR_PSIZE_SHIFT    4
#define FLASH_CR_PSIZE_32       (1UL << FLASH_CR_PSIZE_SHIFT)
#define FLASH_CR_PSIZE_64       (2UL << FLASH_CR_PSIZE_SHIFT)
#define FLASH_CR_FW_START       (1UL << 15)   /* Force write start (H7) */
#define FLASH_CR_START          (1UL << 16)   /* Start erase/program */
#define FLASH_CR_SNB_SHIFT      8            /* Sector number, bits 8..13 */
#define FLASH_CR_SNB_MASK       (0x3FUL << FLASH_CR_SNB_SHIFT)
#define FLASH_CR_LOCK           (1UL << 31)

/* Flash SR bits */
#define FLASH_SR_BSY            (1UL << 0)
#define FLASH_SR_QW             (1UL << 2)    /* Queue wait (H7) */
#define FLASH_SR_WRPERR         (1UL << 8)
#define FLASH_SR_PGSERR         (1UL << 9)
#define FLASH_SR_STRBERR        (1UL << 10)   /* H743 strobe error */
#define FLASH_SR_INCERR         (1UL << 11)   /* H743 */
#define FLASH_SR_OPERR          (1UL << 12)   /* H743 */
#define FLASH_SR_RDPERR         (1UL << 13)   /* H743 RDP */
#define FLASH_SR_RDSERR         (1UL << 14)   /* H743 RDS */
#define FLASH_SR_SNECCERR       (1UL << 15)
#define FLASH_SR_DBECCERR       (1UL << 16)
#define FLASH_SR_EOP            (1UL << 16)   /* End of operation */
#define FLASH_SR_CRCEND         (1UL << 27)

#define FLASH_ERR_MASK          (FLASH_SR_WRPERR | FLASH_SR_PGSERR | FLASH_SR_STRBERR | \
                                 FLASH_SR_INCERR | FLASH_SR_OPERR | FLASH_SR_RDPERR | \
                                 FLASH_SR_RDSERR | FLASH_SR_SNECCERR | FLASH_SR_DBECCERR)

/*============================================================================
 * IWDG base
 *  0x58004800
 *  KR  = 0x00
 *  PR  = 0x04
 *  RLR = 0x08
 *  SR  = 0x0C
 *===========================================================================*/
#define IWDG_BASE               0x58004800UL
typedef struct {
    REG32 KR;
    REG32 PR;
    REG32 RLR;
    REG32 SR;
    REG32 WINR;
} IWDG_TypeDef;

#define IWDG                    ((IWDG_TypeDef *)IWDG_BASE)
#define IWDG_KEY_RELOAD         0x0000AAAAUL
#define IWDG_KEY_START          0x0000CCCCUL
#define IWDG_KEY_UNLOCK         0x00005555UL

/*============================================================================
 * SCB (System Control Block) - VTOR
 *===========================================================================*/
#define SCS_BASE                0xE000E000UL
typedef struct {
    REG32 CPUID;        /* 0x00 */
    REG32 ICSR;         /* 0x04 */
    REG32 VTOR;         /* 0x08 */
    REG32 AIRCR;        /* 0x0C */
    REG32 SCR;          /* 0x10 */
    REG32 CCR;          /* 0x14 */
    REG32 SHPR[3];      /* 0x18, 0x1C, 0x20 */
    REG32 SHCSR;        /* 0x24 */
    REG32 CFSR;         /* 0x28 */
    REG32 HFSR;         /* 0x2C */
    REG32 DFSR;         /* 0x30 */
    REG32 MMFAR;        /* 0x34 */
    REG32 BFAR;         /* 0x38 */
    REG32 AFSR;         /* 0x3C */
} SCB_Type;

#define SCB                     ((SCB_Type *)SCS_BASE)

/* AIRCR reset */
#define SCB_AIRCR_VECTKEY       (0x5FAUL << 16)
#define SCB_AIRCR_SYSRESETREQ   (1UL << 2)

/*============================================================================
 * CMSIS 兼容内联函数 (避免依赖 cmsis_compiler.h)
 *  ARMCC 5.05 C89 模式:
 *   - 不识别 __attribute__((always_inline)), 用 __inline
 *   - 不识别 "::" GCC 语法, 用单 ":"
 *   - MSR MSP 必须用 r0 寄存器 (AAPCS), 不能用 %0 变量绑定
 *===========================================================================*/
static __inline void __bl_disable_irq(void)
{
    __asm("CPSID I");
}
static __inline void __bl_enable_irq(void)
{
    __asm("CPSIE I");
}
/* ARMCC 函数式内联汇编: r0 即第一个参数 (AAPCS)
 *  Modify 2026.6.23: __asm 函数不能 static, 改用 extern 声明
 *  真正定义移到 bootloader.c (避免多 .o 链接冲突) */
extern void __bl_set_msp(uint32_t sp);
#define __disable_irq   __bl_disable_irq
#define __enable_irq    __bl_enable_irq
#define __set_msp       __bl_set_msp

/*============================================================================
 * QUADSPI (Modify 2026.6.23: P1 W25Q64 驱动用)
 *  H743 QUADSPI 寄存器布局 (基地址 0x52005000, AHB3 总线):
 *   CR   = 0x00  控制 (EN, ABORT, DMAEN, FTHRES, FRCK, ...)
 *   DCR  = 0x04  设备配置 (FMSIZE=23=8MB, CSHT, CKMODE)
 *   SR   = 0x08  状态 (BUSY, TOF, SMF, FT, FRL, FTL)
 *   FCR  = 0x0C  标志清除 (CTOF, CSMF, CFTF, CFR, CFT)
 *   DLR  = 0x10  数据长度 (0-based: 0=1字节, N=N+1字节)
 *   CCR  = 0x14  通信配置 (FMODE, DMODE, DCYC, ADSIZE, ADMODE, IMODE, INSTR)
 *   AR   = 0x18  地址寄存器 (24-bit 或 32-bit)
 *   ABR  = 0x1C  替代字节 (不用)
 *   DR   = 0x20  数据 FIFO (32 字节)
 *===========================================================================*/
#define QUADSPI_BASE            0x52005000UL
typedef struct {
    REG32 CR;           /* 0x00 - Control */
    REG32 DCR;          /* 0x04 - Device Configuration */
    REG32 SR;           /* 0x08 - Status */
    REG32 FCR;          /* 0x0C - Flag Clear */
    REG32 DLR;          /* 0x10 - Data Length */
    REG32 CCR;          /* 0x14 - Communication Configuration */
    REG32 AR;           /* 0x18 - Address */
    REG32 ABR;          /* 0x1C - Alternate Bytes */
    REG32 DR;           /* 0x20 - Data (FIFO) */
    REG32 PSMKR;        /* 0x24 - Polling Status Mask */
    REG32 PSMAR;        /* 0x28 - Polling Status Match */
    REG32 PIR;          /* 0x2C - Polling Interval */
    REG32 LPTR;         /* 0x30 - Low-Power Timeout */
} QUADSPI_TypeDef;

#define QUADSPI                 ((QUADSPI_TypeDef *)QUADSPI_BASE)

/* CR bits */
#define QUADSPI_CR_EN           (1UL << 0)    /* Enable */
#define QUADSPI_CR_ABORT        (1UL << 1)    /* Abort ongoing transfer */
#define QUADSPI_CR_DMAEN        (1UL << 2)    /* DMA enable */
#define QUADSPI_CR_TCEN         (1UL << 3)    /* Timeout counter enable */
#define QUADSPI_CR_DPM          (1UL << 4)    /* Data pack mode (read only mode) */
#define QUADSPI_CR_FTHRES_SHIFT 8             /* FIFO threshold (0..15) */
#define QUADSPI_CR_PRESCALER_SHIFT 24         /* Clock prescaler: f_clk = f_source/(PRESCALER+1) */
#define QUADSPI_CR_FSEL         (1UL << 26)   /* Flash select (BK1/BK2) */
#define QUADSPI_CR_FRCK         (1UL << 30)   /* Force CS low (mem-mapped) */

/* DCR bits */
#define QUADSPI_DCR_CKMODE      (1UL << 0)    /* 0=mode0 (low), 1=mode3 (high) */
#define QUADSPI_DCR_CSHT_SHIFT  8             /* CS high time (cycles+1, 0..7) */
#define QUADSPI_DCR_FMSIZE_SHIFT 16           /* Flash size = 2^(FMSIZE+1) bytes */

/* SR bits */
#define QUADSPI_SR_BUSY         (1UL << 5)    /* BUSY (注意 H743 SR bit 5, 不是 bit 0) */
#define QUADSPI_SR_TOF          (1UL << 4)    /* Timeout flag */
#define QUADSPI_SR_SMF          (1UL << 3)    /* Status match flag */
#define QUADSPI_SR_FTF          (1UL << 2)    /* FIFO threshold flag */
#define QUADSPI_SR_TCF          (1UL << 1)    /* Transfer complete flag */
#define QUADSPI_SR_TEF          (1UL << 0)    /* Transfer error flag */

/* FCR bits */
#define QUADSPI_FCR_CTOF        (1UL << 4)
#define QUADSPI_FCR_CSMF        (1UL << 3)
#define QUADSPI_FCR_CTCF        (1UL << 1)
#define QUADSPI_FCR_CTEF        (1UL << 0)

/* CCR bits (Communication Configuration)
 *  Modify 2026.6.24 v16 P1.5: 全部按 ST 官方 CMSIS 头 stm32h743xx.h 修正
 *    INSTR 实际在 bits 7:0 (不是 23:16)
 *    FMODE 实际在 bits 27:26 (不是 1:0)
 *    DMODE 实际在 bits 25:24 (不是 3:2)
 *    DCYC  实际在 bits 22:18 (5 bits, 不是 7:4)
 *    ADSIZE实际在 bits 13:12 (不是 9:8)
 *    ABMODE实际在 bits 15:14
 *    ABSIZE实际在 bits 17:16
 *    ADMODE bits 11:10 ✓ (本来就对)
 *    IMODE 实际在 bits 9:8  (不是 13:12)
 *  错误源自早期对 RM0433 的误读, 之前 v13-v15 所有 CCR 写入都是错的
 *  验证: CCR=0x003B1A09 (v15 写 0xBB<<16) 实际硬件 INSTR=0x09 (bits 7:0), 不是 0xBB */
#define QUADSPI_CCR_INSTR_SHIFT  0            /* Instruction[7:0]  - 8-bit command */
#define QUADSPI_CCR_IMODE_SHIFT  8            /* 00=none, 01=1-line, 10=2-line, 11=4-line */
#define QUADSPI_CCR_ADMODE_SHIFT 10           /* 00=none, 01=1-line, 10=2-line, 11=4-line */
#define QUADSPI_CCR_ADSIZE_SHIFT 12           /* 00=8b, 01=16b, 10=24b, 11=32b */
#define QUADSPI_CCR_ABMODE_SHIFT 14           /* alternate byte mode */
#define QUADSPI_CCR_ABSIZE_SHIFT 16           /* alternate byte size */
#define QUADSPI_CCR_DCYC_SHIFT   18           /* Dummy cycles (5 bits, 0..31) */
#define QUADSPI_CCR_DMODE_SHIFT  24           /* 00=none, 01=1-line, 10=2-line, 11=4-line */
#define QUADSPI_CCR_FMODE_SHIFT  26           /* 00=ind write, 01=ind read, 10=auto poll, 11=mem-mapped */
#define QUADSPI_CCR_SIOO_SHIFT   28           /* Send Instruction Only Once (mem-mapped) */

/* 简化的 CCR 组合宏 (Modify 2026.6.24 v16: 全部按 ST CMSIS 修正) */
#define CCR_FMODE_IND_WRITE     (0x0UL << QUADSPI_CCR_FMODE_SHIFT)   /* 0x00000000 */
#define CCR_FMODE_IND_READ      (0x1UL << QUADSPI_CCR_FMODE_SHIFT)   /* 0x04000000 */
#define CCR_DMODE_NONE          (0x0UL << QUADSPI_CCR_DMODE_SHIFT)   /* 0x00000000 */
#define CCR_DMODE_1LINE         (0x1UL << QUADSPI_CCR_DMODE_SHIFT)   /* 0x01000000 */
#define CCR_DMODE_2LINE         (0x2UL << QUADSPI_CCR_DMODE_SHIFT)   /* 0x02000000 (vendor QSPI dual mode) */
#define CCR_DMODE_4LINE         (0x3UL << QUADSPI_CCR_DMODE_SHIFT)   /* 0x03000000 (vendor QSPI quad mode) */
#define CCR_ADSIZE_8            (0x0UL << QUADSPI_CCR_ADSIZE_SHIFT)
#define CCR_ADSIZE_16           (0x1UL << QUADSPI_CCR_ADSIZE_SHIFT)
#define CCR_ADSIZE_24           (0x2UL << QUADSPI_CCR_ADSIZE_SHIFT)   /* W25Q64 用 */
#define CCR_ADSIZE_32           (0x3UL << QUADSPI_CCR_ADSIZE_SHIFT)
#define CCR_ADMODE_NONE         (0x0UL << QUADSPI_CCR_ADMODE_SHIFT)
#define CCR_ADMODE_1LINE        (0x1UL << QUADSPI_CCR_ADMODE_SHIFT)   /* 0x00000400 */
#define CCR_ADMODE_2LINE        (0x2UL << QUADSPI_CCR_ADMODE_SHIFT)   /* 0x00000800 (vendor QSPI dual mode) */
#define CCR_ADMODE_4LINE        (0x3UL << QUADSPI_CCR_ADMODE_SHIFT)   /* 0x00000C00 (vendor QSPI quad mode) */
#define CCR_IMODE_NONE          (0x0UL << QUADSPI_CCR_IMODE_SHIFT)   /* 0x00000000 */
#define CCR_IMODE_1LINE         (0x1UL << QUADSPI_CCR_IMODE_SHIFT)   /* 0x00000100 (W25Q64 0x03/0xBB 等用) */
#define CCR_IMODE_2LINE         (0x2UL << QUADSPI_CCR_IMODE_SHIFT)   /* 0x00000200 */
#define CCR_IMODE_4LINE         (0x3UL << QUADSPI_CCR_IMODE_SHIFT)   /* 0x00000300 */

#endif /* __STM32H7XX_REGS_H__ */
