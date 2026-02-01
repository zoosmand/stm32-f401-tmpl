/**
  ******************************************************************************
  * @file           : st7796.c
  * @brief          : This file contain ST7796 TFT driver code.
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

#include "st7796.h"


__IO bool st7796_dma_busy = false;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;



// --------------------------------------------------------------------------

__STATIC_INLINE void dc_cmd(void) {
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void dc_data(void) {
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void write_cmd(Display_TypeDef* dev, uint8_t cmd) {
  dc_cmd();
  HAL_SPI_Transmit((SPI_HandleTypeDef*)dev->Bus, &cmd, 1, HAL_MAX_DELAY);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void write_data(Display_TypeDef* dev, const uint8_t *data, uint32_t len) {
  dc_data();
  HAL_SPI_Transmit((SPI_HandleTypeDef*)dev->Bus, (uint8_t*)data, len, HAL_MAX_DELAY);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void display_reset(void) {
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120);
}


// --------------------------------------------------------------------------

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796_dma_busy = false;
  }
}


// --------------------------------------------------------------------------

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796_dma_busy = false;
  }
}


// --------------------------------------------------------------------------

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796_dma_busy = false;
  }
}


// --------------------------------------------------------------------------

__STATIC_INLINE void write_data_dma(Display_TypeDef* dev) {
  
  while (st7796_dma_busy);
  
  dc_data();
  st7796_dma_busy = true;
  
  if (HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)dev->Bus, (uint8_t*)dev->PixBuf, (dev->PixBufActiveSize * 2)) != HAL_OK) {
    st7796_dma_busy = false; // important safety
    return;
  }
  while (st7796_dma_busy);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void write_backgoung_data_dma(Display_TypeDef* dev) {
  
  while (st7796_dma_busy);
  
  dc_data();
  st7796_dma_busy = true;
  
  if (HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)dev->Bus, (uint8_t*)dev->PixBufBg, (dev->PixBufBgActiveSize * 2)) != HAL_OK) {
    st7796_dma_busy = false; // important safety
    return;
  }
  while (st7796_dma_busy);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void read_data_dma(Display_TypeDef* dev) {
  
  while (st7796_dma_busy);

  uint8_t dummy = 0;
  SPI_HandleTypeDef* bus = (SPI_HandleTypeDef*)dev->Bus;

  hdma_spi1_tx.Init.MemInc = DMA_MINC_DISABLE;
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) Error_Handler();

  
  dc_data();
  st7796_dma_busy = true;
  
  if (HAL_SPI_TransmitReceive_DMA(bus, &dummy, (uint8_t*)dev->PixBufBg, (dev->PixBufBgActiveSize *2)) != HAL_OK) {
    st7796_dma_busy = false; // important safety
    return;
  }
  while (st7796_dma_busy);
  hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) Error_Handler();
}


// --------------------------------------------------------------------------

__STATIC_INLINE void display_set_window(Display_TypeDef* dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, TrasmissionDirection_t dir) {

  uint8_t data[4];

  write_cmd(dev, 0x2a);
  #if (DISPLAY_POSITION)
    // vertical
    data[0] = x0 >> 8; data[1] = x0 & 0xff;
    data[2] = x1 >> 8; data[3] = x1 & 0xff;
  #else
    // horizontal
    data[0] = y0 >> 8; data[1] = y0 & 0xff;
    data[2] = y1 >> 8; data[3] = y1 & 0xff;
  #endif
  write_data(dev, data, 4);

  write_cmd(dev, 0x2b);
  #if (DISPLAY_POSITION)
    // vertical
    data[0] = y0 >> 8; data[1] = y0 & 0xff;
    data[2] = y1 >> 8; data[3] = y1 & 0xff;
  #else
    // horizontal
    data[0] = x0 >> 8; data[1] = x0 & 0xff;
    data[2] = x1 >> 8; data[3] = x1 & 0xff;
  #endif
  write_data(dev, data, 4);

  switch (dir) {
    case READ:
      write_cmd(dev, 0x2e);
    break;

    case WRITE:
      write_cmd(dev, 0x2c);
    break;
  
    default:
      Error_Handler();
    break;
  }
}




// --------------------------------------------------------------------------

Display_TypeDef* ST7796_Init(void) {

  __attribute__((section(".dma_buffer_write"), aligned(4))) static uint16_t pixbuf[PIX_BUF_SZ];
  __attribute__((section(".dma_buffer_read"), aligned(4))) static uint16_t pixbuf_bg[PIX_BUF_SZ];
  static Display_TypeDef display_0 = {
    .Model              = 7796,
    .Orientation        = ORIENTATION,
    .Bus                = (uint32_t*)&hspi1,
    .PixBuf             = pixbuf,
    .PixBufSize         = PIX_BUF_SZ,
    .PixBufActiveSize   = 0,
    .PixBufBg           = pixbuf_bg,
    .PixBufBgSize       = PIX_BUF_SZ,
    .PixBufBgActiveSize = 0,
    .Width              = DISPLAY_WIDTH,
    .Height             = DISPLAY_HEIGHT,
  };

  Display_TypeDef* dev = &display_0;
  SPI_HandleTypeDef* bus = (SPI_HandleTypeDef*)dev->Bus;

  if (dev->Lock == DISABLE) dev->Lock = ENABLE;
  if (bus->Lock == HAL_LOCKED) return dev;

  uint8_t initData[16];

  size_t dma_write_size = (size_t)((uintptr_t)&__dma_buffer_write_end__ - (uintptr_t)&__dma_buffer_write_start__);
  size_t dma_read_size = (size_t)((uintptr_t)&__dma_buffer_write_end__ - (uintptr_t)&__dma_buffer_write_start__);

  // optional sanity check
  if ((dma_write_size != (2 * PIX_BUF_SZ)) | (dma_read_size != (2 * PIX_BUF_SZ))) return dev;

  // initialization beginning
  display_reset();

	HAL_Delay(120);

	write_cmd(dev, 0x01);             // Software reset
	HAL_Delay(120);

	write_cmd(dev, 0x11);             // Sleep exit                                            
	HAL_Delay(120);

	write_cmd(dev, 0xf0);             // Command Set control
  initData[0] = 0xc3;               // - Enable extension command 2 partI
	write_data(dev, initData, 1);      
	
	write_cmd(dev, 0xf0);             // Command Set control                                 
  initData[0] = 0x96;               // - Enable extension command 2 partII
  write_data(dev, initData, 1);    
	
	write_cmd(dev, 0x36);             // Memory Data Access Control MX, MY, RGB mode                                    
  initData[0] = ORIENTATION;        // - Orientation, RGB
	write_data(dev, initData, 1);      
	
	write_cmd(dev, 0x3a);             // Interface Pixel Format                                    
  initData[0] = 0x55;               // - Control interface color format set to RGB 565
	write_data(dev, initData, 1);    
	
	write_cmd(dev, 0xb4);             // Column inversion 
  initData[0] = 0x01;               // - 1-dot inversion
	write_data(dev, initData, 1);

	write_cmd(dev, 0xb6);             // Display Function Control
  initData[0] = 0x80;               // - Bypass
  initData[1] = 0x02;               // - Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
  initData[2] = 0x3b;               // - LCD Drive Line=8*(59+1)
	write_data(dev, initData, 3);    

	write_cmd(dev, 0xe8);             // Display Output Ctrl Adjust
  initData[0] = 0x40;
  initData[1] = 0x8a;
  initData[2] = 0x00;
  initData[3] = 0x00;
  initData[4] = 0x29;               // - Source eqaulizing period time= 22.5 us
  initData[5] = 0x19;               // - Timing for "Gate start"=25 (Tclk)
  initData[6] = 0xa5;               // - Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
  initData[7] = 0x33;
	write_data(dev, initData, 8);
	
	write_cmd(dev, 0xc1);             // Power control2                          
  initData[0] = 0x06;               // - VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
	write_data(dev, initData, 1);    
	 
	write_cmd(dev, 0xc2);             // Power control 3                                      
  initData[0] = 0xa7;               // - Source driving current level=low, Gamma driving current level=High
	write_data(dev, initData, 1);    
	 
	write_cmd(dev, 0xc5);             // VCOM Control
  initData[0] = 0x18;               // - VCOM=0.9
	write_data(dev, initData, 1);    

	HAL_Delay(10);
	
	write_cmd(dev, 0xe0);             // Gamma"+"                                             
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
	write_data(dev, initData, 14);
	 
	write_cmd(dev, 0xe1);             // Gamma"-"                                             
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
	write_data(dev, initData, 14);

	write_cmd(dev, 0x29);             // Display on                                          	
  
  HAL_Delay(10);

  // clear display
  if (Display_Fill(dev, COLOR_BLACK, FRONT) != HAL_OK) return dev;

  dev->Lock = DISABLE;
  return dev;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_Fill(Display_TypeDef* dev, uint16_t c, ImageLayer_t l) {
  return Display_FillRectangle(dev, 0, 0, dev->Width, dev->Height, c, l);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawRectangle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t b, uint16_t c, ImageLayer_t l) {
 
  HAL_StatusTypeDef status = HAL_OK;

  #if ORIENTATION
  #else
  #endif

  if (Display_DrawVLine(dev, x, y, (w + b), b, c, l) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawVLine(dev, (x + h), y, (w + b), b, c, l) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawHLine(dev, x, y, h, b, c, l) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawHLine(dev, x, (y + w), h, b, c, l) != HAL_OK) status = HAL_ERROR;

  return status;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c, ImageLayer_t l) {

  uint16_t rw, rh, rx, ry;

  #if DISPLAY_POSITION
    rx = y;
    ry = x;
    rw = h + rx;
    rh = w + ry;
    if (rw > dev->Height) return HAL_ERROR;
    if (rh > dev->Width) return HAL_ERROR;
  #else
    rw = w + x;
    rh = h + y;
    rx = x;
    ry = y;
    if (rw > dev->Width) return HAL_ERROR;
    if (rh > dev->Height) return HAL_ERROR;
  #endif

  display_set_window(dev, rx, ry, (rw - 1), (rh - 1), WRITE);

  /* prepare color & optimize buffer filler */
  uint32_t total = h * w;
  uint32_t ccnt = (total > dev->PixBufSize) ? dev->PixBufSize : total; 
  for (uint32_t i = 0; i < ccnt; i++) {
    dev->PixBuf[i] = c;
  }

  dev->PixBufActiveSize = 0;

  while (total) {
    dev->PixBufActiveSize = (total > dev->PixBufSize) ? dev->PixBufSize : total;
    switch (l) {
      case FRONT:
        write_data_dma(dev);
        break;

      case BACK:
        dev->PixBufBgActiveSize = dev->PixBufActiveSize;
        write_backgoung_data_dma(dev);
        break;
      
      default:
        Error_Handler();
        break;
    }
    total -= dev->PixBufActiveSize;
  }

  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillBackground(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

  uint16_t rw = w + x;
  uint16_t rh = h + y;
  if (rw > dev->Width) return HAL_ERROR;
  if (rh > dev->Height) return HAL_ERROR;
  display_set_window(dev, x, y, (rw - 1), (rh - 1), WRITE);

  write_data_dma(dev);

  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_ReadRectangle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

  uint16_t rw = w + x;
  uint16_t rh = h + y;
  if (rw > dev->Width) return HAL_ERROR;
  if (rh > dev->Height) return HAL_ERROR;
  display_set_window(dev, x, y, (rw - 1), (rh - 1), READ);

  dev->PixBufBgActiveSize = dev->PixBufActiveSize;
  
  read_data_dma(dev);

  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawPixel(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t c, ImageLayer_t layer) {
  return Display_FillRectangle(dev, x, y, 1, 1, c, layer);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t l, uint16_t b, uint16_t c, ImageLayer_t layer) {
  return Display_FillRectangle(dev, x, y, b, l, c, layer);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t l, uint16_t b, uint16_t c, ImageLayer_t layer) {
  return Display_FillRectangle(dev, x, y, l, b, c, layer);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawCircle(Display_TypeDef* dev, uint16_t x0, uint16_t y0, uint16_t r, uint16_t b, uint16_t c, ImageLayer_t l) {

  int16_t x = 0;
  int16_t y = r;
  int16_t d = 1 - r;

  while (x <= y) {
    if (Display_DrawHLine(dev, (x0 + x - b), (y0 + y), b, b, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 + x - b), (y0 - y), b, b, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 + y - b), (y0 + x), b, b, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 + y - b), (y0 - x), b, b, c, l) != HAL_OK) return HAL_ERROR;

    if (Display_DrawHLine(dev, (x0 - x - b), (y0 + y), b, b, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 - x - b), (y0 - y), b, b, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 - y - b), (y0 + x), b, b, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 - y - b), (y0 - x), b, b, c, l) != HAL_OK) return HAL_ERROR;

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
  return HAL_OK;
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillCircle(Display_TypeDef* dev, uint16_t x0, uint16_t y0, uint16_t r, uint16_t c, ImageLayer_t l) {
  int16_t x = 0;
  int16_t y = r;
  int16_t d = 1 - r;

  while (x <= y) {

    if (Display_DrawHLine(dev, (x0 - x - 1), (y0 + y), (2 * x + 1), 1, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 - x - 1), (y0 - y), (2 * x + 1), 1, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 - y - 1), (y0 + x), (2 * y + 1), 1, c, l) != HAL_OK) return HAL_ERROR;
    if (Display_DrawHLine(dev, (x0 - y - 1), (y0 - x), (2 * y + 1), 1, c, l) != HAL_OK) return HAL_ERROR;

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
  return HAL_OK;
}



// --------------------------------------------------------------------------

__STATIC_INLINE void prepare_glyph(Display_TypeDef* dev, Font_TypeDef* f, char ch, uint32_t tp) {

  // shift the glyph index
  if ((ch < 32) || (ch > 126)) {
    if (ch == 176) ch = 95;
    else ch = 32;
  }
  ch -= 32;

  const uint8_t *glyph = f->Font + (ch * f->BytesPerGlif);

  uint32_t bi = dev->PixBufActiveSize;

  uint32_t pixel_count = 0;

  for (uint32_t byte = 0; byte < f->BytesPerGlif; byte++) {
    uint8_t bits = glyph[byte];

    for (uint8_t bit = 0; bit < 8; bit++) {
      if (pixel_count >= tp) break;
      dev->PixBuf[bi++] = (bits & 0x01) ? f->Color : f->Bgcolor;
      bits >>= 1;
      pixel_count++;
    }
  }
  dev->PixBufActiveSize = bi;
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_PrintSymbol(Display_TypeDef* dev, uint16_t x, uint16_t y, Font_TypeDef* f, char ch) {

  uint16_t rh, rw, rx, ry;

  #if DISPLAY_POSITION
    rx = y;
    ry = x;
    rw = rx + f->Height;
    rh = ry + f->Width;
    if (rw > dev->Width || rh > dev->Height) return (HAL_ERROR);
  #else
    rx = x;
    ry = y;
    rw = rx + f->Width;
    rh = ry + f->Height;
    if (rw > dev->Width || rh > dev->Height) return (HAL_ERROR);
  #endif
    
  display_set_window(dev, rx, ry, rw - 1, rh - 1, WRITE);
  
  const uint32_t total_pixels = f->Width * f->Height;

  dev->PixBufActiveSize = 0;

  prepare_glyph(dev, f, ch, total_pixels);

  if (dev->PixBufActiveSize) {
      write_data_dma(dev);
  }

  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_PrintString(Display_TypeDef *dev, uint16_t x, uint16_t y, Font_TypeDef *f, const char *str) {

  if (!str || !f) return HAL_ERROR;

  uint16_t x_shift, y_shift, rw, rh, rx, ry;

  #if DISPLAY_POSITION
    rh = f->Width;
    rw = f->Height;
    rx = y;
    ry = x;
    x_shift = rx;
    y_shift = ry;
  #else
    rh = f->Height;
    rw = f->Width;
    rx = x;
    ry = y;
    x_shift = rx;
  #endif

  uint16_t char_count = 0;

  while (str[char_count++] != '\n') {
    if (char_count > 64) break;
  }
  char_count--; // cut 0x0a
  
  uint32_t chunk = PIX_BUF_SZ / (rw * rh);
  uint32_t total_pixels = rw * rh * chunk;
  
  for (uint8_t i = 1; i <= (char_count / chunk); i++) {

    
    #if DISPLAY_POSITION
      display_set_window(dev, x_shift, y_shift, (x_shift + rw - 1), ((chunk * rh) + y_shift - 1), WRITE);
      // display_set_window(dev, x_shift, y_shift, (x_shift + (chunk * rw) - 1), (ry + rh - 1), WRITE);
      x_shift += chunk * rw;
      if (x_shift > rw) return (HAL_OK);
    #else
      display_set_window(dev, x_shift, y_shift, (x_shift + (chunk * rw) - 1), (ry + rh - 1), WRITE);
      x_shift += chunk * rw;
      if (x_shift > rw) return (HAL_OK);
    #endif

    dev->PixBufActiveSize = 0;
    for (uint8_t j = 0; j < chunk; j++) {
      prepare_glyph(dev, f, str[(j + (chunk * (i - 1)))], total_pixels);
    }

    write_data_dma(dev);
  }

  // print the rest of the string
  uint16_t str_rest = char_count % chunk;
  if (str_rest) {

    #if DISPLAY_POSITION
      display_set_window(dev, x_shift, y_shift, (x_shift + rw - 1), ((str_rest * rh) + y_shift - 1), WRITE);
    #else
      display_set_window(dev, x_shift, y_shift, (x_shift + (str_rest * rw) - 1), (ry + rh - 1), WRITE);
    #endif

    dev->PixBufActiveSize = 0;
    total_pixels = rw * rh * str_rest;

    for (uint8_t j = 0; j < str_rest; j++) {
      prepare_glyph(dev, f, str[char_count - str_rest + j], total_pixels);
    }
    write_data_dma(dev);
  }

  return HAL_OK;
}

