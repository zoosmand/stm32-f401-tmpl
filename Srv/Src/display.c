/**
  ******************************************************************************
  * @file           : display.c
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

#include "display.h"

/**
  * @brief Render coordinates and crosshairs for a released touch.
  * @param display (Display_TypeDef*) Initialized display object.
  * @param touchScreen (TouchScreen_TypeDef*) Processed touchscreen object.
  */
static void display_OnUp(Display_TypeDef* display, TouchScreen_TypeDef* touchScreen) {
  Font_TypeDef font = {
    .backgroundColor = DISPLAY_COLOR_BLUE,
    .color = DISPLAY_COLOR_LIME,
    .fontData = (const uint8_t*)&fontDot10x14,
    .height = 16,
    .width = 12,
    .bytesPerGlyph = 24,
  };

  Display_DrawVLine(display, touchScreen->context->lastX, 0, ST7796_DISPLAY_HEIGHT, 2, DISPLAY_COLOR_BLACK, DISPLAY_LAYER_FRONT);
  Display_DrawHLine(display, 0, touchScreen->context->lastY, ST7796_DISPLAY_WIDTH, 2, DISPLAY_COLOR_BLACK, DISPLAY_LAYER_FRONT);

  char positionText[20];
  snprintf(positionText, sizeof(positionText), "x:%u y:%u\n",
    touchScreen->context->x, touchScreen->context->y);
  Display_FillRectangle(display, 40, 80, (font.width * 10), font.height, DISPLAY_COLOR_BLACK, DISPLAY_LAYER_FRONT);
  Display_PrintString(display, 10, 80, &font, positionText);

  Display_DrawVLine(display, touchScreen->context->x, 0, ST7796_DISPLAY_HEIGHT, 2, DISPLAY_COLOR_WHITE, DISPLAY_LAYER_FRONT);
  Display_DrawHLine(display, 0, touchScreen->context->y, ST7796_DISPLAY_WIDTH, 2, DISPLAY_COLOR_WHITE, DISPLAY_LAYER_FRONT);

  touchScreen->context->lastX = touchScreen->context->x;
  touchScreen->context->lastY = touchScreen->context->y;
}

void Display_Run(Display_TypeDef* display, TouchScreen_TypeDef* touchScreen) {

  if (display->lock == ENABLE) return;
  if (touchInterruptState != TOUCH_STATE_ACTIVE) return;

  TouchScreen_Process(touchScreen);

  switch (touchScreen->event) {
    case TOUCH_EVENT_UP:
      display_OnUp(display, touchScreen);
      break;

    case TOUCH_EVENT_IDLE:
    default:
      __NOP();
      break;
  }
}

