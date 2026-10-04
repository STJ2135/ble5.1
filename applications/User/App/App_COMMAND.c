#include "App_COMMAND.h"
#include "Mid_BLE.h"
#include "App_TIMER.h"
#include "Int_TM1650.h"
#include "Int_RELAY.h"
#include "Mid_FRAME.h"

typedef enum
{
    APP_COMMAND_FUNCTION_TIMING = 0x0001,
    APP_COMMAND_FUNCTION_TIMING_ACK = 0x0002,
    APP_COMMAND_FUNCTION_STOP = 0x0003,
    APP_COMMAND_FUNCTION_STOP_ACK = 0x0004,
    APP_COMMAND_FUNCTION_GET_TIME = 0x0005,
    APP_COMMAND_FUNCTION_GET_TIME_ACK = 0x0006
} App_COMMAND_FunctionType;

static uint8_t s_app_command_tx_buffer[MID_FRAME_MAX_FRAME_SIZE];

static void App_COMMAND_SendResponse(uint16_t function, const uint8_t *p_data, uint16_t length)
{
    uint32_t frame_length;

    frame_length = Mid_FRAME_Build(s_app_command_tx_buffer,
                                   sizeof(s_app_command_tx_buffer),
                                   p_data,
                                   length,
                                   function);
    if (frame_length > 0U)
    {
        Mid_BLE_SendNotify(s_app_command_tx_buffer, (uint16_t)frame_length);
    }
}

static void App_COMMAND_HandleTiming(const Mid_FRAME_Message_Struct *p_st_message)
{
    const uint8_t response_data[2] = {0xFFU, 0xFFU};
    uint16_t minutes;
    uint32_t seconds;

    if (p_st_message->length < 2U)
    {
        return;
    }

    minutes = (uint16_t)(((uint16_t)p_st_message->data[0] << 8U) | p_st_message->data[1]);
    seconds = (uint32_t)minutes * 60U;
    App_TIMER_SetSeconds(seconds);
    App_TIMER_Start();
    Int_TM1650_DisplayTime(seconds);
    Int_RELAY_TurnOn();
    App_COMMAND_SendResponse(APP_COMMAND_FUNCTION_TIMING_ACK, response_data, sizeof(response_data));
}

static void App_COMMAND_HandleStop(void)
{
    const uint8_t response_data[2] = {0xFFU, 0xFFU};

    App_TIMER_Stop();
    Int_RELAY_TurnOff();
    App_COMMAND_SendResponse(APP_COMMAND_FUNCTION_STOP_ACK, response_data, sizeof(response_data));
}

static void App_COMMAND_HandleGetTime(void)
{
    uint8_t response_data[2];
    uint32_t remaining_seconds;
    uint32_t remaining_minutes;

    remaining_seconds = App_TIMER_GetRemainingSeconds();
    remaining_minutes = remaining_seconds / 60U;
    if ((remaining_seconds % 60U) != 0U)
    {
        remaining_minutes++;
    }

    response_data[0] = (uint8_t)(remaining_minutes >> 8U);
    response_data[1] = (uint8_t)(remaining_minutes & 0xFFU);
    App_COMMAND_SendResponse(APP_COMMAND_FUNCTION_GET_TIME_ACK, response_data, sizeof(response_data));
    //App_TIMER_Start();
}

void App_COMMAND_Init(void)
{
    Mid_FRAME_Init();
}

void App_COMMAND_Process(void)
{
    Mid_FRAME_Message_Struct st_message;

    while (Mid_FRAME_IsMessageReady(&st_message))
    {
        switch (st_message.function)
        {
            case APP_COMMAND_FUNCTION_TIMING:
                App_COMMAND_HandleTiming(&st_message);
                break;

            case APP_COMMAND_FUNCTION_STOP:
                App_COMMAND_HandleStop();
                break;

            case APP_COMMAND_FUNCTION_GET_TIME:
                App_COMMAND_HandleGetTime();
                break;

            default:
                break;
        }
    }
}


