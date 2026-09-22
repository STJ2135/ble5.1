#include "Mid_TIME.h"
#include "Dri_TICK.h"

void Mid_TIME_Init(void)
{
    Dri_TICK_Init();
}

uint32_t Mid_TIME_GetMillisecond(void)
{
    return Dri_TICK_GetMillisecond();
}
