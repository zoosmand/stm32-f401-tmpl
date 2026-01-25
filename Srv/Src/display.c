/**
  ******************************************************************************
  * @file           : display.c
  * @brief          : This file contain display routones code.
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

#include "display.h"



#define SIMPLE_PAUSE 1000U;

extern uint8_t touch_event;

static __IO uint32_t step = 0;





/////////////////////////////////////////////////////////////////////////////





// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {

  if (screen->Lock == ENABLE) return;

  switch (touch->Phase){
    case TOUCH_DISABLED:
      return;

    case TOUCH_IDLE:
      if (!touch_event) return;
      touch_event = 0;
      TouchScrean_Read(touch);
      __NOP();
      break;
      
    default:
      touch_event = 0;
      return;
  }


  Font_TypeDef font = {
    .Bgcolor      = COLOR_BLUE,
    .Color        = COLOR_LIME,
    .Font         = (uint8_t*)&font_dot_20x28,
    .Height       = 32,
    .Width        = 24,
    .BytesPerGlif = 96,
  };

  Font_TypeDef font2 = {
    .Bgcolor      = COLOR_BLUE,
    .Color        = COLOR_LIME,
    .Font         = (uint8_t*)&font_dot_5x7,
    .Height       = 8,
    .Width        = 6,
    .BytesPerGlif = 6,
  };


  uint16_t color = (uint16_t)(rand() & 0xffff);

  Display_DrawVLine(screen, 100, 0, 320, 2, COLOR_WHITE);
  Display_DrawHLine(screen, 0, 150, 480, 2, COLOR_WHITE);

  font.Color = color;
  font.Bgcolor = ~color;

  Display_PrintString(screen, 40, 180, &font, "CoroiscreO!8672854\n");
  Display_PrintString(screen, 40, 86, &font2, "1234567890123456789012345678901234567890123456789012345678901234567890\n");

  touch->Phase = TOUCH_IDLE;

}



