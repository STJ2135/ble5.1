#ifndef APP_TIMER_H
#define APP_TIMER_H

#include <stdint.h>

void App_TIMER_Init(void);
void App_TIMER_SetSeconds(uint32_t seconds);
void App_TIMER_Start(void);
void App_TIMER_Stop(void);
uint32_t App_TIMER_GetRemainingSeconds(void);
void App_TIMER_Process(void);

#endif
