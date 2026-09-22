#include <string.h>
#include "Dri_UART.h"
#include "n32wb03x_dma.h"
#include "n32wb03x_gpio.h"
#include "n32wb03x_rcc.h"
#include "n32wb03x_usart.h"
#include "misc.h"
#include "ns_sleep.h"

#define DRI_UART_INDEX              USART2
#define DRI_UART_CLOCK              RCC_APB1_PERIPH_USART2
#define DRI_UART_GPIO_PORT          GPIOB
#define DRI_UART_GPIO_CLOCK         RCC_APB2_PERIPH_GPIOB
#define DRI_UART_RX_PIN             GPIO_PIN_5
#define DRI_UART_TX_PIN             GPIO_PIN_4
#define DRI_UART_RX_AF              GPIO_AF3_USART2
#define DRI_UART_TX_AF              GPIO_AF3_USART2
#define DRI_UART_DATA_ADDRESS       (USART2_BASE + 0x04U)
#define DRI_UART_TX_DMA_CHANNEL     DMA_CH1
#define DRI_UART_RX_DMA_CHANNEL     DMA_CH2
#define DRI_UART_TX_DMA_REMAP       DMA_REMAP_USART2_TX
#define DRI_UART_RX_DMA_REMAP       DMA_REMAP_USART2_RX
#define DRI_UART_TX_DMA_FLAG        DMA_FLAG_TC1
#define DRI_UART_RX_DMA_FLAG        DMA_FLAG_TC2
#define DRI_UART_RX_DMA_HALF_FLAG   DMA_FLAG_HT2
#define DRI_UART_IRQ_CHANNEL        USART2_IRQn
#define DRI_UART_BAUDRATE           115200U
#define DRI_UART_RX_DMA_SIZE        256U
#define DRI_UART_RX_BUFFER_SIZE     1000U
#define DRI_UART_TX_BUFFER_SIZE     1000U

static uint8_t s_dri_uart_rx_dma_buffer[DRI_UART_RX_DMA_SIZE];
static uint16_t s_dri_uart_rx_old_position;
static uint8_t s_dri_uart_rx_buffer[DRI_UART_RX_BUFFER_SIZE];
static volatile uint16_t s_dri_uart_rx_write_index;
static volatile uint16_t s_dri_uart_rx_read_index;
static uint8_t s_dri_uart_tx_buffer[DRI_UART_TX_BUFFER_SIZE];
static volatile uint16_t s_dri_uart_tx_write_index;
static volatile uint16_t s_dri_uart_tx_read_index;
static volatile uint16_t s_dri_uart_tx_dma_length;
static volatile bool s_dri_uart_tx_busy;
static Dri_UART_RxCallback s_dri_uart_rx_callback;

static void Dri_UART_ConfigureClock(void)
{
    RCC_EnableAPB2PeriphClk(DRI_UART_GPIO_CLOCK, ENABLE);
    RCC_EnableAPB1PeriphClk(DRI_UART_CLOCK, ENABLE);
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
}

static void Dri_UART_ConfigureGpio(void)
{
    GPIO_InitType st_gpio_init;

    GPIO_InitStruct(&st_gpio_init);
    st_gpio_init.Pin = DRI_UART_TX_PIN;
    st_gpio_init.GPIO_Mode = GPIO_MODE_AF_PP;
    st_gpio_init.GPIO_Alternate = DRI_UART_TX_AF;
    GPIO_InitPeripheral(DRI_UART_GPIO_PORT, &st_gpio_init);

    st_gpio_init.Pin = DRI_UART_RX_PIN;
    st_gpio_init.GPIO_Alternate = DRI_UART_RX_AF;
    GPIO_InitPeripheral(DRI_UART_GPIO_PORT, &st_gpio_init);
}

static void Dri_UART_ConfigureDma(void)
{
    DMA_InitType st_dma_init;

    DMA_DeInit(DRI_UART_TX_DMA_CHANNEL);
    DMA_RequestRemap(DRI_UART_TX_DMA_REMAP, DMA, DRI_UART_TX_DMA_CHANNEL, ENABLE);
    DMA_ConfigInt(DRI_UART_TX_DMA_CHANNEL, DMA_INT_TXC, ENABLE);

    DMA_DeInit(DRI_UART_RX_DMA_CHANNEL);
    DMA_RequestRemap(DRI_UART_RX_DMA_REMAP, DMA, DRI_UART_RX_DMA_CHANNEL, ENABLE);
    DMA_ConfigInt(DRI_UART_RX_DMA_CHANNEL, DMA_INT_TXC | DMA_INT_HTX, ENABLE);

    st_dma_init.PeriphAddr = DRI_UART_DATA_ADDRESS;
    st_dma_init.MemAddr = (uint32_t)s_dri_uart_rx_dma_buffer;
    st_dma_init.Direction = DMA_DIR_PERIPH_SRC;
    st_dma_init.BufSize = DRI_UART_RX_DMA_SIZE;
    st_dma_init.PeriphInc = DMA_PERIPH_INC_DISABLE;
    st_dma_init.DMA_MemoryInc = DMA_MEM_INC_ENABLE;
    st_dma_init.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    st_dma_init.MemDataSize = DMA_MemoryDataSize_Byte;
    st_dma_init.CircularMode = DMA_MODE_CIRCULAR;
    st_dma_init.Priority = DMA_PRIORITY_HIGH;
    st_dma_init.Mem2Mem = DMA_M2M_DISABLE;
    DMA_Init(DRI_UART_RX_DMA_CHANNEL, &st_dma_init);
}

static void Dri_UART_ConfigureInterrupt(void)
{
    NVIC_InitType st_nvic_init;

    NVIC_DisableIRQ(DMA_Channel1_2_3_4_IRQn);
    st_nvic_init.NVIC_IRQChannel = DMA_Channel1_2_3_4_IRQn;
    st_nvic_init.NVIC_IRQChannelPriority = 3U;
    st_nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&st_nvic_init);

    NVIC_DisableIRQ(DRI_UART_IRQ_CHANNEL);
    st_nvic_init.NVIC_IRQChannel = DRI_UART_IRQ_CHANNEL;
    st_nvic_init.NVIC_IRQChannelPriority = 3U;
    st_nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&st_nvic_init);
}

static void Dri_UART_ConfigurePeripheral(void)
{
    USART_InitType st_uart_init;

    st_uart_init.BaudRate = DRI_UART_BAUDRATE;
    st_uart_init.WordLength = USART_WL_8B;
    st_uart_init.StopBits = USART_STPB_1;
    st_uart_init.Parity = USART_PE_NO;
    st_uart_init.HardwareFlowControl = USART_HFCTRL_NONE;
    st_uart_init.Mode = USART_MODE_RX | USART_MODE_TX;

    USART_Init(DRI_UART_INDEX, &st_uart_init);
    USART_ConfigInt(DRI_UART_INDEX, USART_INT_IDLEF, ENABLE);
    USART_EnableDMA(DRI_UART_INDEX, USART_DMAREQ_RX | USART_DMAREQ_TX, ENABLE);
    DMA_EnableChannel(DRI_UART_RX_DMA_CHANNEL, ENABLE);
    USART_Enable(DRI_UART_INDEX, ENABLE);
}

static void Dri_UART_NotifyRx(const uint8_t *p_data, uint16_t length)
{
    uint16_t next_write_index;

    while (length > 0U)
    {
        next_write_index = (uint16_t)((s_dri_uart_rx_write_index + 1U) % DRI_UART_RX_BUFFER_SIZE);
        if (next_write_index == s_dri_uart_rx_read_index)
        {
            break;
        }

        s_dri_uart_rx_buffer[s_dri_uart_rx_write_index] = *p_data;
        s_dri_uart_rx_write_index = next_write_index;
        p_data++;
        length--;
    }

    if (s_dri_uart_rx_callback != NULL)
    {
        uint16_t callback_length;

        if (s_dri_uart_rx_write_index >= s_dri_uart_rx_read_index)
        {
            callback_length = (uint16_t)(s_dri_uart_rx_write_index - s_dri_uart_rx_read_index);
        }
        else
        {
            callback_length = (uint16_t)(DRI_UART_RX_BUFFER_SIZE - s_dri_uart_rx_read_index);
        }
        s_dri_uart_rx_callback(&s_dri_uart_rx_buffer[s_dri_uart_rx_read_index], callback_length);
    }
}

static void Dri_UART_CaptureRx(void)
{
    uint16_t rx_position;
    uint16_t segment_length;

    rx_position = (uint16_t)(DRI_UART_RX_DMA_SIZE - DMA_GetCurrDataCounter(DRI_UART_RX_DMA_CHANNEL));
    if (rx_position == s_dri_uart_rx_old_position)
    {
        return;
    }

    if (rx_position > s_dri_uart_rx_old_position)
    {
        segment_length = (uint16_t)(rx_position - s_dri_uart_rx_old_position);
        Dri_UART_NotifyRx(&s_dri_uart_rx_dma_buffer[s_dri_uart_rx_old_position], segment_length);
    }
    else
    {
        segment_length = (uint16_t)(DRI_UART_RX_DMA_SIZE - s_dri_uart_rx_old_position);
        Dri_UART_NotifyRx(&s_dri_uart_rx_dma_buffer[s_dri_uart_rx_old_position], segment_length);
        if (rx_position > 0U)
        {
            Dri_UART_NotifyRx(&s_dri_uart_rx_dma_buffer[0], rx_position);
        }
    }

    s_dri_uart_rx_old_position = rx_position;
}

static void Dri_UART_StartTx(void)
{
    DMA_InitType st_dma_init;
    uint16_t tx_length;

    if (s_dri_uart_tx_busy || (s_dri_uart_tx_read_index == s_dri_uart_tx_write_index))
    {
        return;
    }

    if (s_dri_uart_tx_read_index < s_dri_uart_tx_write_index)
    {
        tx_length = (uint16_t)(s_dri_uart_tx_write_index - s_dri_uart_tx_read_index);
    }
    else
    {
        tx_length = (uint16_t)(DRI_UART_TX_BUFFER_SIZE - s_dri_uart_tx_read_index);
    }

    st_dma_init.PeriphAddr = DRI_UART_DATA_ADDRESS;
    st_dma_init.MemAddr = (uint32_t)&s_dri_uart_tx_buffer[s_dri_uart_tx_read_index];
    st_dma_init.Direction = DMA_DIR_PERIPH_DST;
    st_dma_init.BufSize = tx_length;
    st_dma_init.PeriphInc = DMA_PERIPH_INC_DISABLE;
    st_dma_init.DMA_MemoryInc = DMA_MEM_INC_ENABLE;
    st_dma_init.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    st_dma_init.MemDataSize = DMA_MemoryDataSize_Byte;
    st_dma_init.CircularMode = DMA_MODE_NORMAL;
    st_dma_init.Priority = DMA_PRIORITY_VERY_HIGH;
    st_dma_init.Mem2Mem = DMA_M2M_DISABLE;

    s_dri_uart_tx_dma_length = tx_length;
    s_dri_uart_tx_busy = true;
    DMA_DeInit(DRI_UART_TX_DMA_CHANNEL);
    DMA_Init(DRI_UART_TX_DMA_CHANNEL, &st_dma_init);
    DMA_EnableChannel(DRI_UART_TX_DMA_CHANNEL, ENABLE);
}

void Dri_UART_Init(void)
{
    s_dri_uart_rx_old_position = 0U;
    s_dri_uart_rx_write_index = 0U;
    s_dri_uart_rx_read_index = 0U;
    s_dri_uart_tx_write_index = 0U;
    s_dri_uart_tx_read_index = 0U;
    s_dri_uart_tx_dma_length = 0U;
    s_dri_uart_tx_busy = false;

    Dri_UART_ConfigureClock();
    Dri_UART_ConfigureGpio();
    Dri_UART_ConfigureDma();
    Dri_UART_ConfigureInterrupt();
    Dri_UART_ConfigurePeripheral();
}

void Dri_UART_Enable(FunctionalState cmd)
{
    if (cmd == ENABLE)
    {
        ns_sleep_lock_acquire();
        Dri_UART_Init();
    }
    else
    {
        GPIO_InitType st_gpio_init;

        USART_Enable(DRI_UART_INDEX, DISABLE);
        DMA_EnableChannel(DRI_UART_RX_DMA_CHANNEL, DISABLE);
        DMA_EnableChannel(DRI_UART_TX_DMA_CHANNEL, DISABLE);
        GPIO_InitStruct(&st_gpio_init);
        st_gpio_init.Pin = DRI_UART_TX_PIN | DRI_UART_RX_PIN;
        st_gpio_init.GPIO_Mode = GPIO_MODE_ANALOG;
        st_gpio_init.GPIO_Alternate = GPIO_AF0;
        GPIO_InitPeripheral(DRI_UART_GPIO_PORT, &st_gpio_init);
        ns_sleep_lock_release();
    }
}

void Dri_UART_SetRxCallback(Dri_UART_RxCallback p_rx_callback)
{
    s_dri_uart_rx_callback = p_rx_callback;
}

uint16_t Dri_UART_Read(uint8_t *p_data, uint16_t length)
{
    uint16_t read_length = 0U;

    if (p_data == NULL)
    {
        return 0U;
    }

    while ((read_length < length) && (s_dri_uart_rx_read_index != s_dri_uart_rx_write_index))
    {
        p_data[read_length] = s_dri_uart_rx_buffer[s_dri_uart_rx_read_index];
        s_dri_uart_rx_read_index = (uint16_t)((s_dri_uart_rx_read_index + 1U) % DRI_UART_RX_BUFFER_SIZE);
        read_length++;
    }

    return read_length;
}

uint16_t Dri_UART_Send(const uint8_t *p_data, uint16_t length)
{
    uint16_t accepted_length = 0U;
    uint16_t next_write_index;

    if (p_data == NULL)
    {
        return 0U;
    }

    while (accepted_length < length)
    {
        next_write_index = (uint16_t)((s_dri_uart_tx_write_index + 1U) % DRI_UART_TX_BUFFER_SIZE);
        if (next_write_index == s_dri_uart_tx_read_index)
        {
            break;
        }

        s_dri_uart_tx_buffer[s_dri_uart_tx_write_index] = p_data[accepted_length];
        s_dri_uart_tx_write_index = next_write_index;
        accepted_length++;
    }

    Dri_UART_StartTx();
    return accepted_length;
}

void DMA_Channel1_2_3_4_IRQHandler(void)
{
    if (DMA_GetFlagStatus(DRI_UART_TX_DMA_FLAG, DMA) != RESET)
    {
        DMA_ClearFlag(DRI_UART_TX_DMA_FLAG, DMA);
        if (s_dri_uart_tx_busy)
        {
            s_dri_uart_tx_read_index =
                (uint16_t)((s_dri_uart_tx_read_index + s_dri_uart_tx_dma_length) % DRI_UART_TX_BUFFER_SIZE);
            s_dri_uart_tx_busy = false;
            Dri_UART_StartTx();
        }
    }

    if (DMA_GetFlagStatus(DRI_UART_RX_DMA_FLAG, DMA) != RESET)
    {
        DMA_ClearFlag(DRI_UART_RX_DMA_FLAG, DMA);
        Dri_UART_CaptureRx();
    }

    if (DMA_GetFlagStatus(DRI_UART_RX_DMA_HALF_FLAG, DMA) != RESET)
    {
        DMA_ClearFlag(DRI_UART_RX_DMA_HALF_FLAG, DMA);
        Dri_UART_CaptureRx();
    }
}

void USART2_IRQHandler(void)
{
    if (USART_GetFlagStatus(DRI_UART_INDEX, USART_FLAG_IDLEF) != RESET)
    {
        (void)DRI_UART_INDEX->DAT;
        Dri_UART_CaptureRx();
    }
}




