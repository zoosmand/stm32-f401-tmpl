/**
  ******************************************************************************
  * @file           : wizchip_port.h
  * @brief          : W5500 board integration and network initialization.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 06.05.2026 07:03:36 PM
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

#ifndef WIZCHIP_PORT_H
#define WIZCHIP_PORT_H

/**
  * @brief Result of initializing the W5500 network interface.
  */
typedef enum {
  W5500_STATUS_OK = 0,
  W5500_STATUS_INIT_FAILED = -1,
  W5500_STATUS_COMMUNICATION_FAILED = -2,
  W5500_STATUS_LINK_DOWN = -3,
} W5500_StatusTypeDef;

/**
  * @brief Reset and configure the W5500, acquire an address, and initialize DNS.
  * @retval (W5500_StatusTypeDef) W5500_STATUS_OK on success.
  */
W5500_StatusTypeDef W5500_Init(void);

#endif /* WIZCHIP_PORT_H */
