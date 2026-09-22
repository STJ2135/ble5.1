#include "Int_RELAY.h"
#include "n32wb03x_gpio.h"
#include "n32wb03x_rcc.h"

#define INT_RELAY_GPIO_PORT GPIOB
#define INT_RELAY_GPIO_PIN  GPIO_PIN_11

void Int_RELAY_Init(void)
{
    GPIO_InitType st_gpio_init;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    GPIO_InitStruct(&st_gpio_init);
    st_gpio_init.Pin = INT_RELAY_GPIO_PIN;
    st_gpio_init.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitPeripheral(INT_RELAY_GPIO_PORT, &st_gpio_init);
    Int_RELAY_TurnOff();
}

void Int_RELAY_TurnOn(void)
{
    GPIO_SetBits(INT_RELAY_GPIO_PORT, INT_RELAY_GPIO_PIN);
}

void Int_RELAY_TurnOff(void)
{
    GPIO_ResetBits(INT_RELAY_GPIO_PORT, INT_RELAY_GPIO_PIN);
}
