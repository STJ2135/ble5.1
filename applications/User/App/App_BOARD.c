#include "App_BOARD.h"
#include "App_TIMER.h"
#include "Int_BEEP.h"
#include "Int_RELAY.h"
#include "Int_TM1650.h"
#include "Int_LED.h"
#include "Mid_TIME.h"

#define APP_BOARD_DISPLAY_PERIOD_MS 500U

static uint32_t s_app_board_last_display_tick;

void App_BOARD_Init(void)
{
    Int_LED_InitAll();
    Int_BEEP_Init();
    Int_RELAY_Init();
    Int_TM1650_Init();
    Int_TM1650_DisplayTime(0U);
}

void App_BOARD_Process(void)
{
    uint32_t current_tick = Mid_TIME_GetMillisecond();

    if ((current_tick - s_app_board_last_display_tick) < APP_BOARD_DISPLAY_PERIOD_MS)
    {
        return;
    }

    s_app_board_last_display_tick = current_tick;
    Int_LED_Toggle(INT_LED_ID_3);
    Int_TM1650_DisplayTime(App_TIMER_GetRemainingSeconds());
}

