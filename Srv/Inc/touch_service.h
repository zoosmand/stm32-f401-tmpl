/**
  ******************************************************************************
  * @file           : touch_service.h
  * @brief          : FreeRTOS touchscreen event-processing service.
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

#ifndef TOUCH_SERVICE_H
#define TOUCH_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "display.h"

/**
  * @brief Touchscreen service initialization result.
  */
typedef enum {
  TOUCH_SERVICE_STATUS_OK = 0,
  TOUCH_SERVICE_STATUS_ERROR
} TouchService_StatusTypeDef;

/**
  * @brief Create the touchscreen event-processing task.
  * @param display (Display_TypeDef*) Initialized display object.
  * @param touchScreen (TouchScreen_TypeDef*) Initialized touchscreen object.
  * @retval (TouchService_StatusTypeDef) Task creation result.
  */
TouchService_StatusTypeDef TouchService_Init(
  Display_TypeDef* display,
  TouchScreen_TypeDef* touchScreen
);

#ifdef __cplusplus
}
#endif

#endif /* TOUCH_SERVICE_H */
