/**
  ******************************************************************************
  * @file           : rtc_clock.h
  * @brief          : STM32 hardware RTC interface.
  * @project        : STM32F401 Health Check
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 28.07.2026
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

#ifndef RTC_CLOCK_H
#define RTC_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

/**
  * @brief Calendar value stored by the hardware RTC in UTC.
  * @param year (uint16_t) Full Gregorian year in the range 2000 through 2099.
  * @param month (uint8_t) Month in the range 1 through 12.
  * @param day (uint8_t) Day of month in the range 1 through 31.
  * @param hour (uint8_t) Hour in the range 0 through 23.
  * @param minute (uint8_t) Minute in the range 0 through 59.
  * @param second (uint8_t) Second in the range 0 through 59.
  */
typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} RtcClock_DateTimeTypeDef;

/**
  * @brief Enable the LSE-backed RTC without overwriting its calendar.
  * @retval (HAL_StatusTypeDef) HAL_OK when the RTC is ready.
  */
HAL_StatusTypeDef RtcClock_Init(void);

/**
  * @brief Store a validated UTC calendar value and mark the RTC as valid.
  * @param dateTime (const RtcClock_DateTimeTypeDef*) UTC value to store.
  * @retval (HAL_StatusTypeDef) HAL status of the operation.
  */
HAL_StatusTypeDef RtcClock_SetUtc(
  const RtcClock_DateTimeTypeDef* dateTime
);

/**
  * @brief Read the current UTC calendar value.
  * @param dateTime (RtcClock_DateTimeTypeDef*) Destination for the RTC value.
  * @retval (HAL_StatusTypeDef) HAL status of the operation.
  */
HAL_StatusTypeDef RtcClock_GetUtc(RtcClock_DateTimeTypeDef* dateTime);

/**
  * @brief Check whether a successful synchronization marked the RTC as valid.
  * @retval (bool) true when the backup-domain marker is present.
  */
bool RtcClock_IsValid(void);

#endif /* RTC_CLOCK_H */
