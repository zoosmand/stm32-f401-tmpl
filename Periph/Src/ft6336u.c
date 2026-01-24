/**
  ******************************************************************************
  * @file           : ft6336u.c
  * @brief          : This file contain Touchscreen controller FT6336U driver
  *                   code.
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

#include "ft6336u.h"




TouchScreen_TypeDef* FT6336U_Init(I2C_HandleTypeDef *hi2c) {
  uint8_t dummy;
  static TouchState_TypeDef touch_0_state = {};
  static TouchScreen_TypeDef touch_0 = {
    .Phase    = TOUCH_DISABLED,
    .State    = &touch_0_state,
    .Callback = NULL,
  };

  TouchScreen_TypeDef* dev = &touch_0;

  if (HAL_I2C_Mem_Read(
      hi2c,
      FT6336_ADDR,
      0x00,
      I2C_MEMADD_SIZE_8BIT,
      &dummy,
      1,
      HAL_MAX_DELAY
  ) != HAL_OK) return NULL;

  touch_0.Phase = TOUCH_BLOCKED;
  return dev;
}


HAL_StatusTypeDef TC_Read(TouchState_TypeDef *ts) {

    uint8_t buf[7];

    if (HAL_I2C_Mem_Read(
            &hi2c1,
            FT6336_ADDR,
            0x02,
            I2C_MEMADD_SIZE_8BIT,
            buf,
            sizeof(buf),
            HAL_MAX_DELAY
        ) != HAL_OK) {
        return HAL_ERROR;
    }

    ts->touches = buf[0] & 0x0F;

    if (ts->touches == 0) {
        return HAL_OK;
    }

    ts->event = (buf[1] >> 6) & 0x03;

    ts->x = ((buf[1] & 0x0F) << 8) | buf[2];
    ts->y = ((buf[3] & 0x0F) << 8) | buf[4];

    return HAL_OK;
}



void TC_MapToDisplay(
    uint16_t *x,
    uint16_t *y,
    uint16_t w,
    uint16_t h
) {
    uint16_t tx = *x;
    uint16_t ty = *y;

#if ORIENTATION == 0x00
    // normal
#elif ORIENTATION == 0x60
    *x = ty;
    *y = w - tx;
#elif ORIENTATION == 0xC0
    *x = w - tx;
    *y = h - ty;
#elif ORIENTATION == 0xA0
    *x = h - ty;
    *y = tx;
#endif
}



void Touch_Process(TouchState_TypeDef *ts) {

    static TouchState_TypeDef last = {0};

    if (ts->touches == 0 && last.touches == 1) {
        ts->event = TOUCH_UP;
    } else if (ts->touches == 1 && last.touches == 0) {
        ts->event = TOUCH_DOWN;
    } else if (ts->touches == 1) {
        ts->event = TOUCH_HOLD;

        if (abs(ts->x - last.x) < TOUCH_DEADZONE &&
            abs(ts->y - last.y) < TOUCH_DEADZONE) {
            ts->x = last.x;
            ts->y = last.y;
        }
    }

    last = *ts;
}
