/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#include "fonts.h"
#include "common.h"
#include "st7796.h"
#include "ft6336u.h"
#include "display.h"

#include "wizchip_port.h"
#include "loopback/loopback.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern Display_TypeDef* display_0;

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
void __attribute__((weak)) Error_Handler(void);
void _delay_us(uint32_t);
void _delay_ms(uint32_t);


/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HB_LED_Pin GPIO_PIN_13
#define HB_LED_GPIO_Port GPIOC
#define TFT_DC_Pin GPIO_PIN_2
#define TFT_DC_GPIO_Port GPIOA
#define TFT_RST_Pin GPIO_PIN_3
#define TFT_RST_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_4
#define TFT_CS_GPIO_Port GPIOA
#define ETH_IN_Pin GPIO_PIN_0
#define ETH_IN_GPIO_Port GPIOB
#define ETH_RESET_Pin GPIO_PIN_1
#define ETH_RESET_GPIO_Port GPIOB
#define TC_RST_Pin GPIO_PIN_2
#define TC_RST_GPIO_Port GPIOB
#define ETH_CS_Pin GPIO_PIN_15
#define ETH_CS_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_8
#define SD_CS_GPIO_Port GPIOB
#define TC_INT_Pin GPIO_PIN_9
#define TC_INT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define TC_INT_Pin_Pos    9
#define DSPL_OUT


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
