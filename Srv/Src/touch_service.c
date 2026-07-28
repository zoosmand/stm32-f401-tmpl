/**
  ******************************************************************************
  * @file           : touch_service.c
  * @brief          : FreeRTOS touchscreen event-processing service.
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

#include "touch_service.h"

#include "FreeRTOS.h"
#include "rtos_tasks.h"
#include "task.h"

#define TOUCH_SERVICE_TASK_STACK_DEPTH    512U
#define TOUCH_SERVICE_SAMPLE_PERIOD_MS     10U

typedef struct {
  Display_TypeDef* display;
  TouchScreen_TypeDef* touchScreen;
} TouchService_ContextTypeDef;

static TouchService_ContextTypeDef touchServiceContext;
static StaticTask_t touchServiceTaskControlBlock;
static StackType_t touchServiceTaskStack[
  TOUCH_SERVICE_TASK_STACK_DEPTH
];

static void touchService_Task(void*);

TouchService_StatusTypeDef TouchService_Init(
  Display_TypeDef* display,
  TouchScreen_TypeDef* touchScreen
) {
  if ((display == NULL) || (touchScreen == NULL) ||
      (touchScreen->state != TOUCH_STATE_IDLE))
    return TOUCH_SERVICE_STATUS_ERROR;

  touchServiceContext.display = display;
  touchServiceContext.touchScreen = touchScreen;

  if (xTaskCreateStatic(
        touchService_Task,
        "touch",
        TOUCH_SERVICE_TASK_STACK_DEPTH,
        &touchServiceContext,
        tskIDLE_PRIORITY + 2U,
        touchServiceTaskStack,
        &touchServiceTaskControlBlock
      ) == NULL)
    return TOUCH_SERVICE_STATUS_ERROR;

  return TOUCH_SERVICE_STATUS_OK;
}

static void touchService_Task(void* argument) {
  TouchService_ContextTypeDef* context = argument;
  uint32_t handledInterruptSequence = 0U;
  bool touchSequenceActive = false;
  TickType_t lastWakeTick = xTaskGetTickCount();

  for (;;) {
    uint32_t const interruptSequence =
      FT6336U_GetInterruptSequence();

    if (interruptSequence != handledInterruptSequence) {
      handledInterruptSequence = interruptSequence;
      touchSequenceActive = true;
    }

    if (touchSequenceActive) {
      if (TouchScreen_Process(context->touchScreen) != HAL_OK) {
        touchSequenceActive = false;
      } else {
        if (context->touchScreen->event == TOUCH_EVENT_UP) {
          printf(
            "Touch x:%u y:%u\n",
            context->touchScreen->context->x,
            context->touchScreen->context->y
          );
        }

        RtosTasks_DisplayLock();
        Display_HandleTouchEvent(
          context->display,
          context->touchScreen
        );
        RtosTasks_DisplayUnlock();

        if ((context->touchScreen->state == TOUCH_STATE_IDLE) &&
            (context->touchScreen->context->touchCount == 0U))
          touchSequenceActive = false;
      }
    }

    vTaskDelayUntil(
      &lastWakeTick,
      pdMS_TO_TICKS(TOUCH_SERVICE_SAMPLE_PERIOD_MS)
    );
  }
}
