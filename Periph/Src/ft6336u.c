/**
  ******************************************************************************
  * @file           : ft6336u.c
  * @brief          : FT6336U capacitive touchscreen driver.
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

#include "ft6336u.h"

extern I2C_HandleTypeDef hi2c1;

/** @brief Latch a touchscreen interrupt for deferred processing. */
static void ft6336u_InterruptCallback(void);

EXTI_HandleTypeDef touchExtiLine = {
  .Line             = TC_INT_PIN_POSITION,
  .PendingCallback  = ft6336u_InterruptCallback,
};
TouchState_TypeDef touchInterruptState = TOUCH_STATE_IDLE;

/** @brief Reset the FT6336U through its reset GPIO. */
__STATIC_INLINE void ft6336u_Reset(void) {
  HAL_GPIO_WritePin(TC_RST_GPIO_Port, TC_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(TC_RST_GPIO_Port, TC_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
}

static void ft6336u_InterruptCallback(void) {
  touchInterruptState = TOUCH_STATE_ACTIVE;
}

TouchScreen_TypeDef* FT6336U_Init(void) {

  static TouchContext_TypeDef touchContext = {};
  static TouchScreen_TypeDef touchScreen = {
    .model        = 6336,
    .orientation  = ST7796_ORIENTATION,
    .state        = TOUCH_STATE_DISABLED,
    .context      = &touchContext,
    .bus          = &hi2c1,
    .busAddress   = (FT6336U_I2C_ADDRESS << 1),
    .callback     = NULL,
  };

  TouchScreen_TypeDef* device = &touchScreen;
  I2C_HandleTypeDef* bus = (I2C_HandleTypeDef*)device->bus;

  device->state = TOUCH_STATE_LOCKED;
  if (bus->Lock == HAL_LOCKED) return device;

  GPIO_InitTypeDef gpioConfig = {0};

  gpioConfig.Pin = TC_RST_Pin;
  gpioConfig.Mode = GPIO_MODE_OUTPUT_PP;
  gpioConfig.Pull = GPIO_NOPULL;
  gpioConfig.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TC_RST_GPIO_Port, &gpioConfig);

  gpioConfig.Pin = TC_INT_Pin;
  gpioConfig.Mode = GPIO_MODE_IT_FALLING;
  gpioConfig.Pull = GPIO_PULLUP;
  gpioConfig.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TC_INT_GPIO_Port, &gpioConfig);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  ft6336u_Reset();

  if (HAL_I2C_IsDeviceReady(bus, device->busAddress, 3, 100) != HAL_OK) return device;

  uint8_t registerValue;

  if (HAL_I2C_Mem_Read(bus, device->busAddress, 0x00, I2C_MEMADD_SIZE_8BIT, &registerValue, 1, HAL_MAX_DELAY) != HAL_OK) return device;

  device->state = TOUCH_STATE_IDLE;
  return device;
}

/**
  * @brief Read the current FT6336U contact data.
  * @param device (TouchScreen_TypeDef*) Initialized touchscreen object.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
static HAL_StatusTypeDef ft6336u_Read(TouchScreen_TypeDef* device) {

  uint8_t data[7];

  if (HAL_I2C_Mem_Read((I2C_HandleTypeDef*)device->bus, device->busAddress,
      FT6336U_REGISTER_TOUCH_COUNT, I2C_MEMADD_SIZE_8BIT, data, sizeof(data),
      HAL_MAX_DELAY) != HAL_OK) return HAL_ERROR;

  uint8_t touchCount = data[0] & 0x0f;
  device->context->touchCount = touchCount;

  if (touchCount == 0) return HAL_OK;

  device->context->controllerEvent = (data[1] >> 6) & 0x03;
  device->context->rawX = ((data[1] & 0x0f) << 8) | data[2];
  device->context->rawY = ((data[3] & 0x0f) << 8) | data[4];

  return HAL_OK;
}

/**
  * @brief Map raw controller coordinates into the configured display space.
  * @param device (TouchScreen_TypeDef*) Touchscreen object containing raw data.
  */
static void ft6336u_MapToDisplay(TouchScreen_TypeDef* device) {

  int32_t x;
  int32_t y;

  switch (device->orientation & 0xf0) {
    case 0x40:
      x = device->context->rawY;
      y = device->context->rawX;
      break;

    case 0x80:
      x = (ST7796_DISPLAY_WIDTH - 1) - device->context->rawY;
      y = (ST7796_DISPLAY_HEIGHT - 1) - device->context->rawX;
      break;

    case 0xe0:
      x = device->context->rawX;
      y = (ST7796_DISPLAY_HEIGHT - 1) - device->context->rawY;
      break;

    case 0x20:
    default:
      x = (ST7796_DISPLAY_WIDTH - 1) - device->context->rawX;
      y = device->context->rawY;
      break;
  }

  device->context->x = (uint16_t)((x < 0) ? 0 : ((x >= ST7796_DISPLAY_WIDTH) ? (ST7796_DISPLAY_WIDTH - 1) : x));
  device->context->y = (uint16_t)((y < 0) ? 0 : ((y >= ST7796_DISPLAY_HEIGHT) ? (ST7796_DISPLAY_HEIGHT - 1) : y));
}

HAL_StatusTypeDef TouchScreen_Process(TouchScreen_TypeDef* device) {

  if (ft6336u_Read(device) != HAL_OK) return HAL_ERROR;

  if (device->context->touchCount) {
    ft6336u_MapToDisplay(device);
  }

  __NOP();

  switch (device->state) {
    case TOUCH_STATE_IDLE:
      if (device->context->touchCount) {
        device->state = TOUCH_STATE_DEBOUNCE;
        device->event = TOUCH_EVENT_IDLE;
        device->context->stableCount = 0;
        device->context->debounceX = device->context->x;
        device->context->debounceY = device->context->y;
      }
      break;

    case TOUCH_STATE_DEBOUNCE:
      if (!device->context->touchCount) {
        device->state = TOUCH_STATE_IDLE;
        device->event = TOUCH_EVENT_IDLE;
        device->context->stableCount = 0;
        device->context->debounceX = 0;
        device->context->debounceY = 0;
        device->context->releaseCount = 0;
        break;
      }
      if (abs(device->context->x - device->context->debounceX) <= FT6336U_MOVE_THRESHOLD_PIXELS &&
        abs(device->context->y - device->context->debounceY) <= FT6336U_MOVE_THRESHOLD_PIXELS) {
        device->context->stableCount++;

        if (device->context->stableCount >= FT6336U_STABLE_SAMPLE_COUNT) {
          if (HAL_GetTick() > device->context->deadlineMs) {
            device->state = TOUCH_STATE_ACTIVE;
            device->context->x = device->context->debounceX;
            device->context->y = device->context->debounceY;
            device->context->deadlineMs = HAL_GetTick() + FT6336U_RELEASE_DELAY_MS;
          }
        }
      } else {
        if (device->context->stableCount >= FT6336U_STABLE_SAMPLE_COUNT) {
          device->state = TOUCH_STATE_IDLE;
          device->event = TOUCH_EVENT_IDLE;
          break;
        }
        device->context->stableCount++;
        device->context->debounceX = device->context->x;
        device->context->debounceY = device->context->y;
      }
      break;

    case TOUCH_STATE_ACTIVE:
      if (device->context->stableCount >= FT6336U_STABLE_SAMPLE_COUNT) {
        device->context->holdCount++;
        if (device->context->holdCount > 1) {
          if (device->context->holdCount > 5) {
            device->state = TOUCH_STATE_HOLD;
            device->event = TOUCH_EVENT_HOLD;
            device->context->holdCount = 0;
          }
        } else {
          device->state = TOUCH_STATE_DOWN;
          device->event = TOUCH_EVENT_DOWN;
          device->context->stableCount = 0;
          device->context->holdCount = 0;
        }
      }
      break;

    case TOUCH_STATE_RELEASE:
      device->context->releaseCount++;
      if (device->context->releaseCount > FT6336U_RELEASE_SAMPLE_COUNT) {
        device->state = TOUCH_STATE_IDLE;
        device->event = TOUCH_EVENT_IDLE;
        device->context->releaseCount = 0;
      } else {
        device->event = TOUCH_EVENT_IDLE;
      }
      break;

    case TOUCH_STATE_DOWN:
      device->context->releaseCount++;
      if (device->context->releaseCount > FT6336U_RELEASE_SAMPLE_COUNT) {
        device->state = TOUCH_STATE_RELEASE;
        device->event = TOUCH_EVENT_UP;
        device->context->releaseCount = 0;
      } else {
        device->event = TOUCH_EVENT_IDLE;
      }
      break;

    /* TODO Fix On Hold event */
    case TOUCH_STATE_HOLD:
      device->context->releaseCount++;
      if (device->context->releaseCount > (FT6336U_RELEASE_SAMPLE_COUNT * 20)) {
        device->state = TOUCH_STATE_RELEASE;
        device->event = TOUCH_EVENT_HOLD;
        device->context->releaseCount = 0;
      } else {
        device->event = TOUCH_EVENT_IDLE;
      }
      break;

    default:
      break;
  }

  touchInterruptState = TOUCH_STATE_IDLE;

  return HAL_OK;
}
