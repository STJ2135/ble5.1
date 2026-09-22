#include "App_TIMER.h"
#include "Mid_TIME.h"

#define APP_TIMER_TICKS_PER_SECOND 1000U

typedef enum
{
    APP_TIMER_STATE_STOP = 0,
    APP_TIMER_STATE_RUN
} App_TIMER_StateType;

static App_TIMER_StateType s_app_timer_state;
static uint32_t s_app_timer_remaining_seconds;
static uint32_t s_app_timer_last_tick;

void App_TIMER_Init(void)
{
    s_app_timer_state = APP_TIMER_STATE_STOP;
    s_app_timer_remaining_seconds = 0U;
    s_app_timer_last_tick = Mid_TIME_GetMillisecond();
}

void App_TIMER_SetSeconds(uint32_t seconds)
{
    s_app_timer_remaining_seconds = seconds;
    s_app_timer_last_tick = Mid_TIME_GetMillisecond();
}

void App_TIMER_Start(void)
{
    s_app_timer_state = APP_TIMER_STATE_RUN;
    s_app_timer_last_tick = Mid_TIME_GetMillisecond();
}

void App_TIMER_Stop(void)
{
    s_app_timer_state = APP_TIMER_STATE_STOP;
    s_app_timer_remaining_seconds = 0U;
}

uint32_t App_TIMER_GetRemainingSeconds(void)
{
    return s_app_timer_remaining_seconds;
}

void App_TIMER_Process(void)
{
    uint32_t current_tick;

    if (s_app_timer_state != APP_TIMER_STATE_RUN)
    {
        return;
    }

    current_tick = Mid_TIME_GetMillisecond();
    if ((current_tick - s_app_timer_last_tick) < APP_TIMER_TICKS_PER_SECOND)
    {
        return;
    }

    s_app_timer_last_tick = current_tick;
    if (s_app_timer_remaining_seconds > 0U)
    {
        s_app_timer_remaining_seconds--;
    }
}
