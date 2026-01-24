/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : This file contains the common defines for the project.
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

#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"



/**
 * @brief   Display device type definition struct.
 */
typedef struct {
  FunctionalState       Lock;
  uint16_t              Model;
  uint16_t              Width;
  uint16_t              Height;
  uint32_t*             Device;
  uint16_t*             PixBuf;
  uint16_t              PixBufSize;
  HAL_StatusTypeDef     (*Callback)(uint32_t*);
} Display_TypeDef;


typedef struct {
  uint8_t               Width;
  uint8_t               Height;
  uint16_t              Color;
  uint16_t              Bgcolor;
  uint16_t              BytesPerGlif;
  uint8_t*              Font;
} Font_TypeDef;


typedef struct {
  uint8_t  touches;
  uint16_t x;
  uint16_t y;
  uint8_t  event;   // 0=down, 1=up, 2=contact
} TouchState_TypeDef;

typedef enum {
  TOUCH_IDLE,
  TOUCH_DOWN,
  TOUCH_HOLD,
  TOUCH_UP,
  TOUCH_BLOCKED,
  TOUCH_DISABLED,
} TouchPhase_t;

typedef struct {
  TouchState_TypeDef*   State;
  TouchPhase_t          Phase;
  HAL_StatusTypeDef     (*Callback)(uint32_t*);
} TouchScreen_TypeDef;




#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H */


