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
#include "fir_chapu_noise.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define N_MUESTRAS  	411
#define FREQ_MUESTREO	8000
#define BITS 12
#define CUTFREC 1600

/* Header added to the stream */
struct header_struct {
  char     pre[8];
  uint32_t id;
  uint16_t N;
  uint16_t fs ;
  uint16_t cutFrec ;
  uint16_t M ;
  char     pos[4];
}__attribute__ ((packed));

struct header_struct header={"*header*",0,N_MUESTRAS,FREQ_MUESTREO,CUTFREC,h_LENGTH,"end*"};

// M + N - 1 = 512 -> N = 512 + 1 - 101 = 412
// Filter size = 101
// Signal samples = 412

uint32_t tick   = 0   ;
uint16_t tone   = 440 ;
uint16_t B      = 4000;
uint16_t sweept = 10;

q15_t fftAbs  		[ ( N_MUESTRAS+h_LENGTH-1 )*1 ];
q15_t fftInOut		[ ( N_MUESTRAS+h_LENGTH-1 )*2 ];
int16_t adc			[ ( N_MUESTRAS+h_LENGTH-1 )*1 ];

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

	DBG_CyclesCounterInit(CLOCK_SPEED); // Enable the cycle counter
	
	uint16_t sample = 0;

	arm_cfft_instance_q15 CS;

	memset(fftAbs, 0, header.N+h_LENGTH-1);
	memset(fftInOut, 0, (header.N+h_LENGTH-1)*2);
	memset(adc, 0, header.N+h_LENGTH-1);

	float t = 0;

	while (1)
	{
		/* Reset cycle counter to 0 */
		DBG_CyclesCounterReset();

		/* Get the ADC sample */
		adc[sample] = ((((int16_t )ADC_Read(0)-2048))>>(12-BITS))<<(6+(12-BITS)); //10 bits
		fftInOut[sample*2]   = adc[sample];
		fftInOut[sample*2+1] = 0;

		/* Increment the sample counter and check if we are in the last sample */
		if ( ++sample==header.N ) 
		{
			/* Blinks at fs/N frequency */
			gpioToggle (GPIOB,LD1_Pin);

			//------------TRANSFORMADA------------------
         	init_cfft_instance ( &CS,  (header.N+h_LENGTH-1));
         	arm_cfft_q15       ( &CS , fftInOut , 0 , 1     ) ;

			//------------MAGNITUD------------------
        	arm_cmplx_mag_squared_q15 ( fftInOut ,fftAbs ,(header.N+h_LENGTH-1 ));

			//------------FILTRADO MULTIPLICANDO ESPECTROS------------------
         	arm_mult_q15 ( fftAbs,HAbs ,fftAbs ,(header.N+h_LENGTH-1));

			/* Increment id */
			header.id++;

			/* Send the header in an Array */
			uartWriteByteArray (&huart2, (uint8_t*)&header, sizeof(struct header_struct ));

			for (sample=0; sample<(header.N+h_LENGTH-1);sample++ )
			{
				uartWriteByteArray ( &huart2 ,(uint8_t* )&adc[sample]      ,sizeof(adc[0]) );     // envia el sample ANTERIOR
				uartWriteByteArray ( &huart2 ,(uint8_t* )&fftAbs[sample*1] ,sizeof(fftInOut[0])); // envia la fft del sample ANTERIO
			}

			/* Reset the samples */
			sample = 0;
		}
		
		/* Blinks at fs/2 frequency */
		gpioToggle (GPIOB,LD3_Pin);


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
