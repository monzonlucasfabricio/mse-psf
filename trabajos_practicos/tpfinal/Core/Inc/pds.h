/*
 * pds.h
 *
 *  Created on: Aug 22, 2023
 *      Author: lab
 */

#ifndef INC_PDS_H_
#define INC_PDS_H_

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"
#include "usart.h"
#include "stm32f429xx.h"
#include "arm_math.h"

#define CLOCK_SPEED 168000000
// Read Cycles Counter
#define DBG_CyclesCounterRead()		DWT->CYCCNT
// Reset Cycles Counter
#define DBG_CyclesCounterReset()    (DWT->CYCCNT = 0)

/* Function prototypes */
void trigger(int16_t threshold);
uint16_t ADC_Read(uint16_t adc);
bool DBG_CyclesCounterInit( uint32_t clockSpeed );
void uartWriteByteArray( UART_HandleTypeDef *huart, char* byteArray, uint32_t byteArrayLen );
void gpioToggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void DAC_Write(DAC_HandleTypeDef *handle, uint16_t value);
uint16_t DOm(float t);
void init_cfft_instance(arm_cfft_instance_q15* CS,int length);

#endif /* INC_PDS_H_ */
