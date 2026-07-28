/**
  ******************************************************************************
  * @file           : http_monitor_service.c
  * @brief          : Periodic HTTP resource health-monitor implementation.
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

#include "http_monitor_service.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "health_log.h"
#include "socket.h"
#include "task.h"
#include "wizchip_port.h"

#define HTTP_MONITOR_HOST              "hvm-a.ic.local"
#define HTTP_MONITOR_PORT              3000U
#define HTTP_MONITOR_PATH              "/"
#define HTTP_MONITOR_SOCKET            4U
#define HTTP_MONITOR_TASK_STACK_DEPTH  512U
#define HTTP_MONITOR_PERIOD_MS         60000UL
#define HTTP_MONITOR_NETWORK_WAIT_MS   1000U
#define HTTP_MONITOR_RESPONSE_MS       5000U
#define HTTP_MONITOR_POLL_MS           10U
#define HTTP_MONITOR_LINE_SIZE         64U

static const uint8_t httpMonitorRequest[] =
  "HEAD " HTTP_MONITOR_PATH " HTTP/1.1\r\n"
  "Host: " HTTP_MONITOR_HOST ":3000\r\n"
  "Connection: close\r\n"
  "\r\n";

static StaticTask_t httpMonitorTaskControlBlock;
static StackType_t httpMonitorTaskStack[HTTP_MONITOR_TASK_STACK_DEPTH];
static HttpMonitorService_SnapshotTypeDef httpMonitorSnapshot;

static void httpMonitorService_Task(void*);
static bool httpMonitorService_Check(
  uint8_t address[4],
  uint16_t* statusCode,
  HttpMonitorService_ErrorTypeDef* error
);
static bool httpMonitorService_ReadStatus(
  uint16_t* statusCode,
  HttpMonitorService_ErrorTypeDef* error
);
static bool httpMonitorService_ParseStatus(
  const char* line,
  uint16_t length,
  uint16_t* statusCode
);
static void httpMonitorService_Record(
  bool healthy,
  uint16_t statusCode,
  HttpMonitorService_ErrorTypeDef error,
  const uint8_t address[4]
);

HttpMonitorService_StatusTypeDef HttpMonitorService_Init(void) {
  if (xTaskCreateStatic(
        httpMonitorService_Task,
        "http",
        HTTP_MONITOR_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1U,
        httpMonitorTaskStack,
        &httpMonitorTaskControlBlock
      ) == NULL) {
    return HTTP_MONITOR_STATUS_ERROR;
  }
  return HTTP_MONITOR_STATUS_OK;
}

void HttpMonitorService_GetSnapshot(
  HttpMonitorService_SnapshotTypeDef* snapshot
) {
  if (snapshot == NULL)
    return;
  taskENTER_CRITICAL();
  *snapshot = httpMonitorSnapshot;
  taskEXIT_CRITICAL();
}

static void httpMonitorService_Task(void* argument) {
  (void)argument;

  while (!W5500_IsReady())
    vTaskDelay(pdMS_TO_TICKS(HTTP_MONITOR_NETWORK_WAIT_MS));

  bool logReady = HealthLog_Init() == SUCCESS;
  if (!logReady)
    printf("HTTP log: recovery failed\n");

  TickType_t lastWakeTick = xTaskGetTickCount();
  for (;;) {
    uint8_t address[4] = {0U};
    uint16_t statusCode = 0U;
    HttpMonitorService_ErrorTypeDef error = HTTP_MONITOR_ERROR_NONE;
    bool healthy = httpMonitorService_Check(address, &statusCode, &error);

    if (logReady
        && (HealthLog_Append(statusCode, (uint8_t)error, address) != SUCCESS)) {
      logReady = false;
      error = HTTP_MONITOR_ERROR_LOG;
    }
    httpMonitorService_Record(healthy, statusCode, error, address);
    printf(
      "HTTP %s:%u %s (%u)\n",
      HTTP_MONITOR_HOST,
      HTTP_MONITOR_PORT,
      healthy ? "OK" : "FAILED",
      statusCode
    );
    vTaskDelayUntil(
      &lastWakeTick,
      pdMS_TO_TICKS(HTTP_MONITOR_PERIOD_MS)
    );
  }
}

static bool httpMonitorService_Check(
  uint8_t address[4],
  uint16_t* statusCode,
  HttpMonitorService_ErrorTypeDef* error
) {
  if (!W5500_ResolveHost(HTTP_MONITOR_HOST, address)) {
    *error = HTTP_MONITOR_ERROR_DNS;
    return false;
  }

  (void)close(HTTP_MONITOR_SOCKET);
  if (socket(HTTP_MONITOR_SOCKET, Sn_MR_TCP, 0U, 0U)
      != HTTP_MONITOR_SOCKET) {
    *error = HTTP_MONITOR_ERROR_SOCKET;
    return false;
  }
  if (connect(HTTP_MONITOR_SOCKET, address, HTTP_MONITOR_PORT) != SOCK_OK) {
    *error = HTTP_MONITOR_ERROR_CONNECT;
    (void)close(HTTP_MONITOR_SOCKET);
    return false;
  }

  int32_t sent = send(
    HTTP_MONITOR_SOCKET,
    (uint8_t*)httpMonitorRequest,
    (uint16_t)(sizeof(httpMonitorRequest) - 1U)
  );
  if (sent != (int32_t)(sizeof(httpMonitorRequest) - 1U)) {
    *error = HTTP_MONITOR_ERROR_SEND;
    (void)close(HTTP_MONITOR_SOCKET);
    return false;
  }

  bool received = httpMonitorService_ReadStatus(statusCode, error);
  (void)disconnect(HTTP_MONITOR_SOCKET);
  (void)close(HTTP_MONITOR_SOCKET);
  if (!received)
    return false;
  if (*statusCode != 200U) {
    *error = HTTP_MONITOR_ERROR_STATUS;
    return false;
  }
  return true;
}

static bool httpMonitorService_ReadStatus(
  uint16_t* statusCode,
  HttpMonitorService_ErrorTypeDef* error
) {
  char line[HTTP_MONITOR_LINE_SIZE];
  uint16_t length = 0U;
  TickType_t startTick = xTaskGetTickCount();

  while ((xTaskGetTickCount() - startTick)
      < pdMS_TO_TICKS(HTTP_MONITOR_RESPONSE_MS)) {
    if (getSn_RX_RSR(HTTP_MONITOR_SOCKET) > 0U) {
      uint8_t byte;
      if (recv(HTTP_MONITOR_SOCKET, &byte, 1U) != 1) {
        *error = HTTP_MONITOR_ERROR_RESPONSE;
        return false;
      }
      if (length >= (sizeof(line) - 1U)) {
        *error = HTTP_MONITOR_ERROR_RESPONSE;
        return false;
      }
      line[length++] = (char)byte;
      if ((length >= 2U)
          && (line[length - 2U] == '\r')
          && (line[length - 1U] == '\n')) {
        if (!httpMonitorService_ParseStatus(line, length, statusCode)) {
          *error = HTTP_MONITOR_ERROR_RESPONSE;
          return false;
        }
        return true;
      }
    } else {
      uint8_t socketState = getSn_SR(HTTP_MONITOR_SOCKET);
      if ((socketState == SOCK_CLOSED) || (socketState == SOCK_CLOSE_WAIT)) {
        *error = HTTP_MONITOR_ERROR_RESPONSE;
        return false;
      }
      vTaskDelay(pdMS_TO_TICKS(HTTP_MONITOR_POLL_MS));
    }
  }
  *error = HTTP_MONITOR_ERROR_TIMEOUT;
  return false;
}

static bool httpMonitorService_ParseStatus(
  const char* line,
  uint16_t length,
  uint16_t* statusCode
) {
  if ((line == NULL) || (statusCode == NULL) || (length < 14U)
      || (memcmp(line, "HTTP/1.", 7U) != 0)
      || ((line[7] != '0') && (line[7] != '1'))
      || (line[8] != ' ')
      || (line[9] < '0') || (line[9] > '9')
      || (line[10] < '0') || (line[10] > '9')
      || (line[11] < '0') || (line[11] > '9')) {
    return false;
  }
  *statusCode = (uint16_t)(
    ((uint16_t)(line[9] - '0') * 100U)
    + ((uint16_t)(line[10] - '0') * 10U)
    + (uint16_t)(line[11] - '0')
  );
  return true;
}

static void httpMonitorService_Record(
  bool healthy,
  uint16_t statusCode,
  HttpMonitorService_ErrorTypeDef error,
  const uint8_t address[4]
) {
  taskENTER_CRITICAL();
  httpMonitorSnapshot.statusCode = statusCode;
  httpMonitorSnapshot.error = error;
  memcpy(httpMonitorSnapshot.address, address, sizeof(httpMonitorSnapshot.address));
  if (healthy) {
    httpMonitorSnapshot.consecutiveFailures = 0U;
  } else if (httpMonitorSnapshot.consecutiveFailures < UINT16_MAX) {
    httpMonitorSnapshot.consecutiveFailures++;
  }
  taskEXIT_CRITICAL();
}
