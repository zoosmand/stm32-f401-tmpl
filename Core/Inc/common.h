/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : Common display and touchscreen definitions.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 05.01.2026 07:41:29 PM
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

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief Runtime state and buffers for one display controller.
  * @param lock (FunctionalState) ENABLE while the display is unavailable.
  * @param model (uint16_t) Numeric display-controller model identifier.
  * @param width (uint16_t) Logical display width in pixels.
  * @param height (uint16_t) Logical display height in pixels.
  * @param orientation (uint8_t) Controller-specific orientation flags.
  * @param bus (void*) HAL bus handle owned by the peripheral layer.
  * @param pixelBuffer (uint16_t*) Foreground DMA pixel buffer.
  * @param pixelBufferSize (uint32_t) Foreground capacity in pixels.
  * @param pixelBufferActiveSize (uint32_t) Foreground pixels in the next transfer.
  * @param backgroundBuffer (uint16_t*) Background DMA pixel buffer.
  * @param backgroundBufferSize (uint32_t) Background capacity in pixels.
  * @param backgroundBufferActiveSize (uint32_t) Background pixels in the next transfer.
  * @param callback (HAL_StatusTypeDef (*)(void*)) Optional completion callback.
  */
typedef struct {
  FunctionalState lock;
  uint16_t model;
  uint16_t width;
  uint16_t height;
  uint8_t orientation;
  void* bus;
  uint16_t* pixelBuffer;
  uint32_t pixelBufferSize;
  uint32_t pixelBufferActiveSize;
  uint16_t* backgroundBuffer;
  uint32_t backgroundBufferSize;
  uint32_t backgroundBufferActiveSize;
  HAL_StatusTypeDef (*callback)(void*);
} Display_TypeDef;

/**
  * @brief Bitmap font description used by the display renderer.
  * @param width (uint8_t) Glyph width in pixels.
  * @param height (uint8_t) Glyph height in pixels.
  * @param color (uint16_t) RGB565 foreground color.
  * @param backgroundColor (uint16_t) RGB565 background color.
  * @param bytesPerGlyph (uint16_t) Encoded bytes occupied by one glyph.
  * @param fontData (const uint8_t*) Read-only glyph table.
  */
typedef struct {
  uint8_t width;
  uint8_t height;
  uint16_t color;
  uint16_t backgroundColor;
  uint16_t bytesPerGlyph;
  const uint8_t* fontData;
} Font_TypeDef;

/**
  * @brief Current coordinates and debounce state for one touchscreen.
  * @param controllerEvent (uint8_t) Raw FT6336U event code.
  * @param rawX (uint16_t) Unmapped controller X coordinate.
  * @param rawY (uint16_t) Unmapped controller Y coordinate.
  * @param x (uint16_t) Display-mapped X coordinate in pixels.
  * @param y (uint16_t) Display-mapped Y coordinate in pixels.
  * @param lastX (uint16_t) X coordinate of the previous accepted touch.
  * @param lastY (uint16_t) Y coordinate of the previous accepted touch.
  * @param referenceX (uint16_t) Last stable X coordinate.
  * @param referenceY (uint16_t) Last stable Y coordinate.
  * @param stableSampleCount (uint8_t) Consecutive stable press samples.
  * @param releaseSampleCount (uint8_t) Consecutive release samples.
  * @param touchCount (uint8_t) Number of currently reported contacts.
  * @param holdSampleCount (uint16_t) Stable samples accumulated while held.
  */
typedef struct {
  uint8_t controllerEvent;
  uint16_t rawX;
  uint16_t rawY;
  uint16_t x;
  uint16_t y;
  uint16_t lastX;
  uint16_t lastY;
  uint16_t referenceX;
  uint16_t referenceY;
  uint8_t stableSampleCount;
  uint8_t releaseSampleCount;
  uint8_t touchCount;
  uint16_t holdSampleCount;
} TouchContext_TypeDef;

/**
  * @brief Touchscreen processing state.
  */
typedef enum {
  TOUCH_STATE_IDLE,
  TOUCH_STATE_DEBOUNCE,
  TOUCH_STATE_PRESSED,
  TOUCH_STATE_HOLD,
  TOUCH_STATE_LOCKED,
  TOUCH_STATE_DISABLED,
} TouchState_TypeDef;

/**
  * @brief Application event emitted by the touchscreen state machine.
  */
typedef enum {
  TOUCH_EVENT_DOWN,
  TOUCH_EVENT_UP,
  TOUCH_EVENT_MOVE,
  TOUCH_EVENT_HOLD,
  TOUCH_EVENT_IDLE,
} TouchEvent_TypeDef;

/**
  * @brief Configuration and state for one touchscreen controller.
  * @param model (uint16_t) Numeric controller model identifier.
  * @param orientation (uint8_t) Display orientation used for coordinate mapping.
  * @param context (TouchContext_TypeDef*) Caller-independent runtime context.
  * @param state (TouchState_TypeDef) Current state-machine state.
  * @param event (TouchEvent_TypeDef) Most recently emitted application event.
  * @param bus (void*) HAL I2C handle owned by the peripheral layer.
  * @param busAddress (uint8_t) Left-shifted HAL I2C device address.
  * @param callback (HAL_StatusTypeDef (*)(void*)) Optional event callback.
  */
typedef struct {
  uint16_t model;
  uint8_t orientation;
  TouchContext_TypeDef* context;
  TouchState_TypeDef state;
  TouchEvent_TypeDef event;
  void* bus;
  uint8_t busAddress;
  HAL_StatusTypeDef (*callback)(void*);
} TouchScreen_TypeDef;

/**
  * @brief Display controller memory-transfer direction.
  */
typedef enum {
  DISPLAY_DIRECTION_WRITE = 0,
  DISPLAY_DIRECTION_READ,
  DISPLAY_DIRECTION_NONE,
} DisplayTransmissionDirection_TypeDef;

/**
  * @brief Pixel buffer selected for a drawing operation.
  */
typedef enum {
  DISPLAY_LAYER_FRONT = 0,
  DISPLAY_LAYER_BACKGROUND,
  DISPLAY_LAYER_NONE,
} DisplayLayer_TypeDef;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
