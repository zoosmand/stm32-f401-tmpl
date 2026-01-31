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

extern TouchState_t touch_activated_flag;

static __IO uint32_t step = 0;





/////////////////////////////////////////////////////////////////////////////


static void on_down(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
}



static void on_up(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  // Font_TypeDef font = {
  //   .Bgcolor      = COLOR_BLUE,
  //   .Color        = COLOR_LIME,
  //   .Font         = (uint8_t*)&font_dot_20x28,
  //   .Height       = 32,
  //   .Width        = 24,
  //   .BytesPerGlif = 96,
  // };
  Font_TypeDef font = {
    .Bgcolor      = COLOR_BLUE,
    .Color        = COLOR_LIME,
    .Font         = (uint8_t*)&font_dot_5x7,
    .Height       = 8,
    .Width        = 6,
    .BytesPerGlif = 6,
  };

  Display_PrintSymbol(screen, 100, 100, &font, 'H');

  // Display_DrawVLine(screen, touch->Context->LastX, 0, DISPLAY_HEIGHT, 2, COLOR_BLACK, FRONT);
  // Display_DrawHLine(screen, 0, touch->Context->LastY, DISPLAY_WIDTH, 2, COLOR_BLACK, FRONT);

  // char position[16];
  // sprintf(position, "x:%i y:%i\n", touch->Context->X, touch->Context->Y); 
  // Display_FillRectangle(screen, 40, 80, (font.Width * 16), font.Height, COLOR_BLACK, FRONT);
  // Display_PrintString(screen, 40, 80, &font, position);

  // Display_DrawVLine(screen, touch->Context->X, 0, DISPLAY_HEIGHT, 2, COLOR_WHITE, FRONT);
  // Display_DrawHLine(screen, 0, touch->Context->Y, DISPLAY_WIDTH, 2, COLOR_WHITE, FRONT);

  // touch->Context->LastX = touch->Context->X;
  // touch->Context->LastY = touch->Context->Y;

}

static void on_move(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
}


static void on_hold(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
}




// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {

  if (screen->Lock == ENABLE) return;
  if (touch_activated_flag != TOUCH_ACTIVE) return;

  TouchScreen_Process(touch);

  switch (touch->Event) {
    case TOUCH_ON_DOWN:
      on_down(screen, touch);
      break;
    
    case TOUCH_ON_UP:
      on_up(screen, touch);
      break;
    
    case TOUCH_ON_HOLD:
      on_hold(screen, touch);
      break;
    
    case TOUCH_ON_MOVE:
      on_move(screen, touch);
      break;
    
    case TOUCH_ON_IDLE:
      default:
      __NOP();
      break;
  }
}



