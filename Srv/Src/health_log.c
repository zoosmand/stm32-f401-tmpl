/**
  ******************************************************************************
  * @file           : health_log.c
  * @brief          : Power-loss-tolerant circular HTTP health log.
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

#include "health_log.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "rtc_clock.h"
#include "w25qxx.h"

#define HEALTH_LOG_MAGIC                 0x484c4f47UL
#define HEALTH_LOG_VERSION              1U
#define HEALTH_LOG_BLOCK                127U
#define HEALTH_LOG_SECTOR_COUNT          15U
#define HEALTH_LOG_PAGES_PER_SECTOR      16U
#define HEALTH_LOG_SLOT_COUNT \
  (HEALTH_LOG_SECTOR_COUNT * HEALTH_LOG_PAGES_PER_SECTOR)

/**
  * @brief One page-aligned persistent health result.
  * @param magic (uint32_t) Commit marker, programmed after the record body.
  * @param sequence (uint32_t) Monotonically increasing record number.
  * @param year (uint16_t) UTC year, or zero when the RTC is not synchronized.
  * @param month..second (uint8_t) Remaining UTC calendar fields.
  * @param statusCode (uint16_t) HTTP status, or zero if unavailable.
  * @param error (uint8_t) HTTP monitor error category.
  * @param address (uint8_t[4]) Resolved IPv4 address.
  * @param crc (uint32_t) CRC-32 over all preceding fields with final magic.
  */
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint32_t sequence;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t reserved;
  uint16_t statusCode;
  uint8_t error;
  uint8_t address[4];
  uint8_t padding[5];
  uint32_t crc;
} HealthLog_RecordTypeDef;

_Static_assert(
  sizeof(HealthLog_RecordTypeDef) == 32U,
  "Health log record layout changed"
);

static uint16_t healthLogNextSlot;
static uint32_t healthLogNextSequence;

static uint32_t healthLog_Address(uint16_t slot);
static uint32_t healthLog_Crc32(const void* data, uint32_t length);
static bool healthLog_IsValid(const HealthLog_RecordTypeDef* record);

ErrorStatus HealthLog_Init(void) {
  HealthLog_RecordTypeDef record;
  bool found = false;
  uint32_t newestSequence = 0U;
  uint16_t newestSlot = 0U;

  for (uint16_t slot = 0U; slot < HEALTH_LOG_SLOT_COUNT; slot++) {
    if (W25Qxx_Read(
        healthLog_Address(slot),
        (uint8_t*)&record,
        sizeof(record)
      ) != SUCCESS) {
      return ERROR;
    }

    if (healthLog_IsValid(&record)
        && (!found
          || ((int32_t)(record.sequence - newestSequence) > 0))) {
      found = true;
      newestSequence = record.sequence;
      newestSlot = slot;
    }
  }

  healthLogNextSlot = found
    ? (uint16_t)((newestSlot + 1U) % HEALTH_LOG_SLOT_COUNT)
    : 0U;
  healthLogNextSequence = found ? newestSequence + 1U : 1U;
  return SUCCESS;
}

ErrorStatus HealthLog_Append(
  uint16_t statusCode,
  uint8_t error,
  const uint8_t address[4]
) {
  HealthLog_RecordTypeDef record;
  RtcClock_DateTimeTypeDef utc = {0};
  uint32_t addressValue = healthLog_Address(healthLogNextSlot);
  uint32_t committedMagic = HEALTH_LOG_MAGIC;

  if ((healthLogNextSlot % HEALTH_LOG_PAGES_PER_SECTOR) == 0U) {
    if (W25Qxx_Erase(addressValue, 1U) != SUCCESS)
      return ERROR;
  }

  memset(&record, 0xff, sizeof(record));
  record.magic = HEALTH_LOG_MAGIC;
  record.sequence = healthLogNextSequence;
  if (RtcClock_IsValid() && (RtcClock_GetUtc(&utc) == HAL_OK)) {
    record.year = utc.year;
    record.month = utc.month;
    record.day = utc.day;
    record.hour = utc.hour;
    record.minute = utc.minute;
    record.second = utc.second;
  } else {
    record.year = 0U;
    record.month = 0U;
    record.day = 0U;
    record.hour = 0U;
    record.minute = 0U;
    record.second = 0U;
  }
  record.reserved = 0U;
  record.statusCode = statusCode;
  record.error = error;
  memcpy(record.address, address, sizeof(record.address));
  memset(record.padding, 0U, sizeof(record.padding));
  record.crc = healthLog_Crc32(&record, offsetof(HealthLog_RecordTypeDef, crc));

  record.magic = UINT32_MAX;
  if (W25Qxx_Write(
      addressValue,
      (const uint8_t*)&record,
      sizeof(record)
    ) != SUCCESS) {
    return ERROR;
  }
  if (W25Qxx_Write(
      addressValue,
      (const uint8_t*)&committedMagic,
      sizeof(committedMagic)
    ) != SUCCESS) {
    return ERROR;
  }

  healthLogNextSlot =
    (uint16_t)((healthLogNextSlot + 1U) % HEALTH_LOG_SLOT_COUNT);
  healthLogNextSequence++;
  return SUCCESS;
}

static uint32_t healthLog_Address(uint16_t slot) {
  uint32_t sector = slot / HEALTH_LOG_PAGES_PER_SECTOR;
  uint32_t page = slot % HEALTH_LOG_PAGES_PER_SECTOR;
  return W25QXX_PACK_ADDRESS(HEALTH_LOG_BLOCK, sector, page);
}

static uint32_t healthLog_Crc32(const void* data, uint32_t length) {
  const uint8_t* bytes = data;
  uint32_t crc = UINT32_MAX;

  while (length-- > 0U) {
    crc ^= *bytes++;
    for (uint8_t bit = 0U; bit < 8U; bit++) {
      crc = (crc >> 1U) ^ ((crc & 1U) ? 0xedb88320UL : 0U);
    }
  }
  return ~crc;
}

static bool healthLog_IsValid(const HealthLog_RecordTypeDef* record) {
  if ((record == NULL)
      || (record->magic != HEALTH_LOG_MAGIC)
      || (record->sequence == 0U)
      || (record->sequence == UINT32_MAX)) {
    return false;
  }
  return record->crc
    == healthLog_Crc32(record, offsetof(HealthLog_RecordTypeDef, crc));
}
