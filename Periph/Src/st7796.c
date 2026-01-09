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
  #if (ORIENTATION == 0xc0) || (ORIENTATION == 0x00)
    data[0] = x0 >> 8; data[1] = x0 & 0xff;
    data[2] = x1 >> 8; data[3] = x1 & 0xff;
  #endif
  #if (ORIENTATION == 0x40) || (ORIENTATION == 0x80)
    data[0] = y0 >> 8; data[1] = y0 & 0xff;
    data[2] = y1 >> 8; data[3] = y1 & 0xff;
  #endif
  write_data(dev, data, 4);

  write_cmd(dev, 0x2b);
  #if (ORIENTATION == 0xc0) || (ORIENTATION == 0x00)
    data[0] = y0 >> 8; data[1] = y0 & 0xff;
    data[2] = y1 >> 8; data[3] = y1 & 0xff;
  #endif
  #if (ORIENTATION == 0x40) || (ORIENTATION == 0x80)
    data[0] = x0 >> 8; data[1] = x0 & 0xff;
    data[2] = x1 >> 8; data[3] = x1 & 0xff;
  #endif
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
    #if (ORIENTATION == 0xc0) || (ORIENTATION == 0x00)
    .Width      = 320,
    .Height     = 480,
    #endif
    #if (ORIENTATION == 0x80) || (ORIENTATION == 0x40)
    .Width      = 480,
    .Height     = 320,
    #endif
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
 
  HAL_StatusTypeDef status = HAL_OK;

  if (Display_DrawVLine(dev, x, y, (w + b), b, c) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawVLine(dev, (x + h), y, (w + b), b, c) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawHLine(dev, x, y, h, b, c) != HAL_OK) status = HAL_ERROR;
  if (Display_DrawHLine(dev, x, (y + w), h, b, c) != HAL_OK) status = HAL_ERROR;

  return (status);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) {

  uint16_t rw = w + x;
  uint16_t rh = h + y;
  if (rw > dev->Width) return (HAL_ERROR);
  if (rh > dev->Height) return (HAL_ERROR);
  display_set_window(dev, x, y, (rw - 1), (rh - 1));

  /* prepare color & optimize buffer filler */
  uint32_t pcnt = h * w;
  uint32_t ccnt = (pcnt > dev->PixBufSize) ? dev->PixBufSize : pcnt; 
  for (uint32_t i = 0; i < ccnt; i++) {
    dev->PixBuf[i] = c;
  }

  uint32_t total = pcnt * 2;
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
  return Display_FillRectangle(dev, x, y, 1, 1, c);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t l, uint16_t b, uint16_t c) {
  return Display_FillRectangle(dev, x, y, b, l, c);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(Display_TypeDef* dev, uint16_t x, uint16_t y, uint16_t l, uint16_t b, uint16_t c) {
  return Display_FillRectangle(dev, x, y, l, b, c);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawCircle(Display_TypeDef* dev, uint16_t x0, uint16_t y0, uint16_t r, uint16_t b, uint16_t c) {

  int16_t x = 0;
  int16_t y = r;
  int16_t d = 1 - r;

  while (x <= y) {
    Display_DrawHLine(dev, (x0 + x - b), (y0 + y), b, b, c);
    Display_DrawHLine(dev, (x0 + x - b), (y0 - y), b, b, c);
    Display_DrawHLine(dev, (x0 + y - b), (y0 + x), b, b, c);
    Display_DrawHLine(dev, (x0 + y - b), (y0 - x), b, b, c);

    Display_DrawHLine(dev, (x0 - x - b), (y0 + y), b, b, c);
    Display_DrawHLine(dev, (x0 - x - b), (y0 - y), b, b, c);
    Display_DrawHLine(dev, (x0 - y - b), (y0 + x), b, b, c);
    Display_DrawHLine(dev, (x0 - y - b), (y0 - x), b, b, c);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
  return (HAL_OK);
}


// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillCircle(Display_TypeDef* dev, uint16_t x0, uint16_t y0, uint16_t r, uint16_t c) {
  int16_t x = 0;
  int16_t y = r;
  int16_t d = 1 - r;

  while (x <= y) {

    Display_DrawHLine(dev, (x0 - x - 1), (y0 + y), (2 * x + 1), 1, c);
    Display_DrawHLine(dev, (x0 - x - 1), (y0 - y), (2 * x + 1), 1, c);
    Display_DrawHLine(dev, (x0 - y - 1), (y0 + x), (2 * y + 1), 1, c);
    Display_DrawHLine(dev, (x0 - y - 1), (y0 - x), (2 * y + 1), 1, c);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }
  return (HAL_OK);
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_PrintSymbol(Display_TypeDef* dev, uint16_t x, uint16_t y, Font_TypeDef* f, char s) {

  if ((s < 32) || (s > 126)) {
      if (s == 176) s = 95;
      else return (HAL_ERROR);
  } else {
      s -= 32;
  }

  uint16_t rw = x + f->Width;
  uint16_t rh = y + f->Height;
  if (rw > dev->Width || rh > dev->Height) return (HAL_ERROR);

  display_set_window(dev, x, y, rw - 1, rh - 1);

  const uint8_t *glyph = f->Font + (s * f->BytesPerGlif);
  const uint32_t total_pixels = f->Width * f->Height;

  uint32_t pixel_count = 0;
  uint32_t buf_idx = 0;

  for (uint32_t byte = 0; byte < f->BytesPerGlif; byte++) {

    uint8_t bits = glyph[byte];

    for (uint8_t bit = 0; bit < 8; bit++) {

      if (pixel_count >= total_pixels) break;

      dev->PixBuf[buf_idx++] = (bits & 0x01) ? f->Color : f->Bgcolor;
      bits >>= 1;
      pixel_count++;
    }
  }

  if (buf_idx) {
      write_data_dma(dev, dev->PixBuf, buf_idx * 2);
  }

  return HAL_OK;
}


HAL_StatusTypeDef __attribute__((weak)) Display_PrintString(Display_TypeDef *dev, uint16_t x, uint16_t y, Font_TypeDef *f, const char *str) {
    // if (!str || !f) return HAL_ERROR;

    // uint16_t cx = x;
    // uint16_t cy = y;
    // HAL_StatusTypeDef status = HAL_ERROR;

    // const uint16_t fw = f->Width;
    // const uint16_t fh = f->Height;

    // while (*str) {

    //     char c = *str++;

    //     // Handle newline
    //     if (c == '\n') {
    //         cx = x;
    //         cy += fh;
    //         if (cy + fh > dev->Height) break;
    //         continue;
    //     }

    //     // Ignore carriage return
    //     if (c == '\r') {
    //         continue;
    //     }

    //     // Auto-wrap
    //     if (cx + fw > dev->Width) {
    //         cx = x;
    //         cy += fh;
    //         if (cy + fh > dev->Height) break;
    //     }

    //     // Draw character
    //     if (Display_PrintSymbol(dev, cx, cy, f, c) == HAL_OK) {
    //         status = HAL_OK;
    //     }

    //     cx += fw;
    // }

    // return status;



  if (!str || !f) return HAL_ERROR;

  uint16_t lc = 0;
  uint32_t x_shift = x;
  
  while (str[lc++] != '\n') {
    if (lc > 64) break;
  }
  
  uint32_t pc = 1024 / (f->Width * f->Height);
  uint32_t pxc = f->Width * f->Height * pc;
  
  for (uint8_t i = 1; i <= (lc / pc); i++) {
    
    display_set_window(dev, x_shift, y, (x_shift + (pc * f->Width) - 1), (y + f->Height - 1));
    x_shift += pc * f->Width;
    
    // for (uint32_t k = 0; k < pxc; k++) {
      //   dev->PixBuf[k] = f->Color;
      // }
      
      
      // for (uint8_t v = 0; v < f->Height; v++) {
        //   for (uint16_t j = 0; j < pc; j++) {
          //     const uint8_t *glyph = f->Font + (str[j*i] * f->BytesPerGlif);
          
          //     for (uint8_t k = 0; k < f->Width; k++) {
            //       uint8_t bits = glyph[k + (v * f->Height)];
            //       for (uint8_t bit = 0; bit < 8; bit++) {
              //         if (buf_idx >= 1024) break;
              //         dev->PixBuf[buf_idx++] = (bits & 0x01) ? f->Color : f->Bgcolor;
              //         bits >>= 1;
              //       }
              //     }
              //   }
              // }
              
    uint32_t buf_idx = 0;

    for (uint16_t j = 0; j < pc; j++) {
      const uint8_t *glyph = f->Font + (str[(j + ((i - 1) * pc))] * f->BytesPerGlif);

      for (uint8_t h = 0; h < f->Height; h++) {
        for (uint8_t w = 0; w < f->Width; w++) {
          uint8_t bits = glyph[w];
          for (uint8_t bit = 0; bit < 8; bit++) {
            dev->PixBuf[buf_idx++] = (bits & 0x01) ? f->Color : f->Bgcolor;
            bits >>= 1;
          }
        }
        buf_idx += pc * f->Width;
      }
      // buf_idx = i * f->Width;
      __NOP();

    }
  


    write_data_dma(dev, dev->PixBuf, pxc * 2);
  }

  return HAL_OK;
}
