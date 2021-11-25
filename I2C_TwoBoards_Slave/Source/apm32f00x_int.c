/*!
 * @file        apm32f00x_int.c
 *
 * @brief       Main Interrupt Service Routines        
 *
 * @version     V1.0.0
 *
 * @date        2019-11-4
 *
 */

#include "apm32f00x_int.h"
#include "main.h"

 /*!
  * @brief       This function handles NMI exception
  *
  * @param       None
  *
  * @retval      None
  *
  * @note       
  */
 void NMI_Handler(void)
 {
 }

/*!
 * @brief       This function handles Hard Fault exception
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
void HardFault_Handler(void)
{

}

/*!
 * @brief       This function handles SVCall exception
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
void SVC_Handler(void)
{
}

/*!
 * @brief       This function handles PendSV_Handler exception
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
void PendSV_Handler(void)
{
}

/*!
 * @brief       This function handles SysTick exception
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
void SysTick_Handler(void)
{

}

/*!
 * @brief       This function handles I2C exception
 *
 * @param       None
 *
 * @retval      None
 *
 * @note       
 */
void IIC_IRQHandler(void)
{
    I2CIsr();
}
