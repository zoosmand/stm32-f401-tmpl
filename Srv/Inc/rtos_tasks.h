/**
  ******************************************************************************
  * @file           : rtos_tasks.h
  * @brief          : Application FreeRTOS task creation and shared locks.
  * @project        : STM32F401 Test Platform
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

#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "display.h"

/**
  * @brief RTOS application initialization result.
  */
typedef enum {
  RTOS_TASKS_STATUS_OK = 0,
  RTOS_TASKS_STATUS_ERROR
} RtosTasks_StatusTypeDef;

/**
  * @brief Create the default, WIZ server, and WIZ client tasks.
  * @param display (Display_TypeDef*) Initialized display object.
  * @param touchScreen (TouchScreen_TypeDef*) Initialized touchscreen object.
  * @retval (RtosTasks_StatusTypeDef) Task creation result.
  */
RtosTasks_StatusTypeDef RtosTasks_Init(
  Display_TypeDef* display,
  TouchScreen_TypeDef* touchScreen
);

/**
  * @brief Lock the display for a task-level drawing operation.
  */
void RtosTasks_DisplayLock(void);

/**
  * @brief Release the display after a task-level drawing operation.
  */
void RtosTasks_DisplayUnlock(void);

#ifdef __cplusplus
}
#endif

#endif /* RTOS_TASKS_H */
