/**
  ******************************************************************************
  * @file           : fonts.h
  * @brief          : Bitmap font table declarations.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 07.01.2026 08:43:57 PM
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Dmitry Slobodchikov
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef FONTS_H
#define FONTS_H

#include "main.h"

typedef uint8_t FontDot5x7_TypeDef[6];
typedef uint8_t FontDot10x14_TypeDef[24];
typedef uint8_t FontDot15x21_TypeDef[54];
typedef uint8_t FontDot20x28_TypeDef[96];

extern const FontDot5x7_TypeDef fontDot5x7[96];
extern const FontDot10x14_TypeDef fontDot10x14[96];
extern const FontDot15x21_TypeDef fontDot15x21[96];
extern const FontDot20x28_TypeDef fontDot20x28[96];

#endif /* FONTS_H */
