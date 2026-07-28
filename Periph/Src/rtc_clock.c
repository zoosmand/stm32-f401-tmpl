/**
  ******************************************************************************
  * @file           : rtc_clock.c
  * @brief          : LSE-backed STM32 hardware RTC implementation.
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

#include "rtc_clock.h"

#include <stddef.h>

#define RTC_CLOCK_BACKUP_REGISTER    RTC_BKP_DR0
#define RTC_CLOCK_VALID_MARKER        0x52544321UL
#define RTC_CLOCK_MINIMUM_YEAR        2000U
#define RTC_CLOCK_MAXIMUM_YEAR        2099U

static RTC_HandleTypeDef rtcClockHandle;

static bool rtcClock_DateTimeIsValid(
  const RtcClock_DateTimeTypeDef*
);
static uint8_t rtcClock_GetWeekday(uint16_t, uint8_t, uint8_t);

HAL_StatusTypeDef RtcClock_Init(void) {
  RCC_OscInitTypeDef oscillator = {0};
  RCC_PeriphCLKInitTypeDef peripheralClock = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  oscillator.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  oscillator.LSEState = RCC_LSE_ON;
  oscillator.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&oscillator) != HAL_OK)
    return HAL_ERROR;

  peripheralClock.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  peripheralClock.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&peripheralClock) != HAL_OK)
    return HAL_ERROR;

  __HAL_RCC_RTC_ENABLE();

  rtcClockHandle.Instance = RTC;
  rtcClockHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  rtcClockHandle.Init.AsynchPrediv = 127U;
  rtcClockHandle.Init.SynchPrediv = 255U;
  rtcClockHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  rtcClockHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  rtcClockHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  return HAL_RTC_Init(&rtcClockHandle);
}

HAL_StatusTypeDef RtcClock_SetUtc(
  const RtcClock_DateTimeTypeDef* dateTime
) {
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};

  if (!rtcClock_DateTimeIsValid(dateTime))
    return HAL_ERROR;

  time.Hours = dateTime->hour;
  time.Minutes = dateTime->minute;
  time.Seconds = dateTime->second;
  time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  time.StoreOperation = RTC_STOREOPERATION_RESET;

  date.Year = (uint8_t)(dateTime->year - RTC_CLOCK_MINIMUM_YEAR);
  date.Month = dateTime->month;
  date.Date = dateTime->day;
  date.WeekDay = rtcClock_GetWeekday(
    dateTime->year,
    dateTime->month,
    dateTime->day
  );

  if (HAL_RTC_SetTime(&rtcClockHandle, &time, RTC_FORMAT_BIN) != HAL_OK)
    return HAL_ERROR;
  if (HAL_RTC_SetDate(&rtcClockHandle, &date, RTC_FORMAT_BIN) != HAL_OK)
    return HAL_ERROR;

  HAL_RTCEx_BKUPWrite(
    &rtcClockHandle,
    RTC_CLOCK_BACKUP_REGISTER,
    RTC_CLOCK_VALID_MARKER
  );
  return HAL_OK;
}

HAL_StatusTypeDef RtcClock_GetUtc(RtcClock_DateTimeTypeDef* dateTime) {
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;

  if (dateTime == NULL)
    return HAL_ERROR;

  if (HAL_RTC_GetTime(&rtcClockHandle, &time, RTC_FORMAT_BIN) != HAL_OK)
    return HAL_ERROR;
  if (HAL_RTC_GetDate(&rtcClockHandle, &date, RTC_FORMAT_BIN) != HAL_OK)
    return HAL_ERROR;

  dateTime->year = RTC_CLOCK_MINIMUM_YEAR + date.Year;
  dateTime->month = date.Month;
  dateTime->day = date.Date;
  dateTime->hour = time.Hours;
  dateTime->minute = time.Minutes;
  dateTime->second = time.Seconds;
  return HAL_OK;
}

bool RtcClock_IsValid(void) {
  return (
    HAL_RTCEx_BKUPRead(
      &rtcClockHandle,
      RTC_CLOCK_BACKUP_REGISTER
    ) == RTC_CLOCK_VALID_MARKER
  );
}

static bool rtcClock_DateTimeIsValid(
  const RtcClock_DateTimeTypeDef* dateTime
) {
  static const uint8_t daysPerMonth[] = {
    31U, 28U, 31U, 30U, 31U, 30U,
    31U, 31U, 30U, 31U, 30U, 31U,
  };
  uint8_t maximumDay;
  bool leapYear;

  if ((dateTime == NULL)
      || (dateTime->year < RTC_CLOCK_MINIMUM_YEAR)
      || (dateTime->year > RTC_CLOCK_MAXIMUM_YEAR)
      || (dateTime->month < 1U)
      || (dateTime->month > 12U)
      || (dateTime->hour > 23U)
      || (dateTime->minute > 59U)
      || (dateTime->second > 59U)) {
    return false;
  }

  maximumDay = daysPerMonth[dateTime->month - 1U];
  leapYear = ((dateTime->year % 4U) == 0U)
    && (((dateTime->year % 100U) != 0U)
      || ((dateTime->year % 400U) == 0U));
  if ((dateTime->month == 2U) && leapYear)
    maximumDay++;

  return (dateTime->day >= 1U) && (dateTime->day <= maximumDay);
}

static uint8_t rtcClock_GetWeekday(
  uint16_t year,
  uint8_t month,
  uint8_t day
) {
  static const uint8_t monthOffsets[] = {
    0U, 3U, 2U, 5U, 0U, 3U,
    5U, 1U, 4U, 6U, 2U, 4U,
  };

  if (month < 3U)
    year--;

  uint8_t const sundayBasedWeekday = (uint8_t)(
    (
      year
      + (year / 4U)
      - (year / 100U)
      + (year / 400U)
      + monthOffsets[month - 1U]
      + day
    ) % 7U
  );

  return (sundayBasedWeekday == 0U)
    ? RTC_WEEKDAY_SUNDAY
    : sundayBasedWeekday;
}
