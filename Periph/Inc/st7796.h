/**
  ******************************************************************************
  * @file           : st7796.h
  * @brief          : ST7796 TFT display interface.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 03.01.2026 08:05:59 PM
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

#ifndef ST7796_H
#define ST7796_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define ST7796_ORIENTATION       (0x80 | 0x00)

#if ((ST7796_ORIENTATION >> 6) < 3 && (ST7796_ORIENTATION >> 6) != 0)
  #define ST7796_IS_PORTRAIT 0
  #define ST7796_DISPLAY_WIDTH   480
  #define ST7796_DISPLAY_HEIGHT  320
#else
  #define ST7796_IS_PORTRAIT 1
  #define ST7796_DISPLAY_WIDTH   320
  #define ST7796_DISPLAY_HEIGHT  480
#endif

#define DISPLAY_COLOR_WHITE       (uint16_t)0xffff
#define DISPLAY_COLOR_BLACK       (uint16_t)0x0000
#define DISPLAY_COLOR_RED         (uint16_t)0xf800
#define DISPLAY_COLOR_GREEN       (uint16_t)0x001f
#define DISPLAY_COLOR_BLUE        (uint16_t)0x07e0
#define DISPLAY_COLOR_PURPLE      (uint16_t)(DISPLAY_COLOR_RED | DISPLAY_COLOR_BLUE)
#define DISPLAY_COLOR_SKY         (uint16_t)(DISPLAY_COLOR_GREEN | DISPLAY_COLOR_BLUE)
#define DISPLAY_COLOR_LIME        (uint16_t)(DISPLAY_COLOR_GREEN | DISPLAY_COLOR_RED)

#define ST7796_PIXEL_BUFFER_SIZE        4096U
#define ST7796_DMA_TIMEOUT_MS            100U

extern uint8_t __dma_buffer_write_start__;
extern uint8_t __dma_buffer_write_end__;
extern uint8_t __dma_buffer_read_start__;
extern uint8_t __dma_buffer_read_end__;

/**
  * @brief Initialize the ST7796 controller and clear the display.
  * @retval (Display_TypeDef*) Persistent display object. Its lock member is
  *         DISABLE when initialization succeeds.
  */
Display_TypeDef* ST7796_Init(void);

/**
  * @brief Fill the complete display with one RGB565 color.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param color (uint16_t) RGB565 fill color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_Fill(
  Display_TypeDef* device, uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Draw a rectangular RGB565 border.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Left coordinate in pixels.
  * @param y (uint16_t) Top coordinate in pixels.
  * @param width (uint16_t) Rectangle width in pixels.
  * @param height (uint16_t) Rectangle height in pixels.
  * @param thickness (uint16_t) Border thickness in pixels.
  * @param color (uint16_t) RGB565 border color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_DrawRectangle(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width,
  uint16_t height, uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Fill a rectangular display region.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Left coordinate in pixels.
  * @param y (uint16_t) Top coordinate in pixels.
  * @param width (uint16_t) Region width in pixels.
  * @param height (uint16_t) Region height in pixels.
  * @param color (uint16_t) RGB565 fill color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_FillRectangle(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width,
  uint16_t height, uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Draw one pixel.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Horizontal coordinate in pixels.
  * @param y (uint16_t) Vertical coordinate in pixels.
  * @param color (uint16_t) RGB565 pixel color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_DrawPixel(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t color,
  DisplayLayer_TypeDef layer);

/**
  * @brief Draw a vertical line.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Horizontal coordinate in pixels.
  * @param y (uint16_t) Starting vertical coordinate in pixels.
  * @param length (uint16_t) Line length in pixels.
  * @param thickness (uint16_t) Line thickness in pixels.
  * @param color (uint16_t) RGB565 line color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_DrawVLine(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t length,
  uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Draw a horizontal line.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Starting horizontal coordinate in pixels.
  * @param y (uint16_t) Vertical coordinate in pixels.
  * @param length (uint16_t) Line length in pixels.
  * @param thickness (uint16_t) Line thickness in pixels.
  * @param color (uint16_t) RGB565 line color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_DrawHLine(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t length,
  uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Draw a circle border.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param centerX (uint16_t) Circle-center X coordinate in pixels.
  * @param centerY (uint16_t) Circle-center Y coordinate in pixels.
  * @param radius (uint16_t) Circle radius in pixels.
  * @param thickness (uint16_t) Border thickness in pixels.
  * @param color (uint16_t) RGB565 border color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_DrawCircle(
  Display_TypeDef* device, uint16_t centerX, uint16_t centerY, uint16_t radius,
  uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Draw a filled circle.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param centerX (uint16_t) Circle-center X coordinate in pixels.
  * @param centerY (uint16_t) Circle-center Y coordinate in pixels.
  * @param radius (uint16_t) Circle radius in pixels.
  * @param color (uint16_t) RGB565 fill color.
  * @param layer (DisplayLayer_TypeDef) Destination pixel buffer.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_FillCircle(
  Display_TypeDef* device, uint16_t centerX, uint16_t centerY, uint16_t radius,
  uint16_t color, DisplayLayer_TypeDef layer);

/**
  * @brief Restore a rectangle from the background buffer.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Left coordinate in pixels.
  * @param y (uint16_t) Top coordinate in pixels.
  * @param width (uint16_t) Region width in pixels.
  * @param height (uint16_t) Region height in pixels.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_FillBackground(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
  * @brief Read a rectangle into the background buffer.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Left coordinate in pixels.
  * @param y (uint16_t) Top coordinate in pixels.
  * @param width (uint16_t) Region width in pixels.
  * @param height (uint16_t) Region height in pixels.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_ReadRectangle(
  Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
  * @brief Render one font symbol.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Left coordinate in pixels.
  * @param y (uint16_t) Top coordinate in pixels.
  * @param font (Font_TypeDef*) Bitmap font description.
  * @param symbol (char) Printable ASCII character.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_PrintSymbol(
  Display_TypeDef* device, uint16_t x, uint16_t y, Font_TypeDef* font, char symbol);

/**
  * @brief Render a null-terminated string, stopping at newline.
  * @param device (Display_TypeDef*) Initialized display object.
  * @param x (uint16_t) Left coordinate in pixels.
  * @param y (uint16_t) Top coordinate in pixels.
  * @param font (Font_TypeDef*) Bitmap font description.
  * @param string (const char*) Null-terminated text; ownership remains with caller.
  * @retval (HAL_StatusTypeDef) HAL_OK on success.
  */
HAL_StatusTypeDef Display_PrintString(
  Display_TypeDef* device, uint16_t x, uint16_t y, Font_TypeDef* font,
  const char* string);

#ifdef __cplusplus
}
#endif

#endif /* ST7796_H */
