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
  HAL_SPI_Transmit((SPI_HandleTypeDef*)dev->Device, &cmd, 1, HAL_MAX_DELAY);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void write_data(Display_TypeDef* dev, const uint8_t *data, uint32_t len) {
  dc_data();
  HAL_SPI_Transmit((SPI_HandleTypeDef*)dev->Device, (uint8_t*)data, len, HAL_MAX_DELAY);
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

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    st7796_dma_busy = false;
  }
}


// --------------------------------------------------------------------------

__STATIC_INLINE void write_data_dma(Display_TypeDef* dev, uint16_t *data, uint32_t len) {
  
  while (st7796_dma_busy);
  
  dc_data();
  st7796_dma_busy = true;
  
  if (HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)dev->Device, (uint8_t*)data, len) != HAL_OK) {
    st7796_dma_busy = false; // important safety
    return;
  }
  while (st7796_dma_busy);
}


// --------------------------------------------------------------------------

__STATIC_INLINE void display_set_window(Display_TypeDef* dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

uint8_t data[4];

  write_cmd(dev, 0x2a);
  data[0] = x0 >> 8; data[1] = x0 & 0xff;
  data[2] = x1 >> 8; data[3] = x1 & 0xff;
  write_data(dev, data, 4);

  write_cmd(dev, 0x2b);
  data[0] = y0 >> 8; data[1] = y0 & 0xff;
  data[2] = y1 >> 8; data[3] = y1 & 0xff;
  write_data(dev, data, 4);

  write_cmd(dev, 0x2c);
}




// --------------------------------------------------------------------------

Display_TypeDef* ST7796_Init(void) {

  __attribute__((section(".dma_buffer"), aligned(4))) static uint16_t pixbuf[PIX_BUF_SZ];
  static Display_TypeDef display_0 = {
    .Model      = 7796,
    .Device     = (uint32_t*)&hspi1,
    .PixBuf     = pixbuf,
    .PixBufSize = PIX_BUF_SZ,
    .Width      = DISPLAY_WIDTH,
    .Height     = DISPLAY_HEIGHT,
  };

  Display_TypeDef* dev = &display_0;

  if (dev->Lock == DISABLE) dev->Lock = ENABLE;

  uint8_t initData[16];

  size_t dma_size = (size_t)((uintptr_t)&__dma_buffer_end__ - (uintptr_t)&__dma_buffer_start__);

  // optional sanity check
  if (dma_size != 2048) return dev;

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
  // initData[0] = 0x48;            // - X-Mirror, Top-Left to right-Buttom, RGB
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
  if (Display_Fill(dev, COLOR_BLACK) != HAL_OK) return dev;

  dev->Lock = DISABLE;
  return dev;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_Fill(Display_TypeDef* dev, uint16_t c) {

  return Display_FillRectangle(dev, 0, 0, dev->Width, dev->Height, c);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawRectangle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t b, uint16_t c) {
  
  display_set_window(dev, x, y, (w - x - 1), (h - y - 1));

  /* prepare color */
  for (uint32_t i = 0; i < dev->PixBufSize; i++) {
    dev->PixBuf[i] = c;
  }

  uint32_t total = dev->Width * dev->Height * 2;
  uint32_t chunk = 0;
  
  while (total) {
    chunk = (total > dev->PixBufSize) ? dev->PixBufSize : total;
    write_data_dma(dev, dev->PixBuf, chunk);
    total -= chunk;
  }

  return (HAL_OK);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) {

  uint16_t rw, rh;
  #if (ORIENTATION == 0xc0) || (ORIENTATION == 0x00)
    rw = w + x;
    rh = h + y;
    if (rw > dev->Width) return (HAL_ERROR);
    if (rh > dev->Height) return (HAL_ERROR);
    display_set_window(dev, x, y, (rw - 1), (rh - 1));
  #endif
  #if (ORIENTATION == 0x40) || (ORIENTATION == 0x80)
    rw = w + x;
    rh = h + y;
    if (rw > dev->Width) return (HAL_ERROR);
    if (rh > dev->Height) return (HAL_ERROR);
    display_set_window(dev, y, x, (rh - 1), (rw - 1));
  #endif

  /* prepare color */
  for (uint32_t i = 0; i < dev->PixBufSize; i++) {
    dev->PixBuf[i] = c;
  }

  uint32_t total = dev->Width * dev->Height * 2;
  uint32_t chunk = 0;
  
  while (total) {
    chunk = (total > dev->PixBufSize) ? dev->PixBufSize : total;
    write_data_dma(dev, dev->PixBuf, chunk);
    total -= chunk;
  }

  return (HAL_OK);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawPixel(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t c) {

  __NOP();
  return (HAL_OK);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t l, uint16_t b, uint16_t c) {

  __NOP();
  return (HAL_OK);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t l, uint16_t b, uint16_t c) {

  __NOP();
  return (HAL_OK);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawCircle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t r, uint16_t b, uint16_t c) {
  
  __NOP();
  return (HAL_OK);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillCircle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t r, uint16_t c) {
  
  __NOP();
  return (HAL_OK);
}



