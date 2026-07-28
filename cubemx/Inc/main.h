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
#include "user_define.h"
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
#define VCTADC 0

/* USER CODE END EM */


/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_CAN1_Init(void);
void MX_DAC_Init(void);
void MX_SPI1_Init(void);
void MX_USART6_UART_Init(void);
void MX_ADC2_Init(void);
void MX_TIM8_Init(void);
void MX_TIM3_Init(void);
void MX_TIM2_Init(void);
void MX_TIM1_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void DWT_Delay_us(uint32_t us);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PEAK_Pin GPIO_PIN_2
#define PEAK_GPIO_Port GPIOE
#define DLPS_Pin GPIO_PIN_3
#define DLPS_GPIO_Port GPIOE
#define VCT_ADC10_Pin GPIO_PIN_0
#define VCT_ADC10_GPIO_Port GPIOC
#define BD_ADC11_Pin GPIO_PIN_1
#define BD_ADC11_GPIO_Port GPIOC
#define LCR_ADC1_Pin GPIO_PIN_1
#define LCR_ADC1_GPIO_Port GPIOA
#define LCR_ADC3_Pin GPIO_PIN_3
#define LCR_ADC3_GPIO_Port GPIOA
#define LCR_DAC1_Pin GPIO_PIN_4
#define LCR_DAC1_GPIO_Port GPIOA
#define BD_DAC2_Pin GPIO_PIN_5
#define BD_DAC2_GPIO_Port GPIOA
#define RGB1_Pin GPIO_PIN_9
#define RGB1_GPIO_Port GPIOE
#define RGB2_Pin GPIO_PIN_11
#define RGB2_GPIO_Port GPIOE
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
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

CAN_HandleTypeDef hcan1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
DMA_HandleTypeDef hdma_dac2;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim8;
DMA_HandleTypeDef hdma_tim1_ch1;
DMA_HandleTypeDef hdma_tim1_ch2;
UART_HandleTypeDef huart6;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
