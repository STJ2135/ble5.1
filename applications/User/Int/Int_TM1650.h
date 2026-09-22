#ifndef INT_TM1650_H
#define INT_TM1650_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    INT_TM1650_POWER_OFF = 0,
    INT_TM1650_POWER_ON
} Int_TM1650_PowerType;

typedef enum
{
    INT_TM1650_BRIGHTNESS_8 = 0,
    INT_TM1650_BRIGHTNESS_1,
    INT_TM1650_BRIGHTNESS_2,
    INT_TM1650_BRIGHTNESS_3,
    INT_TM1650_BRIGHTNESS_4,
    INT_TM1650_BRIGHTNESS_5,
    INT_TM1650_BRIGHTNESS_6,
    INT_TM1650_BRIGHTNESS_7
} Int_TM1650_BrightnessType;

void Int_TM1650_Init(void);
void Int_TM1650_SetPowerBrightness(Int_TM1650_PowerType power,
                                     Int_TM1650_BrightnessType brightness);
void Int_TM1650_DisplayTime(uint32_t seconds);

#endif


