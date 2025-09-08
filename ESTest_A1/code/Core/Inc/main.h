/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

uint8_t keyboard_scan(void);

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#define TFT_BL_Pin GPIO_PIN_1
#define TFT_BL_GPIO_Port GPIOA
#define TFT_RES_Pin GPIO_PIN_2
#define TFT_RES_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_3
#define TFT_DC_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_4
#define TFT_CS_GPIO_Port GPIOA
#define DHT11_Pin GPIO_PIN_0
#define DHT11_GPIO_Port GPIOB
#define DS18B20_Pin GPIO_PIN_1
#define DS18B20_GPIO_Port GPIOB
#define BH1750_SCL_Pin GPIO_PIN_10
#define BH1750_SCL_GPIO_Port GPIOB
#define BH1750_SDA_Pin GPIO_PIN_11
#define BH1750_SDA_GPIO_Port GPIOB
#define EEPROM_SCL_Pin GPIO_PIN_6
#define EEPROM_SCL_GPIO_Port GPIOB
#define EEPROM_SDA_Pin GPIO_PIN_7
#define EEPROM_SDA_GPIO_Port GPIOB

#define KEY_R1_Pin GPIO_PIN_11
#define KEY_R1_GPIO_Port GPIOA
#define KEY_R2_Pin GPIO_PIN_14
#define KEY_R2_GPIO_Port GPIOC
#define KEY_R3_Pin GPIO_PIN_15
#define KEY_R3_GPIO_Port GPIOC
#define KEY_R4_Pin GPIO_PIN_0
#define KEY_R4_GPIO_Port GPIOA
#define KEY_L1_Pin GPIO_PIN_3
#define KEY_L1_GPIO_Port GPIOB
#define KEY_L2_Pin GPIO_PIN_4
#define KEY_L2_GPIO_Port GPIOB
#define KEY_L3_Pin GPIO_PIN_8
#define KEY_L3_GPIO_Port GPIOB
#define KEY_L4_Pin GPIO_PIN_9
#define KEY_L4_GPIO_Port GPIOB


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
