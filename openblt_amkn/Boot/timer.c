#include "boot.h"
#include "stm32h7xx.h"
#include "stm32h7xx_ll_bus.h"

#define TIMER_COUNTER_FREQ_HZ     (100000U)
#define TIMER_COUNTS_PER_MS       (TIMER_COUNTER_FREQ_HZ / 1000U)

static blt_int32u millisecond_counter;
static blt_int16u free_running_counter_last;

void TimerInit(void)
{
  blt_int32u pclk_tim_frequency = SystemCoreClock;
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
  TIM1->CR1 = TIM_CR1_CEN;
  TIM1->ARR = 65535U;
  TIM1->PSC = (pclk_tim_frequency / TIMER_COUNTER_FREQ_HZ) - 1U;
  TIM1->EGR |= TIM_EGR_UG;
  millisecond_counter = 0;
  free_running_counter_last = (blt_int16u)TIM1->CNT;
}

void TimerReset(void)
{
  LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_TIM1);
  LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_TIM1);
  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM1);
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
}

void TimerUpdate(void)
{
  blt_int16u now = (blt_int16u)TIM1->CNT;
  blt_int16u delta = now - free_running_counter_last;
  if (delta >= TIMER_COUNTS_PER_MS) {
    blt_int16u ms = delta / TIMER_COUNTS_PER_MS;
    millisecond_counter += ms;
    free_running_counter_last += (ms * TIMER_COUNTS_PER_MS);
  }
}

blt_int32u TimerGet(void) { TimerUpdate(); return millisecond_counter; }
uint32_t HAL_GetTick(void) { return TimerGet(); }