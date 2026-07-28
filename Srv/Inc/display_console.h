/**
  ******************************************************************************
  * @file           : display_console.h
  * @brief          : Display-backed standard output service.
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

#ifndef DISPLAY_CONSOLE_H
#define DISPLAY_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "display.h"

/**
  * @brief Display console initialization result.
  */
typedef enum {
  DISPLAY_CONSOLE_STATUS_OK = 0,
  DISPLAY_CONSOLE_STATUS_ERROR
} DisplayConsole_StatusTypeDef;

/**
  * @brief Initialize the display console queue and rendering task.
  * @param display (Display_TypeDef*) Initialized display object.
  * @retval (DisplayConsole_StatusTypeDef) Initialization result.
  */
DisplayConsole_StatusTypeDef DisplayConsole_Init(Display_TypeDef* display);

/**
  * @brief Submit characters from a standard-output write operation.
  * @param data (const char*) Characters to process.
  * @param length (uint32_t) Number of supplied characters.
  * @retval (uint32_t) Number of accepted characters.
  */
uint32_t DisplayConsole_Write(const char* data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_CONSOLE_H */
