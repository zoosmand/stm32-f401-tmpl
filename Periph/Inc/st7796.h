/**
  ******************************************************************************
  * @file           : st7796.h
  * @brief          : Header for st7796.c file.
  *                   This file contains the common defines of the ST7796 TFT
  *                   driver code.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Askug Ltd.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ST7796_H
#define __ST7796_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 0x20 or 0xe0 - vertical
// 0x40 or 0x80 - horizontal
// 0x00 - RGB
// 0x08 - BRG
#define ORIENTATION       (0x80 | 0x00)

#if ((ORIENTATION >> 6) < 3 && (ORIENTATION >> 6) != 0)
  #define DISPLAY_POSITION 0 // horizontal
  #define DISPLAY_WIDTH   480
  #define DISPLAY_HEIGHT  320
#else
  #define DISPLAY_POSITION 1 // vertical
  #define DISPLAY_WIDTH   320
  #define DISPLAY_HEIGHT  480
#endif



/* Define colors*/
#define COLOR_WHITE       (uint16_t)0xffff
#define COLOR_BLACK       (uint16_t)0x0000
#define COLOR_RED         (uint16_t)0xf800
#define COLOR_GREEN       (uint16_t)0x001f
#define COLOR_BLUE        (uint16_t)0x07e0
#define COLOR_PURPLE      (uint16_t)(COLOR_RED | COLOR_BLUE)
#define COLOR_SKY         (uint16_t)(COLOR_GREEN | COLOR_BLUE)
#define COLOR_LIME        (uint16_t)(COLOR_GREEN | COLOR_RED)


#define PIX_BUF_SZ        4096U  // words (4096 pixels)

#define TFT_CS_GPIO_Port  GPIOA
#define TFT_CS_Pin        GPIO_PIN_4

#define TFT_DC_GPIO_Port  GPIOA
#define TFT_DC_Pin        GPIO_PIN_2

#define TFT_RST_GPIO_Port GPIOA
#define TFT_RST_Pin       GPIO_PIN_3

/* TODO realise PWM for LED pin */
// #define TFT_BL_GPIO_Port GPIOB
// #define TFT_BL_Pin       GPIO_PIN_10   // optional (or tie to VCC)


extern uint8_t __dma_buffer_write_start__;
extern uint8_t __dma_buffer_write_end__;


Display_TypeDef* ST7796_Init(void);



HAL_StatusTypeDef __attribute__((weak)) Display_Fill(Display_TypeDef*, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_DrawRectangle(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_DrawPixel(Display_TypeDef*, uint16_t, uint16_t, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_DrawCircle(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, ImageLayer_t);
HAL_StatusTypeDef __attribute__((weak)) Display_FillCircle(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, ImageLayer_t);

HAL_StatusTypeDef __attribute__((weak)) Display_FillBackground(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t);
HAL_StatusTypeDef __attribute__((weak)) Display_ReadRectangle(Display_TypeDef*, uint16_t, uint16_t, uint16_t, uint16_t);

HAL_StatusTypeDef __attribute__((weak)) Display_PrintSymbol(Display_TypeDef*, uint16_t, uint16_t, Font_TypeDef*, char);
HAL_StatusTypeDef __attribute__((weak)) Display_PrintString(Display_TypeDef*, uint16_t, uint16_t, Font_TypeDef*, const char*);

#ifdef __cplusplus
}
#endif

#endif /* __ST7796_H */
