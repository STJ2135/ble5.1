#include "App_RUNTIME.h"
#include "App_BOARD.h"
#include "App_COMMAND.h"
#include "App_TIMER.h"
#include "Mid_TIME.h"
#include "app.h"

void App_RUNTIME_Init(void)
{
    Mid_TIME_Init();
    App_BOARD_Init();
    App_TIMER_Init();
    App_COMMAND_Init();
    app_core_init();
}

void App_RUNTIME_Process(void)
{
    App_TIMER_Process();
    App_COMMAND_Process();
    App_BOARD_Process();
}
