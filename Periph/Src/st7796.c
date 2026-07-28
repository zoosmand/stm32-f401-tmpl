/**
  ******************************************************************************
  * @file           : st7796.c
  * @brief          : ST7796 TFT display driver.
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

#include "st7796.h"

static __IO bool st7796DmaBusy = false;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

/** @brief Select command mode on the ST7796 data/command pin. */
__STATIC_INLINE void st7796_SetCommandMode(void) {
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
}

/** @brief Select data mode on the ST7796 data/command pin. */
__STATIC_INLINE void st7796_SetDataMode(void) {
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
}

/** @brief Write one ST7796 command byte. */
__STATIC_INLINE void st7796_WriteCommand(Display_TypeDef* device, uint8_t command) {
  st7796_SetCommandMode();
  HAL_SPI_Transmit((SPI_HandleTypeDef*)device->bus, &command, 1, HAL_MAX_DELAY);
}

/** @brief Write a blocking byte sequence to the ST7796. */
__STATIC_INLINE void st7796_WriteData(Display_TypeDef* device, const uint8_t *data, uint32_t length) {
  st7796_SetDataMode();
  HAL_SPI_Transmit((SPI_HandleTypeDef*)device->bus, (uint8_t*)data, length, HAL_MAX_DELAY);
}

/** @brief Reset the ST7796 through its reset GPIO. */
__STATIC_INLINE void st7796_Reset(void) {
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796DmaBusy = false;
  }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796DmaBusy = false;
  }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796DmaBusy = false;
  }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796DmaBusy = false;
  }
}

/**
  * @brief Wait for the active display DMA transfer with a bounded timeout.
  * @param device (Display_TypeDef*) Display owning the SPI transfer.
  * @retval (HAL_StatusTypeDef) HAL_OK when complete or HAL_TIMEOUT after abort.
  */
static HAL_StatusTypeDef st7796_WaitForDma(Display_TypeDef* device) {
  uint32_t startTick = HAL_GetTick();

  while (st7796DmaBusy) {
    if ((HAL_GetTick() - startTick) >= ST7796_DMA_TIMEOUT_MS) {
      (void)HAL_SPI_Abort((SPI_HandleTypeDef*)device->bus);
      st7796DmaBusy = false;
      return HAL_TIMEOUT;
    }
  }

  return HAL_OK;
}

/** @brief Transfer the active foreground pixels through SPI DMA. */
__STATIC_INLINE HAL_StatusTypeDef st7796_WriteDataDma(Display_TypeDef* device) {
  if ((device == NULL) || (device->pixelBufferActiveSize == 0U) ||
      (device->pixelBufferActiveSize > device->pixelBufferSize)) return HAL_ERROR;

  HAL_StatusTypeDef status = st7796_WaitForDma(device);
  if (status != HAL_OK) return status;

  st7796_SetDataMode();
  st7796DmaBusy = true;

  if (HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)device->bus, (uint8_t*)device->pixelBuffer, (device->pixelBufferActiveSize * 2)) != HAL_OK) {
    st7796DmaBusy = false;
    return HAL_ERROR;
  }
  return st7796_WaitForDma(device);
}

/** @brief Transfer the active background pixels through SPI DMA. */
__STATIC_INLINE HAL_StatusTypeDef st7796_WriteBackgroundDataDma(Display_TypeDef* device) {
  if ((device == NULL) || (device->backgroundBufferActiveSize == 0U) ||
      (device->backgroundBufferActiveSize > device->backgroundBufferSize)) return HAL_ERROR;

  HAL_StatusTypeDef status = st7796_WaitForDma(device);
  if (status != HAL_OK) return status;

  st7796_SetDataMode();
  st7796DmaBusy = true;

  if (HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)device->bus, (uint8_t*)device->backgroundBuffer, (device->backgroundBufferActiveSize * 2)) != HAL_OK) {
    st7796DmaBusy = false;
    return HAL_ERROR;
  }
  return st7796_WaitForDma(device);
}

/** @brief Read active pixels into the background buffer through SPI DMA. */
__STATIC_INLINE HAL_StatusTypeDef st7796_ReadDataDma(Display_TypeDef* device) {
  if ((device == NULL) || (device->backgroundBufferActiveSize == 0U) ||
      (device->backgroundBufferActiveSize > device->backgroundBufferSize)) return HAL_ERROR;

  HAL_StatusTypeDef status = st7796_WaitForDma(device);
  if (status != HAL_OK) return status;

  HAL_StatusTypeDef transferStatus;
  uint8_t dummyByte = 0;
  SPI_HandleTypeDef* bus = (SPI_HandleTypeDef*)device->bus;

  hdma_spi1_tx.Init.MemInc = DMA_MINC_DISABLE;
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) return HAL_ERROR;

  st7796_SetDataMode();
  st7796DmaBusy = true;

  if (HAL_SPI_TransmitReceive_DMA(bus, &dummyByte, (uint8_t*)device->backgroundBuffer, (device->backgroundBufferActiveSize *2)) != HAL_OK) {
    st7796DmaBusy = false;
    transferStatus = HAL_ERROR;
  } else {
    transferStatus = st7796_WaitForDma(device);
  }

  hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) return HAL_ERROR;

  return transferStatus;
}

/** @brief Select a rectangular controller-memory window and transfer direction. */
__STATIC_INLINE void st7796_SetWindow(
    Display_TypeDef* device, uint16_t startX, uint16_t startY, uint16_t endX,
    uint16_t endY, DisplayTransmissionDirection_TypeDef direction) {

  uint8_t data[4];

  st7796_WriteCommand(device, 0x2a);
  #if (ST7796_IS_PORTRAIT)
    // vertical
    data[0] = startX >> 8; data[1] = startX & 0xff;
    data[2] = endX >> 8; data[3] = endX & 0xff;
  #else
    // horizontal
    data[0] = startY >> 8; data[1] = startY & 0xff;
    data[2] = endY >> 8; data[3] = endY & 0xff;
  #endif
  st7796_WriteData(device, data, 4);

  st7796_WriteCommand(device, 0x2b);
  #if (ST7796_IS_PORTRAIT)
    // vertical
    data[0] = startY >> 8; data[1] = startY & 0xff;
    data[2] = endY >> 8; data[3] = endY & 0xff;
  #else
    // horizontal
    data[0] = startX >> 8; data[1] = startX & 0xff;
    data[2] = endX >> 8; data[3] = endX & 0xff;
  #endif
  st7796_WriteData(device, data, 4);

  switch (direction) {
    case DISPLAY_DIRECTION_READ:
      st7796_WriteCommand(device, 0x2e);
    break;

    case DISPLAY_DIRECTION_WRITE:
      st7796_WriteCommand(device, 0x2c);
    break;

    default:
      Error_Handler();
    break;
  }
}

Display_TypeDef* ST7796_Init(void) {

  __attribute__((section(".dma_buffer_write"), aligned(4))) static uint16_t pixelBuffer[ST7796_PIXEL_BUFFER_SIZE];
  __attribute__((section(".dma_buffer_read"), aligned(4))) static uint16_t backgroundBuffer[ST7796_PIXEL_BUFFER_SIZE];
  static Display_TypeDef displayDevice = {
    .model              = 7796,
    .orientation        = ST7796_ORIENTATION,
    .bus                = &hspi1,
    .pixelBuffer             = pixelBuffer,
    .pixelBufferSize         = ST7796_PIXEL_BUFFER_SIZE,
    .pixelBufferActiveSize   = 0,
    .backgroundBuffer           = backgroundBuffer,
    .backgroundBufferSize       = ST7796_PIXEL_BUFFER_SIZE,
    .backgroundBufferActiveSize = 0,
    .width              = ST7796_DISPLAY_WIDTH,
    .height             = ST7796_DISPLAY_HEIGHT,
  };

  Display_TypeDef* device = &displayDevice;
  SPI_HandleTypeDef* bus = (SPI_HandleTypeDef*)device->bus;

  if (device->lock == DISABLE) device->lock = ENABLE;
  if (bus->Lock == HAL_LOCKED) return device;

  uint8_t initData[16];

  size_t dmaWriteSize = (size_t)((uintptr_t)&__dma_buffer_write_end__ - (uintptr_t)&__dma_buffer_write_start__);
  size_t dmaReadSize = (size_t)((uintptr_t)&__dma_buffer_read_end__ - (uintptr_t)&__dma_buffer_read_start__);

  if ((dmaWriteSize != (2 * ST7796_PIXEL_BUFFER_SIZE)) || (dmaReadSize != (2 * ST7796_PIXEL_BUFFER_SIZE))) return device;

  st7796_Reset();

	HAL_Delay(120);

	st7796_WriteCommand(device, 0x01);             // Software reset
	HAL_Delay(120);

	st7796_WriteCommand(device, 0x11);             // Sleep exit
	HAL_Delay(120);

	st7796_WriteCommand(device, 0xf0);             // Command Set control
  initData[0] = 0xc3;               // - Enable extension command 2 partI
	st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0xf0);             // Command Set control
  initData[0] = 0x96;               // - Enable extension command 2 partII
  st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0x36);             // Memory Data Access Control MX, MY, RGB mode
  initData[0] = ST7796_ORIENTATION;        // - Orientation, RGB
	st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0x3a);             // Interface Pixel Format
  initData[0] = 0x55;               // - Control interface color format set to RGB 565
	st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0xb4);             // Column inversion
  initData[0] = 0x01;               // - 1-dot inversion
	st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0xb6);             // Display Function Control
  initData[0] = 0x80;               // - Bypass
  initData[1] = 0x02;               // - Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
  initData[2] = 0x3b;               // - LCD Drive Line=8*(59+1)
	st7796_WriteData(device, initData, 3);

	st7796_WriteCommand(device, 0xe8);             // Display Output Ctrl Adjust
  initData[0] = 0x40;
  initData[1] = 0x8a;
  initData[2] = 0x00;
  initData[3] = 0x00;
  initData[4] = 0x29;               // - Source eqaulizing period time= 22.5 us
  initData[5] = 0x19;               // - Timing for "Gate start"=25 (Tclk)
  initData[6] = 0xa5;               // - Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
  initData[7] = 0x33;
	st7796_WriteData(device, initData, 8);

	st7796_WriteCommand(device, 0xc1);             // Power control2
  initData[0] = 0x06;               // - VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
	st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0xc2);             // Power control 3
  initData[0] = 0xa7;               // - Source driving current level=low, Gamma driving current level=High
	st7796_WriteData(device, initData, 1);

	st7796_WriteCommand(device, 0xc5);             // VCOM Control
  initData[0] = 0x18;               // - VCOM=0.9
	st7796_WriteData(device, initData, 1);

	HAL_Delay(10);

	st7796_WriteCommand(device, 0xe0);             // Gamma"+"
  initData[0] = 0xf0;
  initData[1] = 0x09;
  initData[2] = 0x0b;
  initData[3] = 0x06;
  initData[4] = 0x04;
  initData[5] = 0x15;
  initData[6] = 0x2f;
  initData[7] = 0x54;
  initData[8] = 0x42;
  initData[9] = 0x3c;
  initData[10] = 0x17;
  initData[11] = 0x14;
  initData[12] = 0x18;
  initData[13] = 0x18;
	st7796_WriteData(device, initData, 14);

	st7796_WriteCommand(device, 0xe1);             // Gamma"-"
  initData[0] = 0xe0;
  initData[1] = 0x09;
  initData[2] = 0x0b;
  initData[3] = 0x06;
  initData[4] = 0x04;
  initData[5] = 0x03;
  initData[6] = 0x2b;
  initData[7] = 0x43;
  initData[8] = 0x42;
  initData[9] = 0x3b;
  initData[10] = 0x16;
  initData[11] = 0x14;
  initData[12] = 0x17;
  initData[13] = 0x1b;
	st7796_WriteData(device, initData, 14);

	st7796_WriteCommand(device, 0x29);             // Display on

  HAL_Delay(10);

  if (Display_Fill(device, DISPLAY_COLOR_BLACK, DISPLAY_LAYER_FRONT) != HAL_OK) return device;

  device->lock = DISABLE;
  return device;
}

HAL_StatusTypeDef Display_Fill(Display_TypeDef* device, uint16_t color, DisplayLayer_TypeDef layer) {
  return Display_FillRectangle(device, 0, 0, device->width, device->height, color, layer);
}

HAL_StatusTypeDef Display_DrawRectangle(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer) {
  if ((width == 0U) || (height == 0U) || (thickness == 0U) || (thickness > width) || (thickness > height)) return HAL_ERROR;

  HAL_StatusTypeDef status = HAL_OK;

  if (Display_DrawVLine(device, x, y, height, thickness, color, layer) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawVLine(device, (x + width - thickness), y, height, thickness, color, layer) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawHLine(device, x, y, width, thickness, color, layer) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawHLine(device, x, (y + height - thickness), width, thickness, color, layer) != HAL_OK) status = HAL_ERROR;

  return status;
}

HAL_StatusTypeDef Display_FillRectangle(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, DisplayLayer_TypeDef layer) {

  if ((device == NULL) || ((layer != DISPLAY_LAYER_FRONT) && (layer != DISPLAY_LAYER_BACKGROUND))) return HAL_ERROR;

  uint16_t windowRight, windowBottom, windowX, windowY;

  #if ST7796_IS_PORTRAIT
    windowX = y;
    windowY = x;
    windowRight = height + windowX;
    windowBottom = width + windowY;
    if (windowRight > device->height) return HAL_ERROR;
    if (windowBottom > device->width) return HAL_ERROR;
  #else
    windowRight = width + x;
    windowBottom = height + y;
    windowX = x;
    windowY = y;
    if (windowRight > device->width) return HAL_ERROR;
    if (windowBottom > device->height) return HAL_ERROR;
  #endif

  st7796_SetWindow(device, windowX, windowY, (windowRight - 1), (windowBottom - 1), DISPLAY_DIRECTION_WRITE);

  /* prepare color & optimize buffer filler */
  uint32_t total = (uint32_t)height * width;
  uint16_t* target = (layer == DISPLAY_LAYER_FRONT) ? device->pixelBuffer : device->backgroundBuffer;
  uint32_t targetSize = (layer == DISPLAY_LAYER_FRONT) ? device->pixelBufferSize : device->backgroundBufferSize;
  if ((target == NULL) || (targetSize == 0U)) return HAL_ERROR;
  uint32_t fillCount = (total > targetSize) ? targetSize : total;
  for (uint32_t i = 0; i < fillCount; i++) {
    target[i] = color;
  }

  device->pixelBufferActiveSize = 0;

  while (total) {
    device->pixelBufferActiveSize = (total > targetSize) ? targetSize : total;
    switch (layer) {
      case DISPLAY_LAYER_FRONT:
        if (st7796_WriteDataDma(device) != HAL_OK) return HAL_ERROR;
        break;

      case DISPLAY_LAYER_BACKGROUND:
        device->backgroundBufferActiveSize = device->pixelBufferActiveSize;
        if (st7796_WriteBackgroundDataDma(device) != HAL_OK) return HAL_ERROR;
        break;

      default:
        return HAL_ERROR;
    }
    total -= device->pixelBufferActiveSize;
  }

  return HAL_OK;
}

HAL_StatusTypeDef Display_FillBackground(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {

  uint32_t total = (uint32_t)width * height;
  uint16_t windowRight = width + x;
  uint16_t windowBottom = height + y;
  if ((total == 0U) || (total > device->backgroundBufferSize)) return HAL_ERROR;
  if (windowRight > device->width) return HAL_ERROR;
  if (windowBottom > device->height) return HAL_ERROR;
  st7796_SetWindow(device, x, y, (windowRight - 1), (windowBottom - 1), DISPLAY_DIRECTION_WRITE);

  device->backgroundBufferActiveSize = total;

  return st7796_WriteBackgroundDataDma(device);
}

HAL_StatusTypeDef Display_ReadRectangle(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {

  uint32_t total = (uint32_t)width * height;
  uint16_t windowRight = width + x;
  uint16_t windowBottom = height + y;
  if ((total == 0U) || (total > device->backgroundBufferSize)) return HAL_ERROR;
  if (windowRight > device->width) return HAL_ERROR;
  if (windowBottom > device->height) return HAL_ERROR;
  st7796_SetWindow(device, x, y, (windowRight - 1), (windowBottom - 1), DISPLAY_DIRECTION_READ);

  device->backgroundBufferActiveSize = total;

  return st7796_ReadDataDma(device);
}

HAL_StatusTypeDef Display_DrawPixel(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t color, DisplayLayer_TypeDef layer) {
  return Display_FillRectangle(device, x, y, 1, 1, color, layer);
}

HAL_StatusTypeDef Display_DrawVLine(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t length, uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer) {
  return Display_FillRectangle(device, x, y, thickness, length, color, layer);
}

HAL_StatusTypeDef Display_DrawHLine(Display_TypeDef* device, uint16_t x, uint16_t y, uint16_t length, uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer) {
  return Display_FillRectangle(device, x, y, length, thickness, color, layer);
}

HAL_StatusTypeDef Display_DrawCircle(Display_TypeDef* device, uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t thickness, uint16_t color, DisplayLayer_TypeDef layer) {

  int16_t x = 0;
  int16_t y = radius;
  int16_t errorTerm = 1 - radius;

  while (x <= y) {
    if (Display_DrawHLine(device, (centerX + x - thickness), (centerY + y), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX + x - thickness), (centerY - y), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX + y - thickness), (centerY + x), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX + y - thickness), (centerY - x), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;

    if (Display_DrawHLine(device, (centerX - x - thickness), (centerY + y), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX - x - thickness), (centerY - y), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX - y - thickness), (centerY + x), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX - y - thickness), (centerY - x), thickness, thickness, color, layer) != HAL_OK) return HAL_ERROR;

    if (errorTerm < 0) {
      errorTerm += 2 * x + 3;
    } else {
      errorTerm += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
  return HAL_OK;
}

HAL_StatusTypeDef Display_FillCircle(Display_TypeDef* device, uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color, DisplayLayer_TypeDef layer) {
  int16_t x = 0;
  int16_t y = radius;
  int16_t errorTerm = 1 - radius;

  while (x <= y) {

    if (Display_DrawHLine(device, (centerX - x - 1), (centerY + y), (2 * x + 1), 1, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX - x - 1), (centerY - y), (2 * x + 1), 1, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX - y - 1), (centerY + x), (2 * y + 1), 1, color, layer) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(device, (centerX - y - 1), (centerY - x), (2 * y + 1), 1, color, layer) != HAL_OK) return HAL_ERROR;

    if (errorTerm < 0) {
      errorTerm += 2 * x + 3;
    } else {
      errorTerm += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
  return HAL_OK;
}

/** @brief Expand one bitmap glyph into the foreground pixel buffer. */
__STATIC_INLINE void st7796_PrepareGlyph(Display_TypeDef* device, Font_TypeDef* font, char symbol, uint32_t totalPixels) {
  if ((symbol < 32) || (symbol > 126)) {
    if (symbol == 176) symbol = 95;
    else symbol = 32;
  }
  symbol -= 32;

  const uint8_t *glyph = font->fontData + (symbol * font->bytesPerGlyph);

  uint32_t bufferIndex = device->pixelBufferActiveSize;

  uint32_t pixelCount = 0;

  for (uint32_t byte = 0; byte < font->bytesPerGlyph; byte++) {
    uint8_t bits = glyph[byte];

    for (uint8_t bit = 0; bit < 8; bit++) {
      if (pixelCount >= totalPixels) break;
      device->pixelBuffer[bufferIndex++] = (bits & 0x01) ? font->color : font->backgroundColor;
      bits >>= 1;
      pixelCount++;
    }
  }
  device->pixelBufferActiveSize = bufferIndex;
}

HAL_StatusTypeDef Display_PrintSymbol(Display_TypeDef* device, uint16_t x, uint16_t y, Font_TypeDef* font, char symbol) {

  uint16_t windowBottom, windowRight, windowX, windowY;

  #if ST7796_IS_PORTRAIT
    windowX = y;
    windowY = x;
    windowRight = windowX + font->height;
    windowBottom = windowY + font->width;
    if (windowRight > device->width || windowBottom > device->height) return (HAL_ERROR);
  #else
    windowX = x;
    windowY = y;
    windowRight = windowX + font->width;
    windowBottom = windowY + font->height;
    if (windowRight > device->width || windowBottom > device->height) return (HAL_ERROR);
  #endif

  st7796_SetWindow(device, windowX, windowY, windowRight - 1, windowBottom - 1, DISPLAY_DIRECTION_WRITE);

  const uint32_t totalPixels = font->width * font->height;

  device->pixelBufferActiveSize = 0;

  st7796_PrepareGlyph(device, font, symbol, totalPixels);

  if (device->pixelBufferActiveSize) {
    if (st7796_WriteDataDma(device) != HAL_OK) return HAL_ERROR;
  }

  return HAL_OK;
}

HAL_StatusTypeDef Display_PrintString(Display_TypeDef *device, uint16_t x, uint16_t y, Font_TypeDef *font, const char *string) {

  if (!string || !font) return HAL_ERROR;

  uint16_t xShift, yShift, windowRight, windowBottom, windowX, windowY;

  #if ST7796_IS_PORTRAIT
    windowBottom = font->width;
    windowRight = font->height;
    windowX = y;
    windowY = x;
    xShift = windowX;
    yShift = windowY;
  #else
    windowBottom = font->height;
    windowRight = font->width;
    windowX = x;
    windowY = y;
    xShift = windowX;
    yShift = windowY;
  #endif

  uint16_t characterCount = 0;

  while ((string[characterCount] != '\0') && (string[characterCount] != '\n') && (characterCount < 64U)) {
    characterCount++;
  }

  if (characterCount == 0U) return HAL_OK;

  uint32_t chunk = ST7796_PIXEL_BUFFER_SIZE / (windowRight * windowBottom);
  if (chunk == 0U) return HAL_ERROR;
  uint32_t totalPixels = windowRight * windowBottom * chunk;

  for (uint8_t i = 1; i <= (characterCount / chunk); i++) {


    #if ST7796_IS_PORTRAIT
      st7796_SetWindow(device, xShift, yShift, (xShift + windowRight - 1), ((chunk * windowBottom) + windowY - 1), DISPLAY_DIRECTION_WRITE);
      yShift += chunk * windowBottom;
      if (yShift > device->width) return HAL_OK;
    #else
      st7796_SetWindow(device, xShift, yShift, (xShift + (chunk * windowRight) - 1), (windowY + windowBottom - 1), DISPLAY_DIRECTION_WRITE);
      xShift += chunk * windowRight;
      if (xShift > device->width) return HAL_OK;
    #endif

    device->pixelBufferActiveSize = 0;
    for (uint8_t j = 0; j < chunk; j++) {
      st7796_PrepareGlyph(device, font, string[(j + (chunk * (i - 1)))], totalPixels);
    }

    if (st7796_WriteDataDma(device) != HAL_OK) return HAL_ERROR;
  }

  uint16_t stringRemainder = characterCount % chunk;
  if (stringRemainder) {

    #if ST7796_IS_PORTRAIT
      st7796_SetWindow(device, xShift, yShift, (xShift + windowRight - 1), ((stringRemainder * windowBottom) + windowY - 1), DISPLAY_DIRECTION_WRITE);
    #else
      st7796_SetWindow(device, xShift, yShift, (xShift + (stringRemainder * windowRight) - 1), (windowY + windowBottom - 1), DISPLAY_DIRECTION_WRITE);
    #endif

    device->pixelBufferActiveSize = 0;
    totalPixels = windowRight * windowBottom * stringRemainder;

    for (uint8_t j = 0; j < stringRemainder; j++) {
      st7796_PrepareGlyph(device, font, string[characterCount - stringRemainder + j], totalPixels);
    }
    if (st7796_WriteDataDma(device) != HAL_OK) return HAL_ERROR;
  }

  return HAL_OK;
}
