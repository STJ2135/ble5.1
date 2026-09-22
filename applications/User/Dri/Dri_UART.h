#ifndef DRI_UART_H
#define DRI_UART_H

#include <stdint.h>
#include "n32wb03x.h"

typedef void (*Dri_UART_RxCallback)(const uint8_t *p_data, uint16_t length);

void Dri_UART_Init(void);
void Dri_UART_Enable(FunctionalState cmd);
void Dri_UART_SetRxCallback(Dri_UART_RxCallback p_rx_callback);
uint16_t Dri_UART_Read(uint8_t *p_data, uint16_t length);
uint16_t Dri_UART_Send(const uint8_t *p_data, uint16_t length);

#endif
