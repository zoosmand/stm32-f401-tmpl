/**
  ******************************************************************************
  * @file           : utils.c
  * @brief          : Application delays, error handling, and printf output.
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

#include "main.h"
#include "display_console.h"

/**
  * @brief Send one character through an enabled ITM stimulus channel.
  * @param character (uint32_t) Character value to send.
  * @param channel (uint32_t) ITM stimulus channel number.
  * @retval (uint32_t) The supplied character value.
  */
__STATIC_INLINE uint32_t utils_SendItmCharacter(uint32_t character, uint32_t channel) {
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&
      ((ITM->TER & (1UL << channel)) != 0UL)) {
    while (ITM->PORT[channel].u32 == 0UL) {
      __NOP();
    }
    ITM->PORT[channel].u8 = (uint8_t)character;
  }
  return character;
}

/**
  * @brief Route one printf character to the configured output devices.
  * @param character (uint8_t) Character to send.
  */
__STATIC_INLINE void utils_PutCharacter(uint8_t character) {
  if (character == '\n') utils_PutCharacter('\r');

  #ifdef SWO_ITM
    utils_SendItmCharacter(character, SWO_ITM);
  #endif

  #ifdef USART_OUT
    while (!(PREG_CHECK(USART_OUT->SR, USART_SR_TXE_Pos)));
    USART_OUT->DR = character;
  #endif
}

/**
  * @brief Provide the newlib write syscall used by printf.
  * @param fileDescriptor (int32_t) Newlib file descriptor; currently ignored.
  * @param data (char*) Characters to write.
  * @param length (int32_t) Number of characters to write.
  * @retval (int) Number of characters accepted.
  */
int _write(int32_t fileDescriptor, char* data, int32_t length) {
  (void)fileDescriptor;

  #ifdef DSPL_OUT
    (void)DisplayConsole_Write(data, (uint32_t)length);
  #endif

  for (int32_t index = 0; index < length; index++)
    utils_PutCharacter((uint8_t)*data++);

  return length;
}

/** @brief Enable and reset the DWT cycle counter. */
__STATIC_INLINE void utils_InitCycleCounter(void) {
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCEVTENA_Msk | DWT_CTRL_CYCCNTENA_Msk;
  __DSB();
  __ISB();
}

void Delay_Microseconds(uint32_t delayUs) {
  utils_InitCycleCounter();
  uint32_t const startCycle = DWT->CYCCNT;
  uint32_t const delayCycles = delayUs * (HAL_RCC_GetSysClockFreq() / 1000000U);
  while ((READ_REG(DWT->CYCCNT) - startCycle) < delayCycles) {
    __NOP();
  }
  DWT->CTRL &= ~(DWT_CTRL_CYCEVTENA_Msk | DWT_CTRL_CYCCNTENA_Msk);
}

void Delay_Milliseconds(uint32_t delayMs) {
  uint32_t startTick = HAL_GetTick();
  while ((HAL_GetTick() - startTick) < delayMs) {
    __NOP();
  }
}
