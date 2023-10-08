/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
static const uint8_t sinval[20]={
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
};

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define USER_Btn_EXTI_IRQn EXTI15_10_IRQn
#define LCD_CS_Pin GPIO_PIN_3
#define LCD_CS_GPIO_Port GPIOF
#define LCD_RST_Pin GPIO_PIN_5
#define LCD_RST_GPIO_Port GPIOF
#define BOTON3_Pin GPIO_PIN_6
#define BOTON3_GPIO_Port GPIOF
#define BOTON3_EXTI_IRQn EXTI9_5_IRQn
#define BOTON2_Pin GPIO_PIN_7
#define BOTON2_GPIO_Port GPIOF
#define BOTON2_EXTI_IRQn EXTI9_5_IRQn
#define BOTON1_Pin GPIO_PIN_8
#define BOTON1_GPIO_Port GPIOF
#define BOTON1_EXTI_IRQn EXTI9_5_IRQn
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define LCD_WR_Pin GPIO_PIN_0
#define LCD_WR_GPIO_Port GPIOC
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define LCD_RS_Pin GPIO_PIN_3
#define LCD_RS_GPIO_Port GPIOC
#define ADC_1_Pin GPIO_PIN_0
#define ADC_1_GPIO_Port GPIOA
#define LCD_RD_Pin GPIO_PIN_3
#define LCD_RD_GPIO_Port GPIOA
#define DAC_1_Pin GPIO_PIN_4
#define DAC_1_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_5
#define SD_SCK_GPIO_Port GPIOA
#define SD_DO_Pin GPIO_PIN_6
#define SD_DO_GPIO_Port GPIOA
#define SD_DI_Pin GPIO_PIN_7
#define SD_DI_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LCD_D0_Pin GPIO_PIN_12
#define LCD_D0_GPIO_Port GPIOF
#define LCD_D7_Pin GPIO_PIN_13
#define LCD_D7_GPIO_Port GPIOF
#define LCD_D4_Pin GPIO_PIN_14
#define LCD_D4_GPIO_Port GPIOF
#define LCD_D2_Pin GPIO_PIN_15
#define LCD_D2_GPIO_Port GPIOF
#define LCD_D6_Pin GPIO_PIN_9
#define LCD_D6_GPIO_Port GPIOE
#define LCD_D5_Pin GPIO_PIN_11
#define LCD_D5_GPIO_Port GPIOE
#define LCD_D3_Pin GPIO_PIN_13
#define LCD_D3_GPIO_Port GPIOE
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define SD_SS_Pin GPIO_PIN_14
#define SD_SS_GPIO_Port GPIOD
#define LCD_D1_Pin GPIO_PIN_15
#define LCD_D1_GPIO_Port GPIOD
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
