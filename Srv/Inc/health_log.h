/**
  ******************************************************************************
  * @file           : health_log.h
  * @brief          : Persistent HTTP health-check log interface.
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

#ifndef HEALTH_LOG_H
#define HEALTH_LOG_H

#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
  * @brief Recover the circular log cursor from valid records in NOR flash.
  * @retval (ErrorStatus) SUCCESS when the log is ready.
  */
ErrorStatus HealthLog_Init(void);

/**
  * @brief Append one HTTP health-check result to the persistent circular log.
  * @param statusCode (uint16_t) HTTP status, or zero when none was received.
  * @param error (uint8_t) Service-specific result category.
  * @param address (const uint8_t[4]) Resolved IPv4 address.
  * @retval (ErrorStatus) SUCCESS after the record commit marker is written.
  */
ErrorStatus HealthLog_Append(
  uint16_t statusCode,
  uint8_t error,
  const uint8_t address[4]
);

#endif /* HEALTH_LOG_H */
