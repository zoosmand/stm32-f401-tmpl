/**
  ******************************************************************************
  * @file           : rtos_tasks.c
  * @brief          : Application FreeRTOS tasks and W5500 access arbitration.
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
#include "loopback.h"
#include "semphr.h"
#include "task.h"

#define DEFAULT_TASK_STACK_DEPTH       256U
#define WIZ_TASK_STACK_DEPTH           512U
#define DEFAULT_TASK_PERIOD_MS         10U
#define WIZ_SERVER_TASK_PERIOD_MS      10U
#define WIZ_CLIENT_TASK_PERIOD_MS      2560U
#define WIZ_SERVER_SOCKET              0U
#define WIZ_CLIENT_SOCKET              1U
#define WIZ_SERVER_PORT                5300U
#define WIZ_CLIENT_PORT                54000U
#define WIZ_LOOPBACK_BUFFER_SIZE       1024U

typedef struct {
  Display_TypeDef* display;
  TouchScreen_TypeDef* touchScreen;
} DefaultTask_ContextTypeDef;

static DefaultTask_ContextTypeDef defaultTaskContext;

static StaticTask_t defaultTaskControlBlock;
static StackType_t defaultTaskStack[DEFAULT_TASK_STACK_DEPTH];
static StaticTask_t wizServerTaskControlBlock;
static StackType_t wizServerTaskStack[WIZ_TASK_STACK_DEPTH];
static StaticTask_t wizClientTaskControlBlock;
static StackType_t wizClientTaskStack[WIZ_TASK_STACK_DEPTH];

static StaticSemaphore_t displayMutexBuffer;
static SemaphoreHandle_t displayMutex;
static StaticSemaphore_t wizMutexBuffer;
static SemaphoreHandle_t wizMutex;

static uint8_t wizServerBuffer[WIZ_LOOPBACK_BUFFER_SIZE];
static uint8_t wizClientBuffer[WIZ_LOOPBACK_BUFFER_SIZE];
static uint8_t wizClientIpAddress[4] = {172U, 18U, 10U, 18U};

static void rtosTasks_DefaultTask(void*);
static void rtosTasks_WizServerTask(void*);
static void rtosTasks_WizClientTask(void*);

RtosTasks_StatusTypeDef RtosTasks_Init(
  Display_TypeDef* display,
  TouchScreen_TypeDef* touchScreen
) {
  if ((display == NULL) || (touchScreen == NULL))
    return RTOS_TASKS_STATUS_ERROR;

  defaultTaskContext.display = display;
  defaultTaskContext.touchScreen = touchScreen;

  displayMutex = xSemaphoreCreateMutexStatic(&displayMutexBuffer);
  wizMutex = xSemaphoreCreateMutexStatic(&wizMutexBuffer);
  if ((displayMutex == NULL) || (wizMutex == NULL))
    return RTOS_TASKS_STATUS_ERROR;

  if (xTaskCreateStatic(
        rtosTasks_DefaultTask,
        "default",
        DEFAULT_TASK_STACK_DEPTH,
        &defaultTaskContext,
        tskIDLE_PRIORITY + 1U,
        defaultTaskStack,
        &defaultTaskControlBlock
      ) == NULL)
    return RTOS_TASKS_STATUS_ERROR;

  if (xTaskCreateStatic(
        rtosTasks_WizServerTask,
        "wiz_server",
        WIZ_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 2U,
        wizServerTaskStack,
        &wizServerTaskControlBlock
      ) == NULL)
    return RTOS_TASKS_STATUS_ERROR;

  if (xTaskCreateStatic(
        rtosTasks_WizClientTask,
        "wiz_client",
        WIZ_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 2U,
        wizClientTaskStack,
        &wizClientTaskControlBlock
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
  DefaultTask_ContextTypeDef* context = argument;
  TickType_t lastWakeTick = xTaskGetTickCount();

  for (;;) {
    RtosTasks_DisplayLock();
    Display_Run(context->display, context->touchScreen);
    RtosTasks_DisplayUnlock();
    vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(DEFAULT_TASK_PERIOD_MS));
  }
}

static void rtosTasks_WizServerTask(void* argument) {
  (void)argument;
  TickType_t lastWakeTick = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(wizMutex, portMAX_DELAY) == pdTRUE) {
      (void)loopback_tcps(
        WIZ_SERVER_SOCKET,
        wizServerBuffer,
        WIZ_SERVER_PORT
      );
      (void)xSemaphoreGive(wizMutex);
    }
    vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(WIZ_SERVER_TASK_PERIOD_MS));
  }
}

static void rtosTasks_WizClientTask(void* argument) {
  (void)argument;
  TickType_t lastWakeTick = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(wizMutex, portMAX_DELAY) == pdTRUE) {
      (void)loopback_tcpc(
        WIZ_CLIENT_SOCKET,
        wizClientBuffer,
        wizClientIpAddress,
        WIZ_CLIENT_PORT
      );
      (void)xSemaphoreGive(wizMutex);
    }
    vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(WIZ_CLIENT_TASK_PERIOD_MS));
  }
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
