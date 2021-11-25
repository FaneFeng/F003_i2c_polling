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

/** Master address  */
#define MASTER_ADDR         (0XA0)
/** Slave address  */
#define SLAVE_ADDR          (0XB0)

#define DATA_BUF_SIZE       (32)
uint8_t dataBufPt = 0;
uint8_t txDataBuf[DATA_BUF_SIZE] = {0};
uint8_t rxDataBuf[DATA_BUF_SIZE] = {0};

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
    KeyInit();
    I2CInit();

    while(1)
    {
        if(KeyPressed())
        {
            /** Data buffer init  */
            DataBufInit();
            
            dataBufPt = 0;
            /** Send data to slave  */
            i2cDirection = I2C_DIRECTION_TX;
            /** Wait bus is idle  */
            while(I2C_ReadStatusFlag(I2C_FLAG_BUSBUSY) == SET);
            /** Generate a start condition */
            I2C_EnableGenerateStart();
            /** Wait end of transmission */
            while(dataBufPt < DATA_BUF_SIZE);
            
            Delay(0x1000);
            
            dataBufPt = 0;
            /** ACK when current byte being received  */
            I2C_ConfigAcknowledge(I2C_ACK_CURRENT);
            /** Receive data from slave  */
            i2cDirection = I2C_DIRECTION_RX;
            /** Wait bus is idle  */
            while(I2C_ReadStatusFlag(I2C_FLAG_BUSBUSY) == SET);
            /** Generate a start condition */
            I2C_EnableGenerateStart();
            /** Wait end of reception */
            while(dataBufPt < DATA_BUF_SIZE);

            /** Verify data */
            if(!BufferCompare(txDataBuf, rxDataBuf, DATA_BUF_SIZE))
            {
                while(1);
            }
        }
    }
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
    i2cConfig.outputClkFreqHz = 100000;
    I2C_Config(&i2cConfig);
    
    NVIC_EnableIRQRequest(IIC_IRQn, 0X01);
    
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
void DataBufInit(void)
{
    uint8_t i;
    
    for(i = 0; i < DATA_BUF_SIZE; i++)
    {
        txDataBuf[i] = 0xa0 + i;
        rxDataBuf[i] = 0;
    }
}

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
BOOL BufferCompare(uint8_t *buf1, uint8_t *buf2, uint8_t size)
{
    uint8_t i;
    
    for(i = 0; i < size; i++)
    {
        if(buf1[i] != buf2[i])
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

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
