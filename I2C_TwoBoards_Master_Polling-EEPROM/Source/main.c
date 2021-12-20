/*!
 * @file        main.c
 *
 * @brief       Main program body        
 *
 * @version     V1.0.0
 *
 * @date        2019-11-4
 *
 */

#include "main.h"
#include "Board.h"
#include "apm32f00x_gpio.h"
#include "apm32f00x_misc.h"
#include "apm32f00x_eint.h"
#include "apm32f00x_i2c.h"
#include "eeprom.h"

/** Master address  */
#define MASTER_ADDR         (0XA0)
/** Slave address  */
#define SLAVE_ADDR          (0XB0)



#define DATA_BUF_SIZE       (32)
uint8_t dataBufPt = 0;
uint8_t rxDataBuf[DATA_BUF_SIZE] = {0};
uint8_t txDataBuf[DATA_BUF_SIZE] = {0};

/** Direction. RX or TX  */
I2C_DIRECTION_T i2cDirection = I2C_DIRECTION_TX;

/* Key Init */
void KeyInit(void);
/* I2C Init */
void I2CInit(void);
/* delay */
void Delay(uint32_t count);
/* Key process */
BOOL KeyPressed(void);
/* Data buffer init */
void DataBufInit(void);
/* Compares two buffers */
BOOL BufferCompare(uint8_t *buf1, uint8_t *buf2, uint8_t size);

uint32_t I2C_Master_Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
uint32_t I2C_Master_Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
uint32_t I2C_Master_Tranmit_NOStop(uint16_t DevAddress, uint8_t *pData, uint16_t Size);

/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
int main(void)
{
	uint16_t i;
	
    KeyInit();
    I2CInit();

    uint8_t trans_data[64];
	uint8_t rxbuffer[64];
	
	for(i=0;i<64;i++){
		trans_data[i] = i+2;
	}

    while(1)
    {
        if(KeyPressed())
        {
            EEPROM_Write(0x0100, trans_data, sizeof(trans_data));
			
			EEPROM_Read(0x0100, rxbuffer, sizeof(rxbuffer)) ;
        }
    }
}

uint32_t I2C_Master_Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
    uint16_t transmit_size = 0;
    uint16_t test_num = 0;

    while(I2C_ReadStatusFlag(I2C_FLAG_BUSBUSY) == SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;
    }
    test_num = 0;

    I2C_EnableGenerateStart();//Start，MMF位会被硬件置1
    while(I2C_ReadStatusFlag(I2C_FLAG_START) != SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;   
    }
    test_num = 0;

    I2C_TxAddress7Bit(DevAddress, I2C_DIRECTION_TX);
    while(I2C_ReadStatusFlag(I2C_FLAG_ADDR) != SET){
        if(test_num>0xffff){
            return FALSE;
        }
        test_num++;   
    }
    (void)I2C->STS3;//清空ADDR标志位，硬性要求流程；
    test_num = 0;

    while(transmit_size < Size){
        //* 当收到ACK时，TXBE会被硬件置位；往DATA数据寄存器写数据清除TXBE标志位 */
        I2C_TxData(pData[transmit_size++]);
        while((I2C_ReadStatusFlag(I2C_FLAG_TXBE)!=SET)&&(I2C_ReadStatusFlag(I2C_FLAG_RWMF)==SET)){
            if(test_num>0x7fff){
                return FALSE;
            }    
            test_num++;
        }
        test_num = 0;
    }
    I2C_EnableGenerateStop();//产生STOP信号后，硬件会清除MMF位，设备从主模式切换成从模式；

    return TRUE;
}

uint32_t I2C_Master_Tranmit_NOStop(uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
    uint16_t transmit_size = 0;
    uint16_t test_num = 0;

    while(I2C_ReadStatusFlag(I2C_FLAG_BUSBUSY) == SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;
    }
    test_num = 0;

    I2C_EnableGenerateStart();//Start
    while(I2C_ReadStatusFlag(I2C_FLAG_START) != SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;   
    }
    test_num = 0;

    I2C_TxAddress7Bit(DevAddress, I2C_DIRECTION_TX);
    while(I2C_ReadStatusFlag(I2C_FLAG_ADDR) != SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;   
    }
    (void)I2C->STS3;//清空ADDR标志位，硬性要求流程；
    test_num = 0;

    while(transmit_size < Size){
        I2C_TxData(pData[transmit_size++]);
        while((I2C_ReadStatusFlag(I2C_FLAG_TXBE)!=SET)&&(I2C_ReadStatusFlag(I2C_FLAG_RWMF)==SET)){
            if(test_num>0x7fff){
                return FALSE;
            }    
            test_num++;
        }
        test_num = 0;
    }

    return TRUE;
}

//具体要分为Size>2，Size=2，Size=1的情况——>由于数据寄存器和数据缓存寄存器的存在
uint32_t I2C_Master_Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
    uint16_t transmit_size = 0;
    uint16_t test_num = 0;

    //** 注：ACK配置仅跟接收模式有关，不管是从机模式接收，还是主机模式接收 */
    //** 针对二次或多次调用Receive函数的常见，需要在函数内重新初始化ACKEN=1 */
    I2C_ConfigAcknowledge(I2C_ACK_CURRENT);//此例程的I2C初始化函数已经配置过一次"ACK_CURRENT"

    while(I2C_ReadStatusFlag(I2C_FLAG_BUSBUSY) == SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;
    }
    test_num = 0;


    I2C_EnableGenerateStart();//Start
    while(I2C_ReadStatusFlag(I2C_FLAG_START) != SET){
        if(test_num>0x7fff){
            return FALSE;
        }
        test_num++;   
    }
    test_num = 0;

    if(Size == 0){
        return FALSE;
    }

    I2C_TxAddress7Bit(DevAddress, I2C_DIRECTION_RX);
    while(I2C_ReadStatusFlag(I2C_FLAG_ADDR) != SET){
    if(test_num>0x7fff){
        return FALSE;
    }
        test_num++;   
    }
    (void)I2C->STS3;//清空ADDR标志位，硬性要求流程；
    test_num = 0;
    
    if(Size >2){
        while(transmit_size < Size){
            while((I2C_ReadStatusFlag(I2C_FLAG_RXBNE) != SET)&&(I2C_ReadStatusFlag(I2C_FLAG_RWMF)==RESET)){
                if(test_num>0x7fff){
                    return FALSE;
                }
                test_num++; 
            }
            test_num = 0;

            if(transmit_size == (Size - 2)){
                I2C_ConfigAcknowledge(I2C_ACK_NONE);
                I2C_EnableGenerateStop();
            }
            pData[transmit_size++] = I2C_RxData();
        }

    }else if(Size == 1){
        I2C_ConfigAcknowledge(I2C_ACK_NONE);//ACKEN=0和STOP=1要在ADDR位清除后，马上配置
        I2C_EnableGenerateStop();

        while((I2C_ReadStatusFlag(I2C_FLAG_RXBNE) != SET)&&(I2C_ReadStatusFlag(I2C_FLAG_RWMF)==RESET)){
            if(test_num>0x7fff){
                return FALSE;
            }
            test_num++; 
        }
        pData[transmit_size] = I2C_RxData();

    }else if(Size == 2){
        //** BTC=1——In reception when a new byte is received (including ACK pulse) and DR has not been readyet (RXNE=1)*/
        //** 这套软件时序逻辑，不能使用BTC位做判别，否则结尾不能出现NAK+STOP，会变成ACK+STOP*/
        while((I2C_ReadStatusFlag(I2C_FLAG_RXBNE) != SET)&&(I2C_ReadStatusFlag(I2C_FLAG_RWMF)==RESET)){
            if(test_num>0x7fff){
                return FALSE;
            }
            test_num++; 
        }
        I2C_ConfigAcknowledge(I2C_ACK_NONE);//注意N=1/N=2情况中，ACKEN=0 + STOP=1的软件时序差异
        I2C_EnableGenerateStop();
        pData[transmit_size++] = I2C_RxData();
        pData[transmit_size] = I2C_RxData();
    }

    return TRUE;
}

/*!
 * @brief       Key Init   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void KeyInit(void)
{
    Board_KeyInit(BOARD_KEY1, DISABLE);
}

/*!
 * @brief       I2C Init   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void I2CInit(void)
{
    I2C_Config_T i2cConfig;
    
    /** ACK when current byte being received  */
    i2cConfig.ack = I2C_ACK_CURRENT;
    /** Set Address  */
    i2cConfig.addr = SLAVE_ADDR;
    /** 7-bit address */
    i2cConfig.addrMode = I2C_ADDR_7_BIT;
    /** duty cycle = 2 */
    i2cConfig.dutyCycle = I2C_DUTYCYCLE_2;
    /** Input clock frequency is 48MHZ */
    i2cConfig.inputClkFreqMhz = 48;
    /** Enable buffer and event interrupt */
    i2cConfig.interrupt = I2C_INT_BUFFER | I2C_INT_EVENT;
    /** Output clock frequency is 100KHZ */
    i2cConfig.outputClkFreqHz = 400000;
    I2C_Config(&i2cConfig);
    
//    NVIC_EnableIRQRequest(IIC_IRQn, 0X01);
    
    /** Enable I2C */
    I2C_Enable();    
}

/*!
 * @brief       Delay   
 *
 * @param       count:  delay count
 *
 * @retval      None
 *
 * @note
 */
void Delay(uint32_t count)
{
    volatile uint32_t delay = count;
    
    while(delay--);
}

/*!
 * @brief       I2C interrupt service routine   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */
void I2CIsr(void)
{
    /** Start condition generated */
    if(I2C_ReadStatusFlag(I2C_FLAG_START) == SET)
    {
        I2C_TxAddress7Bit(SLAVE_ADDR, i2cDirection);
    }
    
    /** Received address matched */
    if(I2C_ReadStatusFlag(I2C_FLAG_ADDR) == SET)
    {
        (void)I2C->STS3;
    }
    
    /** STS1_TXBEF = 1 and STS3_RWMF = 1 */
    if(I2C_ReadStatusFlag(I2C_FLAG_RWMF | I2C_FLAG_TXBE) == SET)
    {
        if(dataBufPt < DATA_BUF_SIZE)
        {
            I2C_TxData(txDataBuf[dataBufPt++]);
        }
        else
        {
            I2C_EnableGenerateStop();
        }        
    }
    /** STS1_RXBNEF = 1 and STS3_RWMF = 0 */
    if((I2C_ReadStatusFlag(I2C_FLAG_RXBNE) == SET) && (I2C_ReadStatusFlag(I2C_FLAG_RWMF) == RESET))
    {
        if(dataBufPt < DATA_BUF_SIZE)
        {
            if(dataBufPt == (DATA_BUF_SIZE - 2))
            {
                I2C_ConfigAcknowledge(I2C_ACK_NONE);
                I2C_EnableGenerateStop();
            }
            rxDataBuf[dataBufPt++] = I2C_RxData();
        }    
    }   
}

/*!
 * @brief       Data buffer init   
 *
 * @param       None
 *
 * @retval      None
 *
 * @note
 */


/*!
 * @brief       Compares two buffers   
 *
 * @param       buf1:    First buffer to be compared
 *
 * @param       buf1:    Second buffer to be compared
 *
 * @param       size:    Buffer size
 *
 * @retval      Return TRUE if buf1 = buf2. If not then return FALSE
 *
 * @note
 */


/*!
 * @brief       Check Key is pressed
 *
 * @param       None
 *
 * @retval      Return TRUE if key has been pressed.If not return FALSE
 *
 * @note       
 */
BOOL KeyPressed(void)
{
    BOOL ret = FALSE;
    
    if(Board_ReadKeyState(BOARD_KEY1) == BIT_RESET)
    {
        Delay(0x7fff);
        if(Board_ReadKeyState(BOARD_KEY1) == BIT_RESET)
        {
            while(Board_ReadKeyState(BOARD_KEY1) == BIT_RESET);
            
            ret = TRUE;
        }
    }
    
    return ret;
}
