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


volatile bool st7796_dma_busy = false;


__STATIC_INLINE void dc_cmd(void){ HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET); }
__STATIC_INLINE void dc_data(void){ HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET); }


#define PIX_BUF_SZ 1024  // words (1024 pixels)
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 480
__attribute__((section(".dma_buffer"), aligned(4))) static uint16_t pixbuf[PIX_BUF_SZ];



// --------------------------------------------------------------------------

__STATIC_INLINE void write_cmd(uint8_t cmd) {
  dc_cmd();
  HAL_SPI_Transmit(&ST7796_SPI, &cmd, 1, HAL_MAX_DELAY);
}




// --------------------------------------------------------------------------

__STATIC_INLINE void write_data(const uint8_t *data, uint32_t len) {
  dc_data();
  HAL_SPI_Transmit(&ST7796_SPI, (uint8_t*)data, len, HAL_MAX_DELAY);
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
  if (hspi == &ST7796_SPI) {
    st7796_dma_busy = false;
    // switch to 8-bit bandwidth
    ST7796_SPI.Instance->CR1 &= ~SPI_CR1_DFF; 
  }
}



// --------------------------------------------------------------------------

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  if (hspi == &ST7796_SPI) {
    st7796_dma_busy = false;
    // switch to 8-bit bandwidth
    ST7796_SPI.Instance->CR1 &= ~SPI_CR1_DFF; 
  }
}



// --------------------------------------------------------------------------

__STATIC_INLINE void write_data_dma(uint16_t *data, uint32_t len) {

  while (st7796_dma_busy);

  dc_data();
  st7796_dma_busy = true;
  // switch to 16-bit bandwidth
  ST7796_SPI.Instance->CR1 |= SPI_CR1_DFF; 
  
  if (HAL_SPI_Transmit_DMA(&ST7796_SPI, (uint8_t*)data, len) != HAL_OK) {
    st7796_dma_busy = false; // important safety
    return;
  }
  while (st7796_dma_busy);
}





// --------------------------------------------------------------------------

__STATIC_INLINE void display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

uint8_t data[4];

  write_cmd(0x2a);
  data[0] = x0 >> 8; data[1] = x0 & 0xff;
  data[2] = x1 >> 8; data[3] = x1 & 0xff;
  write_data(data, 4);

  write_cmd(0x2b);
  data[0] = y0 >> 8; data[1] = y0 & 0xff;
  data[2] = y1 >> 8; data[3] = y1 & 0xff;
  write_data(data, 4);

  write_cmd(0x2c);
}




// --------------------------------------------------------------------------

__STATIC_INLINE void display_prepare_color(uint16_t color) {
  for (uint32_t i = 0; i < PIX_BUF_SZ; i++) {
    pixbuf[i] = color;
  }
}




// --------------------------------------------------------------------------

HAL_StatusTypeDef ST7796_Init(void) {

  uint8_t initData[16];

  size_t dma_size = (size_t)((uintptr_t)&__dma_buffer_end__ - (uintptr_t)&__dma_buffer_start__);

  // optional sanity check
  if (dma_size != 2048) {
    return (HAL_ERROR);
  }

  display_reset();

	HAL_Delay(120);

	write_cmd(0x01);                  // Software reset
	HAL_Delay(120);

	write_cmd(0x11);                  // Sleep exit                                            
	HAL_Delay(120);

	write_cmd(0xf0);                  // Command Set control
  initData[0] = 0xc3;               // - Enable extension command 2 partI
	write_data(initData, 1);      
	
	write_cmd(0xf0);                  // Command Set control                                 
  initData[0] = 0x96;               // - Enable extension command 2 partII
  write_data(initData, 1);    
	
	write_cmd(0x36);                  // Memory Data Access Control MX, MY, RGB mode                                    
  initData[0] = 0x48;               // - X-Mirror, Top-Left to right-Buttom, RGB
	write_data(initData, 1);      
	
	write_cmd(0x3a);                  // Interface Pixel Format                                    
  initData[0] = 0x55;               // - Control interface color format set to 16
	write_data(initData, 1);    
	
	write_cmd(0xb4);                  // Column inversion 
  initData[0] = 0x01;               // - 1-dot inversion
	write_data(initData, 1);

	write_cmd(0xb6);                  // Display Function Control
  initData[0] = 0x80;               // - Bypass
  initData[1] = 0x02;               // - Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
  initData[2] = 0x3b;               // - LCD Drive Line=8*(59+1)
	write_data(initData, 3);    

	write_cmd(0xe8);                  // Display Output Ctrl Adjust
  initData[0] = 0x40;
  initData[1] = 0x8a;
  initData[2] = 0x00;
  initData[3] = 0x00;
  initData[4] = 0x29;               // - Source eqaulizing period time= 22.5 us
  initData[5] = 0x19;               // - Timing for "Gate start"=25 (Tclk)
  initData[6] = 0xa5;               // - Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
  initData[7] = 0x33;
	write_data(initData, 8);
	
	write_cmd(0xc1);                  // Power control2                          
  initData[0] = 0x06;               // - VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
	write_data(initData, 1);    
	 
	write_cmd(0xc2);                  // Power control 3                                      
  initData[0] = 0xa7;               // - Source driving current level=low, Gamma driving current level=High
	write_data(initData, 1);    
	 
	write_cmd(0xc5);                  // VCOM Control
  initData[0] = 0x18;               // - VCOM=0.9
	write_data(initData, 1);    

	HAL_Delay(120);
	
	write_cmd(0xe0);                  // Gamma"+"                                             
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
	write_data(initData, 14);
	 
	write_cmd(0xe1);                  // Gamma"-"                                             
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
	write_data(initData, 14);

  HAL_Delay(120);
	
	write_cmd(0xf0);                  // Command Set control            
  initData[0] = 0x3c;               // - Disable extension command 2 partI
	write_data(initData, 1);    

	write_cmd(0xf0);                  // Command Set control                                 
  initData[0] = 0x69;               // - Disable extension command 2 partII
	write_data(initData, 1);    

  HAL_Delay(120);
  
	write_cmd(0x29);                  // Display on                                          	
  
  HAL_Delay(10);

  return (ST7796_Fill(0x0000));
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef ST7796_Fill(uint16_t color) {
  display_set_window(0, 0, (DISPLAY_WIDTH - 1), (DISPLAY_HEIGHT - 1));

  display_prepare_color(color);
  
  uint32_t total = DISPLAY_WIDTH * DISPLAY_HEIGHT;
  while (total) {
    uint32_t chunk = (total > PIX_BUF_SZ) ? PIX_BUF_SZ : total;
    write_data_dma(pixbuf, chunk);
    total -= chunk;
  }

  return (HAL_OK);
}
