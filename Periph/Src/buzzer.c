/**
  ******************************************************************************
  * @file           : buzzer.c
  * @brief          : Passive buzzer PWM implementation using TIM1 channel 1.
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

#include "buzzer.h"

#include "main.h"

#define BUZZER_TIMER_CLOCK_HZ    84000000UL
#define BUZZER_TIMER_TICK_HZ      1000000UL
#define BUZZER_TIMER_CHANNEL    TIM_CHANNEL_1

static TIM_HandleTypeDef buzzerTimer;

ErrorStatus Buzzer_Init(void) {
  GPIO_InitTypeDef gpio = {0};
  TIM_OC_InitTypeDef outputCompare = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM1_CLK_ENABLE();

  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
  gpio.Pin = BUZZER_Pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &gpio);

  buzzerTimer.Instance = TIM1;
  buzzerTimer.Init.Prescaler =
    (BUZZER_TIMER_CLOCK_HZ / BUZZER_TIMER_TICK_HZ) - 1U;
  buzzerTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
  buzzerTimer.Init.Period = 0xffffU;
  buzzerTimer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  buzzerTimer.Init.RepetitionCounter = 0U;
  buzzerTimer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  if (HAL_TIM_PWM_Init(&buzzerTimer) != HAL_OK)
    return ERROR;

  outputCompare.OCMode = TIM_OCMODE_PWM1;
  outputCompare.Pulse = 0U;
  outputCompare.OCPolarity = TIM_OCPOLARITY_HIGH;
  outputCompare.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  outputCompare.OCFastMode = TIM_OCFAST_DISABLE;
  outputCompare.OCIdleState = TIM_OCIDLESTATE_RESET;
  outputCompare.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(
      &buzzerTimer,
      &outputCompare,
      BUZZER_TIMER_CHANNEL
    ) != HAL_OK) {
    return ERROR;
  }

  Buzzer_Stop();
  return SUCCESS;
}

ErrorStatus Buzzer_Start(uint32_t frequencyHz) {
  uint32_t period;

  if ((frequencyHz < BUZZER_MIN_FREQUENCY_HZ)
      || (frequencyHz > BUZZER_MAX_FREQUENCY_HZ)) {
    return ERROR;
  }

  period = BUZZER_TIMER_TICK_HZ / frequencyHz;
  if ((period < 2U) || (period > 0x10000UL))
    return ERROR;

  if (HAL_TIM_PWM_Stop(&buzzerTimer, BUZZER_TIMER_CHANNEL) != HAL_OK)
    return ERROR;

  __HAL_TIM_SET_AUTORELOAD(&buzzerTimer, period - 1U);
  __HAL_TIM_SET_COMPARE(&buzzerTimer, BUZZER_TIMER_CHANNEL, period / 2U);
  __HAL_TIM_SET_COUNTER(&buzzerTimer, 0U);
  buzzerTimer.Instance->EGR = TIM_EGR_UG;

  return (HAL_TIM_PWM_Start(
    &buzzerTimer,
    BUZZER_TIMER_CHANNEL
  ) == HAL_OK) ? SUCCESS : ERROR;
}

void Buzzer_Stop(void) {
  if (buzzerTimer.Instance == NULL)
    return;

  (void)HAL_TIM_PWM_Stop(&buzzerTimer, BUZZER_TIMER_CHANNEL);
  __HAL_TIM_SET_COMPARE(&buzzerTimer, BUZZER_TIMER_CHANNEL, 0U);
  __HAL_TIM_SET_COUNTER(&buzzerTimer, 0U);
  buzzerTimer.Instance->EGR = TIM_EGR_UG;
}

ErrorStatus Buzzer_SelfTest(void) {
  if (Buzzer_Start(BUZZER_SELF_TEST_FREQUENCY_HZ) != SUCCESS) {
    Buzzer_Stop();
    return ERROR;
  }

  HAL_Delay(BUZZER_SELF_TEST_DURATION_MS);
  Buzzer_Stop();
  return SUCCESS;
}
