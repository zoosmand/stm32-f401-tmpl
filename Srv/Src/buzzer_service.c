/**
  ******************************************************************************
  * @file           : buzzer_service.c
  * @brief          : Queued FreeRTOS passive-buzzer service.
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

#include "buzzer_service.h"

#include <stdio.h>

#include "buzzer.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#define BUZZER_SERVICE_QUEUE_LENGTH          4U
#define BUZZER_SERVICE_TASK_STACK_DEPTH    256U

typedef struct {
  uint32_t frequencyHz;
  uint32_t durationMs;
} BuzzerService_ToneTypeDef;

static StaticQueue_t buzzerServiceQueueControlBlock;
static uint8_t buzzerServiceQueueStorage[
  BUZZER_SERVICE_QUEUE_LENGTH * sizeof(BuzzerService_ToneTypeDef)
];
static QueueHandle_t buzzerServiceQueue;

static StaticTask_t buzzerServiceTaskControlBlock;
static StackType_t buzzerServiceTaskStack[
  BUZZER_SERVICE_TASK_STACK_DEPTH
];

static void buzzerService_Task(void*);

BuzzerService_StatusTypeDef BuzzerService_Init(void) {
  if (Buzzer_Init() != SUCCESS)
    return BUZZER_SERVICE_STATUS_ERROR;

  buzzerServiceQueue = xQueueCreateStatic(
    BUZZER_SERVICE_QUEUE_LENGTH,
    sizeof(BuzzerService_ToneTypeDef),
    buzzerServiceQueueStorage,
    &buzzerServiceQueueControlBlock
  );
  if (buzzerServiceQueue == NULL)
    return BUZZER_SERVICE_STATUS_ERROR;

  if (xTaskCreateStatic(
        buzzerService_Task,
        "buzzer",
        BUZZER_SERVICE_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1U,
        buzzerServiceTaskStack,
        &buzzerServiceTaskControlBlock
      ) == NULL) {
    return BUZZER_SERVICE_STATUS_ERROR;
  }

  return BuzzerService_Play(
    BUZZER_SELF_TEST_FREQUENCY_HZ,
    BUZZER_SELF_TEST_DURATION_MS
  );
}

BuzzerService_StatusTypeDef BuzzerService_Play(
  uint32_t frequencyHz,
  uint32_t durationMs
) {
  BuzzerService_ToneTypeDef tone = {
    .frequencyHz = frequencyHz,
    .durationMs = durationMs,
  };

  if ((buzzerServiceQueue == NULL)
      || (frequencyHz < BUZZER_MIN_FREQUENCY_HZ)
      || (frequencyHz > BUZZER_MAX_FREQUENCY_HZ)
      || (durationMs == 0U)) {
    return BUZZER_SERVICE_STATUS_ERROR;
  }

  return (xQueueSend(buzzerServiceQueue, &tone, 0U) == pdPASS)
    ? BUZZER_SERVICE_STATUS_OK
    : BUZZER_SERVICE_STATUS_ERROR;
}

static void buzzerService_Task(void* argument) {
  BuzzerService_ToneTypeDef tone;
  (void)argument;

  for (;;) {
    if (xQueueReceive(
        buzzerServiceQueue,
        &tone,
        portMAX_DELAY
      ) != pdPASS) {
      continue;
    }

    if (Buzzer_Start(tone.frequencyHz) != SUCCESS) {
      Buzzer_Stop();
      printf("Buzzer: tone failed\n");
      continue;
    }

    vTaskDelay(pdMS_TO_TICKS(tone.durationMs));
    Buzzer_Stop();
  }
}
