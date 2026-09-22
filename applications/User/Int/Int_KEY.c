#include "Int_KEY.h"
#include "Dri_TICK.h"
#include "n32wb03x_gpio.h"
#include "n32wb03x_rcc.h"

#define INT_KEY_GPIO_PORT      GPIOA
#define INT_KEY_1_PIN          GPIO_PIN_0
#define INT_KEY_2_PIN          GPIO_PIN_1
#define INT_KEY_SCAN_PERIOD_MS 10U

static uint32_t s_key_1_time;
static uint32_t s_key_2_time;

void Int_KEY_Init(void)
{
    GPIO_InitType st_gpio_init;

    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    GPIO_InitStruct(&st_gpio_init);
    st_gpio_init.Pin = INT_KEY_1_PIN | INT_KEY_2_PIN;
    st_gpio_init.GPIO_Mode = GPIO_MODE_INPUT;
    GPIO_InitPeripheral(INT_KEY_GPIO_PORT, &st_gpio_init);
}

void Int_KEY_Scan(void)
{
    uint32_t current_time = Dri_TICK_GetMillisecond();

    if ((current_time - s_key_1_time) > INT_KEY_SCAN_PERIOD_MS)
    {
        s_key_1_time = current_time;
        (void)GPIO_ReadInputDataBit(INT_KEY_GPIO_PORT, INT_KEY_1_PIN);
    }

    if ((current_time - s_key_2_time) > INT_KEY_SCAN_PERIOD_MS)
    {
        s_key_2_time = current_time;
        (void)GPIO_ReadInputDataBit(INT_KEY_GPIO_PORT, INT_KEY_2_PIN);
    }
}
