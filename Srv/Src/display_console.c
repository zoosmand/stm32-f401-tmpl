/**
  ******************************************************************************
  * @file           : display_console.c
  * @brief          : Queued display-backed standard output service.
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

#include "display_console.h"

#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "rtos_tasks.h"
#include "semphr.h"
#include "task.h"

#define DISPLAY_CONSOLE_COLUMNS             64U
#define DISPLAY_CONSOLE_QUEUE_LENGTH         8U
#define DISPLAY_CONSOLE_TASK_STACK_DEPTH   384U
#define DISPLAY_CONSOLE_X                   10U
#define DISPLAY_CONSOLE_Y                  310U
#define DISPLAY_CONSOLE_LINE_COUNT          33U

typedef struct {
  char data[DISPLAY_CONSOLE_COLUMNS + 1U];
} DisplayConsole_LineTypeDef;

static Display_TypeDef* displayConsoleDevice;
static char displayConsoleBuffer[DISPLAY_CONSOLE_COLUMNS + 1U];
static uint16_t displayConsoleBufferCount;
static uint16_t displayConsoleLineIndex;

static StaticQueue_t displayConsoleQueueControlBlock;
static uint8_t displayConsoleQueueStorage[
  DISPLAY_CONSOLE_QUEUE_LENGTH * sizeof(DisplayConsole_LineTypeDef)
];
static QueueHandle_t displayConsoleQueue;

static StaticSemaphore_t displayConsoleWriteMutexBuffer;
static SemaphoreHandle_t displayConsoleWriteMutex;

static StaticTask_t displayConsoleTaskControlBlock;
static StackType_t displayConsoleTaskStack[
  DISPLAY_CONSOLE_TASK_STACK_DEPTH
];

static Font_TypeDef displayConsoleFont = {
  .backgroundColor = DISPLAY_COLOR_BLACK,
  .color = DISPLAY_COLOR_LIME,
  .fontData = (const uint8_t*)&fontDot5x7,
  .height = 8U,
  .width = 6U,
  .bytesPerGlyph = 6U,
};

static void displayConsole_Task(void*);
static void displayConsole_SubmitLine(void);
static void displayConsole_RenderLine(const DisplayConsole_LineTypeDef*);

DisplayConsole_StatusTypeDef DisplayConsole_Init(Display_TypeDef* display) {
  if (display == NULL)
    return DISPLAY_CONSOLE_STATUS_ERROR;

  displayConsoleDevice = display;
  displayConsoleBufferCount = 0U;
  displayConsoleLineIndex = 0U;
  displayConsoleBuffer[0] = '\0';

  displayConsoleQueue = xQueueCreateStatic(
    DISPLAY_CONSOLE_QUEUE_LENGTH,
    sizeof(DisplayConsole_LineTypeDef),
    displayConsoleQueueStorage,
    &displayConsoleQueueControlBlock
  );
  displayConsoleWriteMutex = xSemaphoreCreateMutexStatic(
    &displayConsoleWriteMutexBuffer
  );
  if ((displayConsoleQueue == NULL) || (displayConsoleWriteMutex == NULL))
    return DISPLAY_CONSOLE_STATUS_ERROR;

  if (xTaskCreateStatic(
        displayConsole_Task,
        "display_out",
        DISPLAY_CONSOLE_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1U,
        displayConsoleTaskStack,
        &displayConsoleTaskControlBlock
      ) == NULL)
    return DISPLAY_CONSOLE_STATUS_ERROR;

  return DISPLAY_CONSOLE_STATUS_OK;
}

uint32_t DisplayConsole_Write(const char* data, uint32_t length) {
  if ((data == NULL) || (displayConsoleDevice == NULL))
    return 0U;

  BaseType_t const schedulerRunning =
    (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED);
  if (schedulerRunning)
    (void)xSemaphoreTake(displayConsoleWriteMutex, portMAX_DELAY);

  for (uint32_t index = 0U; index < length; index++) {
    uint8_t character = (uint8_t)data[index];

    if (character == '\r')
      continue;

    if (character == '\n') {
      displayConsole_SubmitLine();
      continue;
    }

    if ((character < ' ') || (character > '~'))
      character = '?';

    displayConsoleBuffer[displayConsoleBufferCount++] = (char)character;
    if (displayConsoleBufferCount == DISPLAY_CONSOLE_COLUMNS)
      displayConsole_SubmitLine();
  }

  if (schedulerRunning)
    (void)xSemaphoreGive(displayConsoleWriteMutex);

  return length;
}

static void displayConsole_SubmitLine(void) {
  DisplayConsole_LineTypeDef line;

  memcpy(line.data, displayConsoleBuffer, displayConsoleBufferCount);
  memset(
    &line.data[displayConsoleBufferCount],
    ' ',
    DISPLAY_CONSOLE_COLUMNS - displayConsoleBufferCount
  );
  line.data[DISPLAY_CONSOLE_COLUMNS] = '\0';
  displayConsoleBufferCount = 0U;
  displayConsoleBuffer[0] = '\0';

  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    displayConsole_RenderLine(&line);
    return;
  }

  /*
   * Standard output must never block an application task. If the console
   * cannot keep up, discard only this new line and preserve queued output.
   */
  (void)xQueueSend(displayConsoleQueue, &line, 0U);
}

static void displayConsole_RenderLine(
  const DisplayConsole_LineTypeDef* line
) {
  uint16_t const y = DISPLAY_CONSOLE_Y -
    (displayConsoleLineIndex * displayConsoleFont.height);

  (void)Display_PrintString(
    displayConsoleDevice,
    DISPLAY_CONSOLE_X,
    y,
    &displayConsoleFont,
    line->data
  );

  displayConsoleLineIndex++;
  if (displayConsoleLineIndex >= DISPLAY_CONSOLE_LINE_COUNT)
    displayConsoleLineIndex = 0U;

  (void)Display_FillRectangle(
    displayConsoleDevice,
    DISPLAY_CONSOLE_X,
    DISPLAY_CONSOLE_Y -
      (displayConsoleLineIndex * displayConsoleFont.height),
    DISPLAY_CONSOLE_COLUMNS * displayConsoleFont.width,
    displayConsoleFont.height,
    displayConsoleFont.backgroundColor,
    DISPLAY_LAYER_FRONT
  );
}

static void displayConsole_Task(void* argument) {
  (void)argument;
  DisplayConsole_LineTypeDef line;

  for (;;) {
    if (xQueueReceive(displayConsoleQueue, &line, portMAX_DELAY) == pdTRUE) {
      RtosTasks_DisplayLock();
      displayConsole_RenderLine(&line);
      RtosTasks_DisplayUnlock();
    }
  }
}
