#include "Dri_TICK.h"
#include "n32wb03x.h"

#define DRI_TICK_FREQUENCY_HZ 1000U

static volatile uint32_t s_dri_tick_millisecond;

void SysTick_Handler(void)
{
    s_dri_tick_millisecond++;
}

void Dri_TICK_Init(void)
{
    SystemCoreClockUpdate();
    (void)SysTick_Config(SystemCoreClock / DRI_TICK_FREQUENCY_HZ);
}

uint32_t Dri_TICK_GetMillisecond(void)
{
    return s_dri_tick_millisecond;
}
