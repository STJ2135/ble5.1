#include "Int_LED.h"
#include "n32wb03x_gpio.h"
#include "n32wb03x_rcc.h"

#define INT_LED_PORT_A GPIOA
#define INT_LED_PORT_B GPIOB
#define INT_LED_1_PIN  GPIO_PIN_2
#define INT_LED_2_PIN  GPIO_PIN_3
#define INT_LED_3_PIN  GPIO_PIN_0

static GPIO_Module *Int_LED_GetPort(Int_LED_IdType led_id)
{
    return (led_id == INT_LED_ID_3) ? INT_LED_PORT_B : INT_LED_PORT_A;
}

static uint16_t Int_LED_GetPin(Int_LED_IdType led_id)
{
    uint16_t led_pin;

    switch (led_id)
    {
        case INT_LED_ID_1:
            led_pin = INT_LED_1_PIN;
            break;

        case INT_LED_ID_2:
            led_pin = INT_LED_2_PIN;
            break;

        case INT_LED_ID_3:
            led_pin = INT_LED_3_PIN;
            break;

        default:
            led_pin = 0U;
            break;
    }

    return led_pin;
}

static void Int_LED_InitOne(Int_LED_IdType led_id)
{
    GPIO_InitType st_gpio_init;

    GPIO_InitStruct(&st_gpio_init);
    st_gpio_init.Pin = Int_LED_GetPin(led_id);
    st_gpio_init.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitPeripheral(Int_LED_GetPort(led_id), &st_gpio_init);
}

void Int_LED_InitAll(void)
{
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB, ENABLE);
    Int_LED_InitOne(INT_LED_ID_1);
    Int_LED_InitOne(INT_LED_ID_2);
    Int_LED_InitOne(INT_LED_ID_3);
    Int_LED_TurnOff(INT_LED_ID_1);
    Int_LED_TurnOff(INT_LED_ID_2);
    Int_LED_TurnOff(INT_LED_ID_3);
}

void Int_LED_TurnOn(Int_LED_IdType led_id)
{
    GPIO_ResetBits(Int_LED_GetPort(led_id), Int_LED_GetPin(led_id));
}

void Int_LED_TurnOff(Int_LED_IdType led_id)
{
    GPIO_SetBits(Int_LED_GetPort(led_id), Int_LED_GetPin(led_id));
}

void Int_LED_Toggle(Int_LED_IdType led_id)
{
    GPIO_TogglePin(Int_LED_GetPort(led_id), Int_LED_GetPin(led_id));
}
