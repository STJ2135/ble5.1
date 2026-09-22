#include "Int_BEEP.h"
#include "n32wb03x_gpio.h"
#include "n32wb03x_rcc.h"

#define INT_BEEP_GPIO_PORT GPIOA
#define INT_BEEP_GPIO_PIN  GPIO_PIN_6

void Int_BEEP_Init(void)
{
    GPIO_InitType st_gpio_init;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    GPIO_InitStruct(&st_gpio_init);
    st_gpio_init.Pin = INT_BEEP_GPIO_PIN;
    st_gpio_init.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitPeripheral(INT_BEEP_GPIO_PORT, &st_gpio_init);
    Int_BEEP_TurnOff();
}

void Int_BEEP_TurnOn(void)
{
    GPIO_SetBits(INT_BEEP_GPIO_PORT, INT_BEEP_GPIO_PIN);
}

void Int_BEEP_TurnOff(void)
{
    GPIO_ResetBits(INT_BEEP_GPIO_PORT, INT_BEEP_GPIO_PIN);
}
