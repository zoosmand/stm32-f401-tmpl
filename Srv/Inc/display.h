/**
  ******************************************************************************
  * @file           : display.h
  * @brief          : Touch-driven display demonstration service.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 05.01.2026 03:37:54 PM
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

#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief Handle a processed touch event on the demonstration display.
  * @param display (Display_TypeDef*) Initialized display object.
  * @param touchScreen (TouchScreen_TypeDef*) Processed touchscreen object.
  */
void Display_HandleTouchEvent(
  Display_TypeDef* display,
  TouchScreen_TypeDef* touchScreen
);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */
