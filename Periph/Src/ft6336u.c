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


/* --- exported public variables --- */
extern I2C_HandleTypeDef hi2c1;


/* --- private functions --- */
static void tc_int_event_callback(void);
static HAL_StatusTypeDef tc_read(TouchScreen_TypeDef*);
static void tc_map_to_display(TouchScreen_TypeDef*);





/* --- private variables --- */


/* --- public variables --- */
EXTI_HandleTypeDef exti_line_9 = {
  .Line             = TC_INT_Pin_Pos,
  .PendingCallback  = tc_int_event_callback,
};
TouchState_t touch_event = TOUCH_IDLE;






// --------------------------------------------------------------------------

__STATIC_INLINE void tc_reset(void) {
  HAL_GPIO_WritePin(TC_RST_GPIO_Port, TC_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(TC_RST_GPIO_Port, TC_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
}


// --------------------------------------------------------------------------

static void tc_int_event_callback(void) {
  touch_event = TOUCH_ACTIVE;
}


// --------------------------------------------------------------------------

TouchScreen_TypeDef* FT6336U_Init(void) {

  static TouchContext_TypeDef touch_0_context = {};
  static TouchScreen_TypeDef touch_0 = {
    .Model        = 6336,
    .Orientation  = ORIENTATION,
    .State        = TOUCH_DISABLED,
    .Context      = &touch_0_context,
    .Bus          = (uint32_t*)&hi2c1,
    .BusAddr      = (FT6336_ADDR << 1),
    .Callback     = NULL,
  };

  TouchScreen_TypeDef* dev = &touch_0;
  I2C_HandleTypeDef* bus = (I2C_HandleTypeDef*)dev->Bus;

  dev->State = TOUCH_LOCKED;
  if (bus->Lock == HAL_LOCKED) return dev;

  /* Initialize RESET Pin */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  GPIO_InitStruct.Pin = TC_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TC_RST_GPIO_Port, &GPIO_InitStruct);
  
  /* Initialize INT Pin */
  GPIO_InitStruct.Pin = TC_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TC_INT_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  tc_reset();

  /* Probe device */
  if (HAL_I2C_IsDeviceReady(bus, dev->BusAddr, 3, 100) != HAL_OK) return dev;
  
  uint8_t dummy;

  if (HAL_I2C_Mem_Read(bus, dev->BusAddr, 0x00, I2C_MEMADD_SIZE_8BIT, &dummy, 1, HAL_MAX_DELAY) != HAL_OK) return dev;
  
  dev->State = TOUCH_IDLE;
  return dev;
}



// --------------------------------------------------------------------------

static HAL_StatusTypeDef tc_read(TouchScreen_TypeDef* dev) {

    uint8_t buf[7];

    if (HAL_I2C_Mem_Read((I2C_HandleTypeDef*)dev->Bus, dev->BusAddr, 0x02, I2C_MEMADD_SIZE_8BIT, buf, sizeof(buf), HAL_MAX_DELAY) != HAL_OK) return HAL_ERROR;

    uint8_t touches = buf[0] & 0x0f;

    if (touches == 0) return HAL_OK;

    dev->Context->Event = (buf[1] >> 6) & 0x03;
    dev->Context->RawX  = ((buf[1] & 0x0f) << 8) | buf[2];
    dev->Context->RawY  = ((buf[3] & 0x0f) << 8) | buf[4];

    return HAL_OK;
}



// --------------------------------------------------------------------------

static void tc_map_to_display(TouchScreen_TypeDef* dev) {

  switch (dev->Orientation & 0xf0) {
    case 0x40:
      dev->Context->Y = dev->Context->RawX;
      dev->Context->X = dev->Context->RawY;
      break;
    
    case 0x80:
      dev->Context->Y = DISPLAY_HEIGHT - dev->Context->RawX;
      dev->Context->X = DISPLAY_WIDTH - dev->Context->RawY;
      break;
    
    case 0xc0:
      dev->Context->Y = DISPLAY_HEIGHT - dev->Context->RawY;
      break;
    
    case 0x00:
    default:
      dev->Context->X = DISPLAY_WIDTH - dev->Context->RawX;
      break;
  }
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) TouchScreen_Process(TouchScreen_TypeDef* dev) {

  if (tc_read(dev) != HAL_OK) return HAL_ERROR;

  // Normilixe coordinates
  tc_map_to_display(dev);


    // static TouchState_TypeDef last = {0};

    // if (ts->touches == 0 && last.touches == 1) {
    //     ts->event = TOUCH_UP;
    // } else if (ts->touches == 1 && last.touches == 0) {
    //     ts->event = TOUCH_DOWN;
    // } else if (ts->touches == 1) {
    //     ts->event = TOUCH_HOLD;

    //     if (abs(ts->x - last.x) < TOUCH_DEADZONE &&
    //         abs(ts->y - last.y) < TOUCH_DEADZONE) {
    //         ts->x = last.x;
    //         ts->y = last.y;
    //     }
    // }

    // last = *ts;

  return HAL_OK;
}
