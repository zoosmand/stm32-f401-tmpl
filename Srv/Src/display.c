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
static volatile uint32_t step = 0;


/////////////////////////////////////////////////////////////////////////////





// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* dev) {
  
  uint32_t tick = HAL_GetTick();
  
  if (step > tick) {
    return;
  } else {
    step = tick + SIMPLE_PAUSE;
    
    // ST7796_Fill(CLR_RED); // red
    // ST7796_Fill(CLR_GREEN); // green
    // ST7796_Fill(CLR_BLUE); // blue
    // ST7796_Fill(CLR_PURPLE); // purple
    // ST7796_Fill(CLR_SKY); // sky-blue
    // ST7796_Fill(CLR_LIME); // yellow
    
    Display_Fill(dev, (uint16_t)(step & 0xffff));
    DisplayFillRectangle(dev, 40, 120, 10, 65, COLOR_LIME);
  }
}



