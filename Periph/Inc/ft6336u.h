/**
  ******************************************************************************
  * @file           : ft6336u.h
  * @brief          : Header for ft6336u.c file.
  *                   This file contains the common defines of the Touchscreen
  *                   controller FT6336U driver code.
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



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FT6336U_H
#define __FT6336U_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// | Signal      | Notes                             |
// | ----------- | --------------------------------- |
// | I²C address | `0x38` (7-bit)                    |
// | INT         | Optional (can poll instead)       |
// | RST         | Optional (often tied to MCU GPIO) |
// | VDD         | 3.3 V                             |
// | Pullups     | Required on SDA/SCL               |

#define FT6336_ADDR (0x38 << 1)

#define TC_RST_GPIO_Port  GPIOB
#define TC_RST_Pin        GPIO_PIN_5

#define TC_INT_GPIO_Port  GPIOB
#define TC_INT_Pin        GPIO_PIN_9


// Register	Addr	Size	Meaning
#define TD_STATUS	0x02	// 1	Number of touch points (0–2)
#define TOUCH1_XH	0x03	// 1	Touch 1 X high
#define TOUCH1_XL	0x04	// 1	Touch 1 X low
#define TOUCH1_YH	0x05	// 1	Touch 1 Y high
#define TOUCH1_YL	0x06	// 1	Touch 1 Y low

// uint16_t x = ((xh & 0x0F) << 8) | xl;
// uint16_t y = ((yh & 0x0F) << 8) | yl;

extern I2C_HandleTypeDef hi2c1;


TouchScreen_TypeDef* FT6336U_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef TC_Read(TouchState_TypeDef *ts);


#define TOUCH_DEADZONE 3  // pixels

// abs(x - last_x) < TOUCH_DEADZONE &&
// abs(y - last_y) < TOUCH_DEADZONE



#ifdef __cplusplus
}
#endif

#endif /* __FT6336U_H */
