/**
  ******************************************************************************
  * @file           : st7796.h
  * @brief          : Header for st7796.c file.
  *                   This file contains the common defines of the ST7796 TFT
  *                   driver code.
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


#ifndef __FONTS_H_
#define __FONTS_H_


#include "main.h"

typedef uint8_t font_dot_5x7_t[6];
typedef uint8_t font_dot_10x14_t[24];
typedef uint8_t font_dot_15x21_t[54];
typedef uint8_t font_dot_20x28_t[96];

extern const font_dot_5x7_t font_dot_5x7[96];
extern const font_dot_10x14_t font_dot_10x14[96];
extern const font_dot_15x21_t font_dot_15x21[96];
extern const font_dot_20x28_t font_dot_20x28[96];



#endif /* __FONTS_H_ */