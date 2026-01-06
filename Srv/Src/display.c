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
  
  uint32_t tick = HAL_GetTick();
  
  if (step >= tick) {
    return;
  } else {
    step = tick + SIMPLE_PAUSE;
    
    // Display_Fill(dev, (uint16_t)(step & 0xffff));
    Display_FillRectangle(dev, 300, 100, 20, 80, (uint16_t)(tick & 0xffff));
  }
}



