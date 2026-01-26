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

extern TouchState_t touch_event;

static __IO uint32_t step = 0;





/////////////////////////////////////////////////////////////////////////////





// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {

  if (screen->Lock == ENABLE) return;
  if (touch_event != TOUCH_ACTIVE) return;

  TouchScreen_Process(touch);
  
  switch (touch->State) {
    case TOUCH_DOWN:
    /* code */
    break;
    
    case TOUCH_UP:
    /* code */
    break;
    
    case TOUCH_HOLD:
    /* code */
    break;
    
    default:
    break;
  }
  
  touch_event = TOUCH_IDLE;
  touch->State = TOUCH_IDLE;


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


  char position[16];
  sprintf(position, "x:%i y:%i\n", touch->Context->X, touch->Context->Y); 
  Display_FillRectangle(screen, 40, 80, (font.Width * 16), font.Height, COLOR_BLACK, FRONT);
  Display_PrintString(screen, 40, 80, &font, position);


  Display_DrawVLine(screen, touch->Context->LastX, 0, DISPLAY_HEIGHT, 2, COLOR_BLACK, FRONT);
  Display_DrawVLine(screen, touch->Context->X, 0, DISPLAY_HEIGHT, 2, COLOR_WHITE, FRONT);
  
  Display_DrawHLine(screen, 0, touch->Context->LastY, DISPLAY_WIDTH, 2, COLOR_BLACK, FRONT);
  Display_DrawHLine(screen, 0, touch->Context->Y, DISPLAY_WIDTH, 2, COLOR_WHITE, FRONT);

  touch->Context->LastX = touch->Context->X;
  touch->Context->LastY = touch->Context->Y;
  

}



