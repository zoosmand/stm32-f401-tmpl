/**
  ******************************************************************************
  * @file           : http_monitor_service.h
  * @brief          : Periodic HTTP resource health-monitor interface.
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

#ifndef HTTP_MONITOR_SERVICE_H
#define HTTP_MONITOR_SERVICE_H

#include <stdint.h>

typedef enum {
  HTTP_MONITOR_STATUS_OK = 0,
  HTTP_MONITOR_STATUS_ERROR,
} HttpMonitorService_StatusTypeDef;

typedef enum {
  HTTP_MONITOR_ERROR_NONE = 0U,
  HTTP_MONITOR_ERROR_DNS,
  HTTP_MONITOR_ERROR_SOCKET,
  HTTP_MONITOR_ERROR_CONNECT,
  HTTP_MONITOR_ERROR_SEND,
  HTTP_MONITOR_ERROR_TIMEOUT,
  HTTP_MONITOR_ERROR_RESPONSE,
  HTTP_MONITOR_ERROR_STATUS,
  HTTP_MONITOR_ERROR_LOG,
} HttpMonitorService_ErrorTypeDef;

/**
  * @brief Latest volatile HTTP monitor result.
  * @param statusCode (uint16_t) HTTP status, or zero when none was received.
  * @param error (HttpMonitorService_ErrorTypeDef) Latest result category.
  * @param address (uint8_t[4]) Most recently resolved IPv4 address.
  * @param consecutiveFailures (uint16_t) Number of checks since last success.
  */
typedef struct {
  uint16_t statusCode;
  HttpMonitorService_ErrorTypeDef error;
  uint8_t address[4];
  uint16_t consecutiveFailures;
} HttpMonitorService_SnapshotTypeDef;

/**
  * @brief Create the statically allocated periodic HTTP monitor task.
  * @retval (HttpMonitorService_StatusTypeDef) Task creation status.
  */
HttpMonitorService_StatusTypeDef HttpMonitorService_Init(void);

/**
  * @brief Copy the latest result atomically.
  * @param snapshot (HttpMonitorService_SnapshotTypeDef*) Destination.
  */
void HttpMonitorService_GetSnapshot(
  HttpMonitorService_SnapshotTypeDef* snapshot
);

#endif /* HTTP_MONITOR_SERVICE_H */
