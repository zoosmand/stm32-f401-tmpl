/**
  ******************************************************************************
  * @file           : watchdog_service.c
  * @brief          : Independent watchdog FreeRTOS service implementation.
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

#include "watchdog_service.h"

#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#define WATCHDOG_TASK_STACK_DEPTH        256U
#define WATCHDOG_REFRESH_PERIOD_MS      1000U
#define WATCHDOG_LSI_START_TIMEOUT_MS    100U
#define WATCHDOG_REGISTER_TIMEOUT_MS     100U

#define WATCHDOG_KEY_RELOAD        0x0000aaaaUL
#define WATCHDOG_KEY_ENABLE        0x0000ccccUL
#define WATCHDOG_KEY_WRITE_ACCESS  0x00005555UL
#define WATCHDOG_PRESCALER_DIV64   0x00000004UL
#define WATCHDOG_RELOAD_VALUE      1999UL

static StaticTask_t watchdogTaskControlBlock;
static StackType_t watchdogTaskStack[WATCHDOG_TASK_STACK_DEPTH];
static TaskHandle_t watchdogTaskHandle;

static void watchdogService_Task(void*);
static bool watchdogService_HardwareStart(void);

WatchdogService_StatusTypeDef WatchdogService_Init(void) {
  watchdogTaskHandle = xTaskCreateStatic(
    watchdogService_Task,
    "watchdog",
    WATCHDOG_TASK_STACK_DEPTH,
    NULL,
    tskIDLE_PRIORITY + 2U,
    watchdogTaskStack,
    &watchdogTaskControlBlock
  );
  return (watchdogTaskHandle != NULL)
    ? WATCHDOG_SERVICE_STATUS_OK
    : WATCHDOG_SERVICE_STATUS_ERROR;
}

void WatchdogService_Start(void) {
  if (watchdogTaskHandle != NULL)
    xTaskNotifyGive(watchdogTaskHandle);
}

static void watchdogService_Task(void* argument) {
  (void)argument;

  (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (!watchdogService_HardwareStart()) {
    printf("IWDG: initialization failed\n");
    for (;;)
      vTaskDelay(portMAX_DELAY);
  }

  printf("IWDG: active\n");
  TickType_t lastWakeTick = xTaskGetTickCount();
  for (;;) {
    IWDG->KR = WATCHDOG_KEY_RELOAD;
    vTaskDelayUntil(
      &lastWakeTick,
      pdMS_TO_TICKS(WATCHDOG_REFRESH_PERIOD_MS)
    );
  }
}

static bool watchdogService_HardwareStart(void) {
  uint32_t startTick;

  SET_BIT(RCC->CSR, RCC_CSR_LSION);
  startTick = HAL_GetTick();
  while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U) {
    if ((HAL_GetTick() - startTick) >= WATCHDOG_LSI_START_TIMEOUT_MS)
      return false;
  }

  __HAL_DBGMCU_FREEZE_IWDG();
  IWDG->KR = WATCHDOG_KEY_ENABLE;
  IWDG->KR = WATCHDOG_KEY_WRITE_ACCESS;
  IWDG->PR = WATCHDOG_PRESCALER_DIV64;
  IWDG->RLR = WATCHDOG_RELOAD_VALUE;

  startTick = HAL_GetTick();
  while (IWDG->SR != 0U) {
    if ((HAL_GetTick() - startTick) >= WATCHDOG_REGISTER_TIMEOUT_MS)
      return false;
  }
  IWDG->KR = WATCHDOG_KEY_RELOAD;
  return true;
}
