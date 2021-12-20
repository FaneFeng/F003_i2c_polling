#ifndef __EEPROM_H
#define __EEPROM_H

#include "apm32f00x_i2c.h"

void EEPROM_Write(uint16_t addr, uint8_t *pData, uint32_t size);
void EEPROM_Read(uint16_t addr, uint8_t *rxbuffer, uint32_t size);

#endif
