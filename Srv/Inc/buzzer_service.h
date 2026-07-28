/**
  ******************************************************************************
  * @file           : buzzer_service.h
  * @brief          : Non-blocking FreeRTOS buzzer service interface.
  * @project        : STM32F401 Health Check
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 28.07.2026
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

#ifndef BUZZER_SERVICE_H
#define BUZZER_SERVICE_H

#include <stdint.h>

/**
  * @brief Buzzer service operation status.
  */
typedef enum {
  BUZZER_SERVICE_STATUS_OK = 0,
  BUZZER_SERVICE_STATUS_ERROR,
} BuzzerService_StatusTypeDef;

/**
  * @brief Initialize the buzzer and create its static command queue and task.
  * @retval (BuzzerService_StatusTypeDef) Service initialization status.
  */
BuzzerService_StatusTypeDef BuzzerService_Init(void);

/**
  * @brief Queue a finite-duration tone without blocking the caller.
  * @param frequencyHz (uint32_t) Tone frequency in hertz.
  * @param durationMs (uint32_t) Tone duration in milliseconds.
  * @retval (BuzzerService_StatusTypeDef) Queueing status.
  */
BuzzerService_StatusTypeDef BuzzerService_Play(
  uint32_t frequencyHz,
  uint32_t durationMs
);

#endif /* BUZZER_SERVICE_H */
