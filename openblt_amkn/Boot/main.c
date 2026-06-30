#include "boot.h"
#include "stm32h7xx_hal.h"

#define BL_INITIAL_TIMEOUT_MS  5000

static volatile unsigned int g_tick = 0;
extern blt_bool ComIsConnected(void);
void TimerInit(void) { g_tick = 0; }
void TimerUpdate(void) { }
void TimerReset(void) { g_tick = 0; }
blt_int32u TimerGet(void) { return (blt_int32u)g_tick; }
unsigned int HAL_GetTick(void) { return g_tick; }

static void raw_putc(char c) {
    volatile unsigned int *ISR = (volatile unsigned int*)0x4001101Cu;
    volatile unsigned int *TDR = (volatile unsigned int*)0x40011028u;
    volatile int t = 1000000;
    while (!(*ISR & (1u << 7)) && --t) {}
    *TDR = c;
}
static void raw_puts(const char *s) { while (*s) raw_putc(*s++); }
static void raw_puthex(unsigned int v) {
    int i; raw_putc('0'); raw_putc('x');
    for (i=28;i>=0;i-=4) { unsigned int n=(v>>i)&0xFu; raw_putc(n<10?('0'+n):('A'+n-10)); }
}

static void delay_1ms(void) {
    volatile unsigned int d = 64000;
    while (--d) { __asm("nop"); }
}

static void SystemClock_Config(void)
{
    #define RCC_APB1LENR (*(volatile unsigned int*)0x580244E8u)
    RCC_APB1LENR |= (1u << 28);
    (void)RCC_APB1LENR;
    #define PWR_CR1 (*(volatile unsigned int*)0x58024800u)
    PWR_CR1 |= (1u << 8);
    while (!(RCC->CR & RCC_CR_HSIRDY)) {}
    FLASH->ACR = 0x04u;
    SystemCoreClock = 64000000u;
}

int main(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    GPIOA->MODER   = (GPIOA->MODER & ~((3u<<18)|(3u<<20))) | (2u<<18) | (2u<<20);
    GPIOA->OTYPER &= ~(1u<<9);
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~(3u<<18)) | (3u<<18);
    GPIOA->PUPDR   = (GPIOA->PUPDR & ~((3u<<18)|(3u<<20))) | (1u<<18) | (1u<<20);
    GPIOA->AFR[1]  = (GPIOA->AFR[1] & ~((0xFu<<4)|(0xFu<<8))) | (7u<<4) | (7u<<8);
    USART1->CR1 = 0;
    USART1->BRR = 0x22B9u;
    USART1->CR1 = USART_CR1_FIFOEN | USART_CR1_UE;
    { volatile int t = 5000000; while (!(USART1->ISR & USART_ISR_REACK) && --t) {} }
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    { volatile int t = 5000000; while (!(USART1->ISR & USART_ISR_TEACK) && --t) {} }
    { volatile int t = 5000000; while (!(USART1->ISR & USART_ISR_REACK) && --t) {} }

    __HAL_RCC_GPIOI_CLK_ENABLE();
    GPIOI->MODER &= ~(3u<<16);
    GPIOI->MODER |= (1u<<16);
    GPIOI->BSRR = (1u<<8);

    /* delay removed */
    raw_puts("\r\n=== AMKN8639 BL V3.5d ===\r\n");

    HAL_Init();
    SystemClock_Config();
    CpuInit();
    CopInit();
    TimerInit();
    NvmInit();
    ComInit();
    raw_puts("Init OK. XCP window.\r\n");

    GPIOI->BSRR = (1u<<(8+16));

    {
        unsigned int last = 0;
        blt_bool booted = BLT_FALSE;
        {
            unsigned int tick_counter = 0;
            for (;;) {
                /* Fast poll: ~80us per BootTask call, ~13 calls/ms */
                { volatile unsigned int d = 5000; while (--d) { __asm("nop"); } }
                BootTask();
                tick_counter++;
                if (tick_counter >= 13) { tick_counter = 0; g_tick++; }

                if (g_tick - last > 2000) {
                    last = g_tick;
                    if (ComIsConnected() == BLT_FALSE) { raw_puts(".\r\n"); }
                }

                if (!booted && g_tick >= BL_INITIAL_TIMEOUT_MS) {
                    if (ComIsConnected() == BLT_TRUE) {
                    } else {
                        booted = BLT_TRUE;
                        raw_puts("Timeout, booting APP......\r\n");
                        CpuStartUserProgram();
                        raw_puts("APP boot failed. Stay in BL.\r\n");
                    }
                }
            }
        }
    }
}

blt_bool CpuUserProgramStartHook(void) { return BLT_TRUE; }
blt_bool NvmVerifyChecksumHook(void) { return BLT_TRUE; }
blt_bool NvmWriteChecksumHook(void) { return BLT_TRUE; }
void SysTick_Handler(void) {}
void HardFault_Handler(void) { raw_puts("\r\n!!! HARDFAULT !!!\r\n"); while(1) {} }