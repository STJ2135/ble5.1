#ifndef DRI_SOFT_I2C_H
#define DRI_SOFT_I2C_H

#include <stdbool.h>
#include <stdint.h>

void Dri_SoftI2C_Init(void);
void Dri_SoftI2C_Start(void);
void Dri_SoftI2C_Stop(void);
bool Dri_SoftI2C_IsAckReceived(void);
void Dri_SoftI2C_WriteByte(uint8_t data);
uint8_t Dri_SoftI2C_ReadByte(bool acknowledge);

#endif
