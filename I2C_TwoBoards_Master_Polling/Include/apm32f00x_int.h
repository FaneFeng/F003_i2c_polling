/*!
 * @file        apm32f00x_int.h
 *
 * @brief       This file contains the headers of the interrupt handlers   
 *
 * @version     V1.0.0
 *
 * @date        2019-11-4
 *
 */
#ifndef APM32F00X_INT_H
#define APM32F00X_INT_H

#include "apm32f00x.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
#endif

