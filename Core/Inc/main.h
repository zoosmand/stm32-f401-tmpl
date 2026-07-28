/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Application-wide declarations and board pin assignments.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 03.01.2026 06:40:21 PM
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Dmitry Slobodchikov
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "fonts.h"
#include "common.h"
#include "st7796.h"
#include "ft6336u.h"
#include "display.h"

#include "wizchip_port.h"

extern Display_TypeDef* displayDevice;

/**
  * @brief Enter the unrecoverable system-error loop.
  */
void Error_Handler(void);

/**
  * @brief Busy-wait for a number of microseconds using the DWT cycle counter.
  * @param delayUs (uint32_t) Delay duration in microseconds.
  */
void Delay_Microseconds(uint32_t delayUs);

/**
  * @brief Busy-wait for a number of milliseconds using the HAL tick.
  * @param delayMs (uint32_t) Delay duration in milliseconds.
  */
void Delay_Milliseconds(uint32_t delayMs);

#define HB_LED_Pin GPIO_PIN_13
#define HB_LED_GPIO_Port GPIOC
#define TFT_DC_Pin GPIO_PIN_2
#define TFT_DC_GPIO_Port GPIOA
#define TFT_RST_Pin GPIO_PIN_3
#define TFT_RST_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_4
#define TFT_CS_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOA
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

#define TC_INT_PIN_POSITION 9U
#define DSPL_OUT

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
