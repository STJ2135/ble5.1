/**
 * @file main.c
 * @brief Bluetooth timing controller
 */

#include "App_RUNTIME.h"
#include "rwip.h"
#include "ns_sleep.h"

void app_sleep_prepare_proc(void)
{
}

void app_sleep_resume_proc(void)
{
}

int main(void)
{
    App_RUNTIME_Init();

    while (1)
    {
        rwip_schedule();
        App_RUNTIME_Process();
    }
}
