#ifndef INT_LED_H
#define INT_LED_H

typedef enum
{
    INT_LED_ID_1 = 0,
    INT_LED_ID_2,
    INT_LED_ID_3
} Int_LED_IdType;

void Int_LED_InitAll(void);
void Int_LED_TurnOn(Int_LED_IdType led_id);
void Int_LED_TurnOff(Int_LED_IdType led_id);
void Int_LED_Toggle(Int_LED_IdType led_id);

#endif

