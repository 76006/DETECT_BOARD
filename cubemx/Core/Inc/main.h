/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DLPS_Pin GPIO_PIN_3
#define DLPS_GPIO_Port GPIOE
#define LCR_ADC1_Pin GPIO_PIN_1
#define LCR_ADC1_GPIO_Port GPIOA
#define LCR_ADC3_Pin GPIO_PIN_3
#define LCR_ADC3_GPIO_Port GPIOA
#define LCR_DAC1_Pin GPIO_PIN_4
#define LCR_DAC1_GPIO_Port GPIOA
#define BD_DAC2_Pin GPIO_PIN_5
#define BD_DAC2_GPIO_Port GPIOA
#define CONV_Pin GPIO_PIN_15
#define CONV_GPIO_Port GPIOA
#define SCK_Pin GPIO_PIN_3
#define SCK_GPIO_Port GPIOB
#define SDO_Pin GPIO_PIN_4
#define SDO_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_0
#define LED_G_GPIO_Port GPIOE
#define LED_R_Pin GPIO_PIN_1
#define LED_R_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
