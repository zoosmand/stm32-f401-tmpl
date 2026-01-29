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
  uint8_t               Orientation;
  uint32_t*             Bus;
  uint16_t*             PixBuf;
  uint32_t              PixBufSize;
  uint32_t              PixBufActiveSize;
  uint16_t*             PixBufBg;
  uint32_t              PixBufBgSize;
  uint32_t              PixBufBgActiveSize;
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
  // uint8_t               touches;
  // uint16_t x;
  // uint16_t y;
  uint8_t               Event;   // 0=down, 1=up, 2=contact
  uint16_t              RawX;
  uint16_t              RawY;
  uint16_t              X;
  uint16_t              Y;
  uint16_t              LastX;
  uint16_t              LastY;
  uint16_t              BounceX;
  uint16_t              BounceY;
  uint8_t               StableCount;
  uint8_t               ReleaseCount;
  uint8_t               Touches;
  uint32_t              Timestamp;
} TouchContext_TypeDef;

typedef enum {
  TOUCH_IDLE,
  TOUCH_DOWN,
  TOUCH_HOLD,
  TOUCH_RELEASE,
  TOUCH_UP,
  TOUCH_DEBOUNCE,
  TOUCH_ACTIVE,
  TOUCH_LOCKED,
  TOUCH_DISABLED,
} TouchState_t;

typedef enum {
  TOUCH_ON_DOWN,
  TOUCH_ON_UP,
  TOUCH_ON_MOVE,
  TOUCH_ON_HOLD,
  TOUCH_ON_IDLE,
} TouchEvent_t;

typedef struct {
  uint16_t              Model;
  uint8_t               Orientation;
  TouchContext_TypeDef* Context;
  TouchState_t          State;
  TouchEvent_t          Event;
  uint32_t*             Bus;
  uint8_t               BusAddr;
  HAL_StatusTypeDef     (*Callback)(uint32_t*);
} TouchScreen_TypeDef;


typedef enum {
  WRITE = 0,
  READ  = 1,
  NOOP  = 2  
} TrasmissionDirection_t;

typedef enum {
  FRONT = 0,
  BACK  = 1,
  NONE  = 2  
} ImageLayer_t;


#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H */


