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
#include "stm32l0xx_hal.h"

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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define D0_Pin GPIO_PIN_0
#define D0_GPIO_Port GPIOC
#define D1_Pin GPIO_PIN_1
#define D1_GPIO_Port GPIOC
#define D2_Pin GPIO_PIN_2
#define D2_GPIO_Port GPIOC
#define D3_Pin GPIO_PIN_3
#define D3_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_5
#define LED_GPIO_Port GPIOA
#define E_Pin GPIO_PIN_10
#define E_GPIO_Port GPIOB
#define RS_Pin GPIO_PIN_11
#define RS_GPIO_Port GPIOB
#define IN4_Pin GPIO_PIN_6
#define IN4_GPIO_Port GPIOC
#define IN3_Pin GPIO_PIN_8
#define IN3_GPIO_Port GPIOC
#define D4_Pin GPIO_PIN_10
#define D4_GPIO_Port GPIOA
#define D5_Pin GPIO_PIN_11
#define D5_GPIO_Port GPIOA
#define D6_Pin GPIO_PIN_12
#define D6_GPIO_Port GPIOA
#define D7_Pin GPIO_PIN_13
#define D7_GPIO_Port GPIOA
#define IN1_Pin GPIO_PIN_10
#define IN1_GPIO_Port GPIOC
#define IN2_Pin GPIO_PIN_12
#define IN2_GPIO_Port GPIOC
#define IR1_Pin GPIO_PIN_3
#define IR1_GPIO_Port GPIOB
#define IR3_Pin GPIO_PIN_4
#define IR3_GPIO_Port GPIOB
#define IR2_Pin GPIO_PIN_5
#define IR2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
