/**
  ******************************************************************************
  * @file           : time_service.h
  * @brief          : Network time synchronization service interface.
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

#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

/**
  * @brief Result of creating the network time task.
  */
typedef enum {
  TIME_SERVICE_STATUS_OK = 0,
  TIME_SERVICE_STATUS_ERROR,
} TimeService_StatusTypeDef;

/**
  * @brief Create the statically allocated RTC synchronization task.
  * @retval (TimeService_StatusTypeDef) Service initialization status.
  */
TimeService_StatusTypeDef TimeService_Init(void);

/**
  * @brief Supply the WIZnet DNS client with its required one-second tick.
  *
  * This function must be called from every FreeRTOS tick interrupt.
  */
void TimeService_Tick(void);

#endif /* TIME_SERVICE_H */
