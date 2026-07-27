/**
  ******************************************************************************
  * @file           : ft6336u.h
  * @brief          : FT6336U capacitive touchscreen interface.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 24.01.2026 12:51:32 PM
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

#ifndef FT6336U_H
#define FT6336U_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define FT6336U_I2C_ADDRESS                 0x38U
#define FT6336U_REGISTER_TOUCH_COUNT        0x02U
#define FT6336U_REGISTER_TOUCH1_X_HIGH      0x03U
#define FT6336U_REGISTER_TOUCH1_X_LOW       0x04U
#define FT6336U_REGISTER_TOUCH1_Y_HIGH      0x05U
#define FT6336U_REGISTER_TOUCH1_Y_LOW       0x06U
#define FT6336U_STABLE_SAMPLE_COUNT         3U
#define FT6336U_MOVE_THRESHOLD_PIXELS       3U
#define FT6336U_RELEASE_SAMPLE_COUNT        5U
#define FT6336U_DEADZONE_PIXELS             3U
#define FT6336U_RELEASE_DELAY_MS            500U

/**
  * @brief Initialize the FT6336U controller and its GPIO interrupt.
  * @retval (TouchScreen_TypeDef*) Persistent device object. Its state is
  *         TOUCH_STATE_IDLE when initialization succeeds.
  */
TouchScreen_TypeDef* FT6336U_Init(void);

/**
  * @brief Read and process one touchscreen sample.
  * @param device (TouchScreen_TypeDef*) Initialized touchscreen object.
  * @retval (HAL_StatusTypeDef) HAL_OK on a valid sample; HAL_ERROR on I2C failure.
  */
HAL_StatusTypeDef TouchScreen_Process(TouchScreen_TypeDef* device);

/**
  * @brief EXTI handle used by the FT6336U interrupt line.
  */
extern EXTI_HandleTypeDef touchExtiLine;

/**
  * @brief Interrupt-latched touchscreen activity state.
  */
extern TouchState_TypeDef touchInterruptState;

#ifdef __cplusplus
}
#endif

#endif /* FT6336U_H */
