/**
  ******************************************************************************
  * @file           : time_service.c
  * @brief          : DNS, SNTP, and hardware RTC synchronization service.
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

#include "time_service.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "DNS/dns.h"
#include "FreeRTOS.h"
#include "rtc_clock.h"
#include "SNTP/sntp.h"
#include "socket.h"
#include "task.h"
#include "wizchip_port.h"

#define TIME_SERVICE_TASK_STACK_DEPTH        512U
#define TIME_SERVICE_SNTP_SOCKET               5U
#define TIME_SERVICE_NETWORK_WAIT_MS         1000U
#define TIME_SERVICE_SNTP_POLL_MS              50U
#define TIME_SERVICE_SNTP_TIMEOUT_MS        10000U
#define TIME_SERVICE_RETRY_PERIOD_MS        60000UL
#define TIME_SERVICE_SYNC_PERIOD_MS      21600000UL

static const uint8_t timeServiceServerName[] = "pool.ntp.org";
static uint8_t timeServiceSntpBuffer[MAX_SNTP_BUF_SIZE];

static StaticTask_t timeServiceTaskControlBlock;
static StackType_t timeServiceTaskStack[
  TIME_SERVICE_TASK_STACK_DEPTH
];

static void timeService_Task(void*);
static bool timeService_Synchronize(void);
static void timeService_PrintRetainedTime(void);

TimeService_StatusTypeDef TimeService_Init(void) {
  if (xTaskCreateStatic(
        timeService_Task,
        "time",
        TIME_SERVICE_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1U,
        timeServiceTaskStack,
        &timeServiceTaskControlBlock
      ) == NULL) {
    return TIME_SERVICE_STATUS_ERROR;
  }

  return TIME_SERVICE_STATUS_OK;
}

void TimeService_Tick(void) {
  static uint32_t elapsedTicks;

  elapsedTicks++;
  if (elapsedTicks >= configTICK_RATE_HZ) {
    elapsedTicks = 0U;
    DNS_time_handler();
  }
}

static void timeService_Task(void* argument) {
  (void)argument;

  timeService_PrintRetainedTime();

  while (!W5500_IsReady())
    vTaskDelay(pdMS_TO_TICKS(TIME_SERVICE_NETWORK_WAIT_MS));

  for (;;) {
    bool const synchronized = timeService_Synchronize();
    vTaskDelay(pdMS_TO_TICKS(
      synchronized
        ? TIME_SERVICE_SYNC_PERIOD_MS
        : TIME_SERVICE_RETRY_PERIOD_MS
    ));
  }
}

static bool timeService_Synchronize(void) {
  uint8_t dnsServer[4];
  uint8_t ntpServer[4];
  datetime networkTime;
  RtcClock_DateTimeTypeDef rtcTime;

  if (!W5500_GetDnsServer(dnsServer))
    return false;

  if (DNS_run(
      dnsServer,
      (uint8_t*)timeServiceServerName,
      ntpServer
    ) != 1) {
    printf("RTC: NTP DNS failed\n");
    return false;
  }

  SNTP_init(
    TIME_SERVICE_SNTP_SOCKET,
    ntpServer,
    SNTP_TIME_ZONE_UTC,
    timeServiceSntpBuffer
  );

  TickType_t const startTick = xTaskGetTickCount();
  while ((xTaskGetTickCount() - startTick)
      < pdMS_TO_TICKS(TIME_SERVICE_SNTP_TIMEOUT_MS)) {
    if (SNTP_run(&networkTime) == 1) {
      rtcTime.year = networkTime.yy;
      rtcTime.month = networkTime.mo;
      rtcTime.day = networkTime.dd;
      rtcTime.hour = networkTime.hh;
      rtcTime.minute = networkTime.mm;
      rtcTime.second = networkTime.ss;

      if (RtcClock_SetUtc(&rtcTime) != HAL_OK) {
        printf("RTC: invalid NTP time\n");
        return false;
      }

      printf(
        "RTC UTC: %04u-%02u-%02u %02u:%02u:%02u\n",
        rtcTime.year,
        rtcTime.month,
        rtcTime.day,
        rtcTime.hour,
        rtcTime.minute,
        rtcTime.second
      );
      return true;
    }

    vTaskDelay(pdMS_TO_TICKS(TIME_SERVICE_SNTP_POLL_MS));
  }

  close(TIME_SERVICE_SNTP_SOCKET);
  printf("RTC: NTP timeout\n");
  return false;
}

static void timeService_PrintRetainedTime(void) {
  RtcClock_DateTimeTypeDef rtcTime;

  if (!RtcClock_IsValid())
    return;
  if (RtcClock_GetUtc(&rtcTime) != HAL_OK)
    return;

  printf(
    "RTC retained: %04u-%02u-%02u %02u:%02u:%02u UTC\n",
    rtcTime.year,
    rtcTime.month,
    rtcTime.day,
    rtcTime.hour,
    rtcTime.minute,
    rtcTime.second
  );
}
