#include "eeprom.h"

#define EEPROM_ADDR         (0xA0)

//移植代码，需要提供对应以下命名的函数API接口
extern uint32_t I2C_Master_Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
extern uint32_t I2C_Master_Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
extern uint32_t I2C_Master_Tranmit_NOStop(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
extern void Delay(uint32_t count);

void EEPROM_Write(uint16_t addr, uint8_t *pData, uint32_t size)
{
	uint8_t trans[size+2];
	uint16_t i;

	/* EEPROM Write Sequence*/
	
	trans[0] = addr>>8;
	trans[1] = addr;
	for(i=0;i<size;i++){
		trans[i+2] = pData[i];
	}
	
    I2C_Master_Transmit(EEPROM_ADDR, trans, (size+2));
	Delay(0x4fff); //等待EEPROM写入操作完成，5ms一个周期
                   //48MHz主频，软延时0x4fff ≈ 5ms
}

void EEPROM_Read(uint16_t addr, uint8_t *rxbuffer, uint32_t size)
{
	uint8_t temp[2];
	temp[0] = addr>>8;
	temp[1] = addr;
	I2C_Master_Transmit(EEPROM_ADDR, temp, 2); 
	I2C_Master_Receive(EEPROM_ADDR, rxbuffer, size);
}




