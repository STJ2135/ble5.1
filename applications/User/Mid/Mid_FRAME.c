#include <string.h>
#include "Mid_FRAME.h"
#include "Com_CRC.h"

#define MID_FRAME_INPUT_BUFFER_SIZE 1024U

typedef enum
{
    MID_FRAME_RX_STATE_HEADER_1 = 0,
    MID_FRAME_RX_STATE_HEADER_2,
    MID_FRAME_RX_STATE_FUNCTION_HIGH,
    MID_FRAME_RX_STATE_FUNCTION_LOW,
    MID_FRAME_RX_STATE_LENGTH_HIGH,
    MID_FRAME_RX_STATE_LENGTH_LOW,
    MID_FRAME_RX_STATE_DATA,
    MID_FRAME_RX_STATE_CRC_1,
    MID_FRAME_RX_STATE_CRC_2,
    MID_FRAME_RX_STATE_CRC_3,
    MID_FRAME_RX_STATE_CRC_4,
    MID_FRAME_RX_STATE_END_1,
    MID_FRAME_RX_STATE_END_2
} Mid_FRAME_RxStateType;

static uint8_t s_input_buffer[MID_FRAME_INPUT_BUFFER_SIZE];
static volatile uint16_t s_input_write_index;
static volatile uint16_t s_input_read_index;
static Mid_FRAME_RxStateType s_rx_state;
static Mid_FRAME_Message_Struct s_rx_message;
static uint16_t s_rx_data_index;
static uint32_t s_rx_crc_value;
static uint8_t s_rx_crc_data[MID_FRAME_MAX_FRAME_SIZE];
static uint16_t s_rx_crc_length;

static void Mid_FRAME_ResetRx(void)
{
    s_rx_state = MID_FRAME_RX_STATE_HEADER_1;
    s_rx_message.function = 0U;
    s_rx_message.length = 0U;
    s_rx_data_index = 0U;
    s_rx_crc_value = 0U;
    s_rx_crc_length = 0U;
}

void Mid_FRAME_Init(void)
{
    s_input_write_index = 0U;
    s_input_read_index = 0U;
    Mid_FRAME_ResetRx();
}

uint16_t Mid_FRAME_AppendToRxBuffer(const uint8_t *p_data, uint16_t length)
{
    uint16_t accepted_length = 0U;
    uint16_t next_write_index;

    if (p_data == NULL)
    {
        return 0U;
    }

    while (accepted_length < length)
    {
        next_write_index = (uint16_t)((s_input_write_index + 1U) % MID_FRAME_INPUT_BUFFER_SIZE);
        if (next_write_index == s_input_read_index)
        {
            break;
        }

        s_input_buffer[s_input_write_index] = p_data[accepted_length];
        s_input_write_index = next_write_index;
        accepted_length++;
    }

    return accepted_length;
}

static void Mid_FRAME_AppendCrcData(uint8_t data)
{
    if (s_rx_crc_length < MID_FRAME_MAX_FRAME_SIZE)
    {
        s_rx_crc_data[s_rx_crc_length] = data;
        s_rx_crc_length++;
    }
}

static bool Mid_FRAME_IsFrameComplete(uint8_t data, Mid_FRAME_Message_Struct *p_st_message)
{
    bool is_message_complete = false;
    uint32_t calculated_crc;

    switch (s_rx_state)
    {
        case MID_FRAME_RX_STATE_HEADER_1:
            if (data == MID_FRAME_HEADER_1)
            {
                Mid_FRAME_ResetRx();
                Mid_FRAME_AppendCrcData(data);
                s_rx_state = MID_FRAME_RX_STATE_HEADER_2;
            }
            break;

        case MID_FRAME_RX_STATE_HEADER_2:
            if (data == MID_FRAME_HEADER_2)
            {
                Mid_FRAME_AppendCrcData(data);
                s_rx_state = MID_FRAME_RX_STATE_FUNCTION_HIGH;
            }
            else
            {
                Mid_FRAME_ResetRx();
            }
            break;

        case MID_FRAME_RX_STATE_FUNCTION_HIGH:
            s_rx_message.function = (uint16_t)((uint16_t)data << 8U);
            Mid_FRAME_AppendCrcData(data);
            s_rx_state = MID_FRAME_RX_STATE_FUNCTION_LOW;
            break;

        case MID_FRAME_RX_STATE_FUNCTION_LOW:
            s_rx_message.function |= data;
            Mid_FRAME_AppendCrcData(data);
            s_rx_state = MID_FRAME_RX_STATE_LENGTH_HIGH;
            break;

        case MID_FRAME_RX_STATE_LENGTH_HIGH:
            s_rx_message.length = (uint16_t)((uint16_t)data << 8U);
            Mid_FRAME_AppendCrcData(data);
            s_rx_state = MID_FRAME_RX_STATE_LENGTH_LOW;
            break;

        case MID_FRAME_RX_STATE_LENGTH_LOW:
            s_rx_message.length |= data;
            Mid_FRAME_AppendCrcData(data);
            if ((s_rx_message.length == 0U) || (s_rx_message.length > MID_FRAME_MAX_PAYLOAD))
            {
                Mid_FRAME_ResetRx();
                break;
            }

            s_rx_data_index = 0U;
            s_rx_state = MID_FRAME_RX_STATE_DATA;
            break;

        case MID_FRAME_RX_STATE_DATA:
            s_rx_message.data[s_rx_data_index] = data;
            s_rx_data_index++;
            Mid_FRAME_AppendCrcData(data);
            if (s_rx_data_index >= s_rx_message.length)
            {
                s_rx_state = MID_FRAME_RX_STATE_CRC_1;
            }
            break;

        case MID_FRAME_RX_STATE_CRC_1:
            s_rx_crc_value = (uint32_t)data << 24U;
            s_rx_state = MID_FRAME_RX_STATE_CRC_2;
            break;

        case MID_FRAME_RX_STATE_CRC_2:
            s_rx_crc_value |= (uint32_t)data << 16U;
            s_rx_state = MID_FRAME_RX_STATE_CRC_3;
            break;

        case MID_FRAME_RX_STATE_CRC_3:
            s_rx_crc_value |= (uint32_t)data << 8U;
            s_rx_state = MID_FRAME_RX_STATE_CRC_4;
            break;

        case MID_FRAME_RX_STATE_CRC_4:
            s_rx_crc_value |= data;
            calculated_crc = Com_CRC_Calculate32(s_rx_crc_data, s_rx_crc_length);
            if (calculated_crc != s_rx_crc_value)
            {
                Mid_FRAME_ResetRx();
                break;
            }

            s_rx_state = MID_FRAME_RX_STATE_END_1;
            break;

        case MID_FRAME_RX_STATE_END_1:
            if (data != MID_FRAME_END_1)
            {
                Mid_FRAME_ResetRx();
                break;
            }

            s_rx_state = MID_FRAME_RX_STATE_END_2;
            break;

        case MID_FRAME_RX_STATE_END_2:
            if (data == MID_FRAME_END_2)
            {
                if (p_st_message != NULL)
                {
                    memcpy(p_st_message, &s_rx_message, sizeof(Mid_FRAME_Message_Struct));
                    is_message_complete = true;
                }
            }

            Mid_FRAME_ResetRx();
            break;

        default:
            Mid_FRAME_ResetRx();
            break;
    }

    return is_message_complete;
}

bool Mid_FRAME_IsMessageReady(Mid_FRAME_Message_Struct *p_st_message)
{
    uint8_t data;

    if (p_st_message == NULL)
    {
        return false;
    }

    while (s_input_read_index != s_input_write_index)
    {
        data = s_input_buffer[s_input_read_index];
        s_input_read_index = (uint16_t)((s_input_read_index + 1U) % MID_FRAME_INPUT_BUFFER_SIZE);

        if (Mid_FRAME_IsFrameComplete(data, p_st_message))
        {
            return true;
        }
    }

    return false;
}

uint32_t Mid_FRAME_Build(uint8_t *p_buffer,
                         uint32_t capacity,
                         const uint8_t *p_data,
                         uint16_t data_length,
                         uint16_t function)
{
    uint32_t frame_length;
    uint32_t crc_value;
    uint32_t index;

    if ((p_buffer == NULL) || (data_length > MID_FRAME_MAX_PAYLOAD))
    {
        return 0U;
    }

    if ((data_length > 0U) && (p_data == NULL))
    {
        return 0U;
    }

    frame_length = 2U + 2U + 2U + data_length + 4U + 2U;
    if ((frame_length > capacity) || (frame_length > MID_FRAME_MAX_FRAME_SIZE))
    {
        return 0U;
    }

    index = 0U;
    p_buffer[index++] = MID_FRAME_HEADER_1;
    p_buffer[index++] = MID_FRAME_HEADER_2;
    p_buffer[index++] = (uint8_t)(function >> 8U);
    p_buffer[index++] = (uint8_t)(function & 0xFFU);
    p_buffer[index++] = (uint8_t)(data_length >> 8U);
    p_buffer[index++] = (uint8_t)(data_length & 0xFFU);

    if (data_length > 0U)
    {
        memcpy(&p_buffer[index], p_data, data_length);
        index += data_length;
    }

    crc_value = Com_CRC_Calculate32(p_buffer, index);
    p_buffer[index++] = (uint8_t)(crc_value >> 24U);
    p_buffer[index++] = (uint8_t)(crc_value >> 16U);
    p_buffer[index++] = (uint8_t)(crc_value >> 8U);
    p_buffer[index++] = (uint8_t)crc_value;
    p_buffer[index++] = MID_FRAME_END_1;
    p_buffer[index++] = MID_FRAME_END_2;

    return index;
}




