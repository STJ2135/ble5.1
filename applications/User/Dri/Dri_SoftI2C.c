#include "Dri_SoftI2C.h"
#include "n32wb03x_gpio.h"
#include "n32wb03x_rcc.h"
#include "ns_delay.h"

#define DRI_SOFT_I2C_PORT      GPIOB
#define DRI_SOFT_I2C_SCL_PIN   GPIO_PIN_7
#define DRI_SOFT_I2C_SDA_PIN   GPIO_PIN_6
#define DRI_SOFT_I2C_GPIO_CLK  RCC_APB2_PERIPH_GPIOB
#define DRI_SOFT_I2C_PIN_MODE  GPIO_MODE_OUTPUT_OD

static void Dri_SoftI2C_SetSda(bool is_high)
{
    if (is_high)
    {
        GPIO_SetBits(DRI_SOFT_I2C_PORT, DRI_SOFT_I2C_SDA_PIN);
    }
    else
    {
        GPIO_ResetBits(DRI_SOFT_I2C_PORT, DRI_SOFT_I2C_SDA_PIN);
    }
}

static void Dri_SoftI2C_SetScl(bool is_high)
{
    if (is_high)
    {
        GPIO_SetBits(DRI_SOFT_I2C_PORT, DRI_SOFT_I2C_SCL_PIN);
    }
    else
    {
        GPIO_ResetBits(DRI_SOFT_I2C_PORT, DRI_SOFT_I2C_SCL_PIN);
    }
}

void Dri_SoftI2C_Init(void)
{
    GPIO_InitType st_gpio_init;

    Dri_SoftI2C_SetSda(true);
    Dri_SoftI2C_SetScl(true);
    RCC_EnableAPB2PeriphClk(DRI_SOFT_I2C_GPIO_CLK, ENABLE);

    GPIO_InitStruct(&st_gpio_init);
    st_gpio_init.Pin = DRI_SOFT_I2C_SDA_PIN | DRI_SOFT_I2C_SCL_PIN;
    st_gpio_init.GPIO_Mode = DRI_SOFT_I2C_PIN_MODE;
    GPIO_InitPeripheral(DRI_SOFT_I2C_PORT, &st_gpio_init);
}

void Dri_SoftI2C_Start(void)
{
    Dri_SoftI2C_SetSda(true);
    Dri_SoftI2C_SetScl(true);
    delay_n_10us(1U);
    Dri_SoftI2C_SetSda(false);
    delay_n_10us(1U);
    Dri_SoftI2C_SetScl(false);
}

void Dri_SoftI2C_Stop(void)
{
    Dri_SoftI2C_SetScl(false);
    Dri_SoftI2C_SetSda(false);
    delay_n_10us(1U);
    Dri_SoftI2C_SetScl(true);
    delay_n_10us(1U);
    Dri_SoftI2C_SetSda(true);
    delay_n_10us(1U);
}

bool Dri_SoftI2C_IsAckReceived(void)
{
    uint8_t error_count = 0U;

    Dri_SoftI2C_SetSda(true);
    delay_n_10us(1U);
    Dri_SoftI2C_SetScl(true);
    delay_n_10us(1U);

    while (GPIO_ReadInputDataBit(DRI_SOFT_I2C_PORT, DRI_SOFT_I2C_SDA_PIN) != 0U)
    {
        error_count++;
        if (error_count > 250U)
        {
            return false;
        }
    }

    Dri_SoftI2C_SetScl(false);
    return true;
}

void Dri_SoftI2C_WriteByte(uint8_t data)
{
    uint8_t bit_index;

    Dri_SoftI2C_SetScl(false);
    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
        Dri_SoftI2C_SetSda((data & 0x80U) != 0U);
        data <<= 1U;
        delay_n_10us(1U);
        Dri_SoftI2C_SetScl(true);
        delay_n_10us(1U);
        Dri_SoftI2C_SetScl(false);
        delay_n_10us(1U);
    }
}

uint8_t Dri_SoftI2C_ReadByte(bool acknowledge)
{
    uint8_t data = 0U;
    uint8_t bit_index;

    Dri_SoftI2C_SetSda(true);
    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
        data <<= 1U;
        Dri_SoftI2C_SetScl(true);
        delay_n_10us(1U);
        if (GPIO_ReadInputDataBit(DRI_SOFT_I2C_PORT, DRI_SOFT_I2C_SDA_PIN) != 0U)
        {
            data |= 0x01U;
        }
        Dri_SoftI2C_SetScl(false);
        delay_n_10us(1U);
    }

    Dri_SoftI2C_SetSda(!acknowledge);
    Dri_SoftI2C_SetScl(true);
    delay_n_10us(1U);
    Dri_SoftI2C_SetScl(false);
    Dri_SoftI2C_SetSda(true);

    return data;
}
