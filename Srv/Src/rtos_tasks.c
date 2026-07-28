/**
  ******************************************************************************
  * @file           : rtos_tasks.c
  * @brief          : Application FreeRTOS task and display access arbitration.
  * @project        : STM32F401 Test Platform
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

#include "rtos_tasks.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "wizchip_port.h"

#define DEFAULT_TASK_STACK_DEPTH    512U
#define DEFAULT_TASK_PERIOD_MS      1000U

static StaticTask_t defaultTaskControlBlock;
static StackType_t defaultTaskStack[DEFAULT_TASK_STACK_DEPTH];

static StaticSemaphore_t displayMutexBuffer;
static SemaphoreHandle_t displayMutex;

static void rtosTasks_DefaultTask(void*);

RtosTasks_StatusTypeDef RtosTasks_Init(void) {
  displayMutex = xSemaphoreCreateMutexStatic(&displayMutexBuffer);
  if (displayMutex == NULL)
    return RTOS_TASKS_STATUS_ERROR;

  if (xTaskCreateStatic(
        rtosTasks_DefaultTask,
        "default",
        DEFAULT_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1U,
        defaultTaskStack,
        &defaultTaskControlBlock
      ) == NULL)
    return RTOS_TASKS_STATUS_ERROR;

  return RTOS_TASKS_STATUS_OK;
}

void RtosTasks_DisplayLock(void) {
  if (displayMutex != NULL)
    (void)xSemaphoreTake(displayMutex, portMAX_DELAY);
}

void RtosTasks_DisplayUnlock(void) {
  if (displayMutex != NULL)
    (void)xSemaphoreGive(displayMutex);
}

static void rtosTasks_DefaultTask(void* argument) {
  (void)argument;

  if (W5500_Init() != W5500_STATUS_OK)
    printf("W5500 initialization failed\n");

  TickType_t lastWakeTick = xTaskGetTickCount();

  for (;;)
    vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(DEFAULT_TASK_PERIOD_MS));
}

void vApplicationStackOverflowHook(
  TaskHandle_t task,
  char* taskName
) {
  (void)task;
  (void)taskName;
  taskDISABLE_INTERRUPTS();
  for (;;) {
  }
}
