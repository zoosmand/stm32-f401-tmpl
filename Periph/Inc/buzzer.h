/**
  ******************************************************************************
  * @file           : buzzer.h
  * @brief          : Passive buzzer PWM interface.
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

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

#include "stm32f4xx_hal.h"

#define BUZZER_MIN_FREQUENCY_HZ          16U
#define BUZZER_MAX_FREQUENCY_HZ       20000U
#define BUZZER_SELF_TEST_FREQUENCY_HZ  2000U
#define BUZZER_SELF_TEST_DURATION_MS     200U

/**
  * @brief Configure PA8 and TIM1 channel 1 for passive-buzzer PWM.
  * @retval (ErrorStatus) SUCCESS when the output is initialized and silent.
  */
ErrorStatus Buzzer_Init(void);

/**
  * @brief Start a square-wave tone on the buzzer.
  * @param frequencyHz (uint32_t) Tone frequency in hertz.
  * @retval (ErrorStatus) SUCCESS when the requested frequency is valid.
  */
ErrorStatus Buzzer_Start(uint32_t frequencyHz);

/**
  * @brief Stop the tone and leave the 2N2222 transistor switched off.
  */
void Buzzer_Stop(void);

/**
  * @brief Emit the short, blocking audible confirmation tone.
  * @retval (ErrorStatus) SUCCESS when the test tone was generated.
  *
  * This verifies the configured timer path but cannot electrically confirm
  * that the buzzer produced sound. Services should use BuzzerService_Play()
  * when the caller must remain non-blocking.
  */
ErrorStatus Buzzer_SelfTest(void);

#endif /* BUZZER_H */
