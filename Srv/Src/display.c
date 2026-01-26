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

  // Font_TypeDef font2 = {
  //   .Bgcolor      = COLOR_BLUE,
  //   .Color        = COLOR_LIME,
  //   .Font         = (uint8_t*)&font_dot_5x7,
  //   .Height       = 8,
  //   .Width        = 6,
  //   .BytesPerGlif = 6,
  // };


  // uint16_t color = (uint16_t)(rand() & 0xffff);
  // font.Color = color;
  // font.Bgcolor = ~color;

  static uint16_t ptx; 
  static uint16_t pty; 
  uint16_t tx = touch->State->x;
  uint16_t ty = touch->State->y;
  TouchScreen_MapToDisplay(&tx, &ty, ORIENTATION);


  char position[16];
  sprintf(position, "x:%i y:%i\n", touch->State->x, touch->State->y); 
  Display_FillRectangle(screen, 40, 180, (font.Width * 16), font.Height, COLOR_BLACK, FRONT);
  Display_PrintString(screen, 40, 180, &font, position);

  char position2[16];
  sprintf(position2, "x:%i y:%i\n", tx, ty); 
  Display_FillRectangle(screen, 40, 80, (font.Width * 16), font.Height, COLOR_BLACK, FRONT);
  Display_PrintString(screen, 40, 80, &font, position2);


  Display_DrawVLine(screen, ptx, 0, DISPLAY_HEIGHT, 2, COLOR_BLACK, FRONT);
  Display_DrawVLine(screen, tx, 0, DISPLAY_HEIGHT, 2, COLOR_WHITE, FRONT);
  
  Display_DrawHLine(screen, 0, pty, DISPLAY_WIDTH, 2, COLOR_BLACK, FRONT);
  Display_DrawHLine(screen, 0, ty, DISPLAY_WIDTH, 2, COLOR_WHITE, FRONT);

  ptx = tx;
  pty = ty;

  touch->Phase = TOUCH_IDLE;

}



