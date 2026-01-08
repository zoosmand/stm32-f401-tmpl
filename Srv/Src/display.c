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

static __IO uint32_t step = 0;





/////////////////////////////////////////////////////////////////////////////





// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* dev) {

  if (dev->Lock == ENABLE) return;

  Font_TypeDef font = {
    .Bgcolor      = COLOR_BLUE,
    .Color        = COLOR_LIME,
    .Font         = (uint8_t*)&font_dot_15x21,
    .Height       = 24,
    .Width        = 18,
    .BytesPerGlif = 54,
  };


  uint32_t tick = HAL_GetTick();
  uint16_t color;
  // static uint16_t r_step = 280;
  // static uint16_t pr_step;
  // static bool narrow = true;
  
  if (step >= tick) {
    return;
  } else {
    step = tick + SIMPLE_PAUSE;
    
    // Display_Fill(dev, (uint16_t)(step & 0xffff));
    // Display_FillRectangle(dev, 150, 100, 20, 40, (uint16_t)(tick & 0xffff));
    color = (uint16_t)(rand() & 0xffff);

    // Display_DrawVLine(dev, 100, 0, 300, 2, color);
    // Display_DrawHLine(dev, 0, 150, 460, 2, color);

    // Display_DrawRectangle(dev, 120, 160, 50, 80, 4, color);

    // Display_DrawCircle(dev, pr_step, 230, 30, 2, COLOR_BLACK);
    // pr_step = r_step;
    
    // Display_DrawCircle(dev, r_step, 230, 30, 2, COLOR_LIME);
    
    // if (narrow) {
      
    //   if (r_step >= 480 - (30 + 2)) {
    //     narrow = false;
    //   }
    //   r_step += 2;
      
    // } else {
    //   if (r_step < (30 + 2 + 4)) {
    //     narrow = true;
    //   }
    //   r_step -= 2;
      
    // }
    
    
    // Display_DrawCircle(dev, 280, 230, 10, 3, color);
    // Display_FillCircle(dev, 220, 115, 30, color);


    // Display_PrintSymbol(dev, 140, 80, &font, '3');
    Display_PrintString(dev, 140, 80, &font, "CoroideVO!\n");
  }
}



