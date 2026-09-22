#include "Int_TM1650.h"
#include "Dri_SoftI2C.h"

#define INT_TM1650_CONTROL_ADDR 0x48U

static const uint8_t s_tm1650_segment_map[16] =
{
    0x3FU, 0x06U, 0x5BU, 0x4FU, 0x66U, 0x6DU, 0x7DU, 0x07U,
    0x7FU, 0x6FU, 0x77U, 0x7CU, 0x39U, 0x5EU, 0x79U, 0x71U
};
static const uint8_t s_tm1650_digit_address[4] = {0x68U, 0x6AU, 0x6CU, 0x6EU};

static void Int_TM1650_WriteCommand(uint8_t address, uint8_t data)
{
    Dri_SoftI2C_Start();
    Dri_SoftI2C_WriteByte(address);
    (void)Dri_SoftI2C_IsAckReceived();
    Dri_SoftI2C_WriteByte(data);
    (void)Dri_SoftI2C_IsAckReceived();
    Dri_SoftI2C_Stop();
}

static void Int_TM1650_Display(uint8_t first,
                                 uint8_t second,
                                 uint8_t third,
                                 uint8_t fourth,
                                 uint8_t point)
{
    uint8_t digit_index;
    uint8_t data_array[4];

    data_array[0] = first;
    data_array[1] = second;
    data_array[2] = third;
    data_array[3] = fourth;

    for (digit_index = 0U; digit_index < 4U; digit_index++)
    {
        Dri_SoftI2C_Start();
        Dri_SoftI2C_WriteByte(s_tm1650_digit_address[digit_index]);
        (void)Dri_SoftI2C_IsAckReceived();
        Dri_SoftI2C_WriteByte((uint8_t)(s_tm1650_segment_map[data_array[digit_index]] |
                                        (point != 0U ? 0x80U : 0x00U)));
        (void)Dri_SoftI2C_IsAckReceived();
        Dri_SoftI2C_Stop();
    }
}

static void Int_TM1650_SecondsToTime(uint32_t seconds,
                                       uint8_t *p_hour,
                                       uint8_t *p_minute,
                                       uint8_t *p_second)
{
    *p_hour = (uint8_t)(seconds / 3600U);
    *p_minute = (uint8_t)((seconds % 3600U) / 60U);
    *p_second = (uint8_t)(seconds % 60U);
}

static void Int_TM1650_ClearDisplay(void)
{
    uint8_t digit_index;

    for (digit_index = 0U; digit_index < 4U; digit_index++)
    {
        Int_TM1650_WriteCommand(s_tm1650_digit_address[digit_index], 0x00U);
    }
}

void Int_TM1650_SetPowerBrightness(Int_TM1650_PowerType power,
                                   Int_TM1650_BrightnessType brightness)
{
    uint8_t control_value;

    control_value = (uint8_t)(((uint8_t)brightness << 4U) | (uint8_t)power);
    Int_TM1650_WriteCommand(INT_TM1650_CONTROL_ADDR, control_value);
}

void Int_TM1650_Init(void)
{
    Dri_SoftI2C_Init();
    Int_TM1650_SetPowerBrightness(INT_TM1650_POWER_ON, INT_TM1650_BRIGHTNESS_8);
    Int_TM1650_ClearDisplay();
}

void Int_TM1650_DisplayTime(uint32_t seconds)
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    Int_TM1650_SecondsToTime(seconds, &hour, &minute, &second);
    if (seconds < 3600U)
    {
        Int_TM1650_Display((uint8_t)(minute / 10U),
                             (uint8_t)(minute % 10U),
                             (uint8_t)(second / 10U),
                             (uint8_t)(second % 10U),
                             1U);
    }
    else
    {
        Int_TM1650_Display((uint8_t)(hour / 10U),
                             (uint8_t)(hour % 10U),
                             (uint8_t)(minute / 10U),
                             (uint8_t)(minute % 10U),
                             1U);
    }
}


