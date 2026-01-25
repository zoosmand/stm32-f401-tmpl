/**
  ******************************************************************************
  * @file           : display.h
  * @brief          : Header for display.c file.
  *                   This file contains the common defines of display routines
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



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"



void Display_Run(Display_TypeDef*, TouchScreen_TypeDef*);




#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
