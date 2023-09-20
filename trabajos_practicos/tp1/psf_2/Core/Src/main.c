/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
#include "pds.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define N_MUESTRAS  	128
#define FREQ_MUESTREO	30000
#define BITS 10

/* Header added to the stream */
struct header_struct {
   	char     head[4];
   	uint32_t id;
   	uint16_t N;
   	uint16_t fs ;
	uint32_t maxIndex;
   	uint32_t minIndex;
   	q15_t    maxValue;
   	q15_t    minValue;
   	q15_t    rms;
   	char     tail[4];
} header={"head",0,N_MUESTRAS,FREQ_MUESTREO,0,0,0,0,0,"tail"};


uint32_t tick   = 0   ;
uint16_t tone   = 440 ;
uint16_t B      = 4000;
uint16_t sweept = 10;

void SystemClock_Config(void);

int main(void)
{
  /* System Initialization */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_DAC_Init();

  uint16_t sample = 0;
  DBG_CyclesCounterInit(CLOCK_SPEED); // Enable the cycle counter
  int16_t adc [N_MUESTRAS];
  uint16_t tone_value = 0;
  float t = 0;

  while (1)
  {
	/* Reset cycle counter to 0 */
	DBG_CyclesCounterReset();

	/* Get the ADC sample */
//	 adc[sample] = ((((int16_t )ADC_Read(0)-512))>>(10-BITS))<<(6+(10-BITS)); //10 bits

	/* Send the sample in an Array */
//     uartWriteByteArray(&huart2, (uint8_t* )&adc[sample], sizeof(adc[0]));

	t = tick/(float)header.fs;
	tick++;
	
	/* Calculate the the tone value. The nucleo board has a DAC that can work in 12 or 8 bits.*/
	tone_value = 2048*arm_sin_f32 (t*tone*2*PI)+2048;
    DAC_Write( &hdac, tone_value); 

	/* Increment the sample counter and check if we are in the last sample */
	if ( ++sample==header.N ) 
	{
		/* Send the max value */
//		DAC_Write( &hdac, 2048);

		/* Blinks at fs/N frequency */
		gpioToggle (GPIOB,LD1_Pin);

		/* Calculate max, min and rms */
		arm_max_q15 ( adc, header.N, &header.maxValue,&header.maxIndex );
		arm_min_q15 ( adc, header.N, &header.minValue,&header.minIndex );
		arm_rms_q15 ( adc, header.N, &header.rms                       );

		/* Increment id */
		header.id++;

		/* Send the header in an Array */
//		uartWriteByteArray (&huart2, (uint8_t*)&header, sizeof(header));

		/* Reset the samples */
		sample = 0;
	}
	/* Blinks at fs/2 frequency */
	gpioToggle (GPIOB,LD3_Pin);
	if (tick >= 4294967296) tick = 0;
	/* Wait until it completes the Cycles. 168.000.000/10.000 = 16.800 cycles */
	while(DBG_CyclesCounterRead() < CLOCK_SPEED/header.fs);
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
