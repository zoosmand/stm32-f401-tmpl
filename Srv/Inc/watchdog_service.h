/**
  ******************************************************************************
  * @file           : watchdog_service.h
  * @brief          : Independent watchdog FreeRTOS service interface.
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

#ifndef WATCHDOG_SERVICE_H
#define WATCHDOG_SERVICE_H

typedef enum {
  WATCHDOG_SERVICE_STATUS_OK = 0,
  WATCHDOG_SERVICE_STATUS_ERROR,
} WatchdogService_StatusTypeDef;

/**
  * @brief Create the statically allocated watchdog task in its stopped state.
  * @retval (WatchdogService_StatusTypeDef) Task creation status.
  */
WatchdogService_StatusTypeDef WatchdogService_Init(void);

/**
  * @brief Start IWDG supervision after all potentially slow startup work.
  *
  * The start request is idempotent. Once started, hardware prevents the IWDG
  * from being disabled until the next reset.
  */
void WatchdogService_Start(void);

#endif /* WATCHDOG_SERVICE_H */
