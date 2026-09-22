#ifndef MID_FRAME_H
#define MID_FRAME_H

#include <stdbool.h>
#include <stdint.h>

#define MID_FRAME_HEADER_1          0x50U
#define MID_FRAME_HEADER_2          0x4CU
#define MID_FRAME_END_1             0xA5U
#define MID_FRAME_END_2             0x5AU
#define MID_FRAME_MAX_PAYLOAD       512U
#define MID_FRAME_MAX_FRAME_SIZE    (2U + 2U + 2U + MID_FRAME_MAX_PAYLOAD + 4U + 2U)

typedef struct
{
    uint16_t function;
    uint16_t length;
    uint8_t data[MID_FRAME_MAX_PAYLOAD];
} Mid_FRAME_Message_Struct;

void Mid_FRAME_Init(void);
uint16_t Mid_FRAME_AppendToRxBuffer(const uint8_t *p_data, uint16_t length);
bool Mid_FRAME_IsMessageReady(Mid_FRAME_Message_Struct *p_st_message);
uint32_t Mid_FRAME_Build(uint8_t *p_buffer,
                         uint32_t capacity,
                         const uint8_t *p_data,
                         uint16_t data_length,
                         uint16_t function);

#endif



