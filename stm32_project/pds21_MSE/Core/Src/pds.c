/*
 * pds.c
 *
 *  Created on: Aug 22, 2023
 *      Author: lab
 */

#include "pds.h"
#include "adc.h"
#include "dac.h"

#define DO  261.63
#define RE  293.66
#define MI  329.63
#define FA  349.23
#define SOL 392.00
#define LA  440.00
#define SI  493.88

enum{
	CH0 = 0,
	CH1 = 1,
	CH2 = 2,
}adc_e;

static uint32_t ClockSpeed = CLOCK_SPEED;

void ADC_SelectCH0(void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

uint16_t ADC_Read(uint16_t adc)
{
	uint16_t adc_value;
	switch(adc){
		case CH0:
			ADC_SelectCH0();
		break;
		default:
			ADC_SelectCH0();
		break;
	}

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	adc_value = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return adc_value;
}


bool DBG_CyclesCounterInit( uint32_t clockSpeed )
{
   //Asigna  a la variable local ClockSpeed el valor recibido como argumento.
   ClockSpeed = clockSpeed;
   //Iniciar el contador de ciclos de clock.
   DWT->CTRL  |= 1; // *DWT_CTRL  |= 1;
   return 1;
}

// Blocking Write 1 byte to TX FIFO
void uartWriteByte( UART_HandleTypeDef *huart, const uint8_t value)
{
	uint8_t val = value;
	HAL_UART_Transmit(huart, &val, 1, HAL_MAX_DELAY);
}


// Blocking, Send a Byte Array
void uartWriteByteArray( UART_HandleTypeDef *huart, const uint8_t* byteArray, uint32_t byteArrayLen )
{
   uint32_t i = 0;
   for( i=0; i<byteArrayLen; i++ ) {
	   uartWriteByte(huart, byteArray[i]);
   }
}

void gpioToggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

void trigger(int16_t threshold)
{
   while((ADC_Read(CH0)-512)>threshold)
      ;
   while((ADC_Read(CH0)-512)<threshold)
      ;
   return;
}

uint16_t DOm(float t)
{
   return 512*0.4*arm_sin_f32 (2*PI*t*DO)+\
          512*0.2*arm_sin_f32 (2*PI*t*MI)+\
          512*0.3*arm_sin_f32 (2*PI*t*SOL)+512;
}


void DAC_Write(DAC_HandleTypeDef *handle, uint16_t value)
{
	// DAC is 12 bit so maximum value should be 4096
	if( value > 4096 )
	{
		value = 4096; // Saturation
	}

	HAL_DAC_SetValue(handle, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, value);
}


