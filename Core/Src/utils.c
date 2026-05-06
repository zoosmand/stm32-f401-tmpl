/**
  ******************************************************************************
  * @file           : utils.c
  * @brief          : Rrogram utilities
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"




/********************************************************************************/
/*                         printf() output supply block                         */
/********************************************************************************/


static char print_dspl_buf[78];
static uint16_t print_dspl_buf_cnt = 0;
static uint16_t print_dspl_line_cnt = 0;

// extern Display_TypeDef* display_0;


__STATIC_INLINE void print_dspl(Display_TypeDef*);




/**
  * @brief  Sends a symbol into ITM channel. It could be cought with SWO pin on an MC. 
  * @param ch: a symbol to be output
  * @param channel: number of an ITM channel
  * @retval the same symbol 
  */
 __STATIC_INLINE uint32_t ITM_SendCharChannel(uint32_t ch, uint32_t channel) {
  /* ITM enabled and ITM Port enabled */
 if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) && ((ITM->TER & (1 << channel)) != 0UL)) {
   while (ITM->PORT[channel].u32 == 0UL) {
     __NOP();
   }
   ITM->PORT[channel].u8 = (uint8_t)ch;
 }
 return (ch);
}



void __attribute__((weak)) Error_Handler(void) {
  while (1);
}


/**
 * @brief  Sends a symbol into USART. 
 * @param device: a pointer USART_TypeDef
 * @param ch: a symbol to be output
 * @param check: a pointer to a BitBand check bit
 * @retval none: 
 */
__STATIC_INLINE void _putc(uint8_t ch) {
 if (ch == '\n') _putc('\r');

  #ifdef SWO_ITM
    ITM_SendCharChannel(ch, SWO_ITM);
  #endif

  #ifdef DSPL_OUT
  if (ch == '\n') {
    for (uint16_t i = print_dspl_buf_cnt; i < sizeof(print_dspl_buf); i++) {
      print_dspl_buf[i] = ' ';
    }
    print_dspl_buf_cnt = 0;
    print_dspl(display_0);
  } else {
    print_dspl_buf[print_dspl_buf_cnt++] = ch;
  }
  #endif

  #ifdef USART_OUT
    while (!(PREG_CHECK(USART_OUT->SR, USART_SR_TXE_Pos)));
    USART_OUT->DR = ch;
  #endif
}



/**
 * @brief An interpretation of the __weak system _write()
 * @param file: IO file
 * @param ptr: pointer to a char(symbol) array
 * @param len: length oa the array
 * @retval length of the array 
 */
int _write(int32_t file, char *ptr, int32_t len) {
 // static uint32_t check = 0;
 for(int32_t i = 0 ; i < len ; i++) {
   _putc(*ptr++);  
 }
 return len;
}



__STATIC_INLINE void _DWT_Init(void) {
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCEVTENA_Msk | DWT_CTRL_CYCCNTENA_Msk;
  __DSB();
  __ISB();
}


void _delay_us(uint32_t us) {
  _DWT_Init();
  uint32_t const start = DWT->CYCCNT;
  uint32_t const ticks = us * (HAL_RCC_GetSysClockFreq() / 1000000U);
  while ((READ_REG(DWT->CYCCNT) - start) < ticks) { __asm volatile("nop"); }
  DWT->CTRL &= ~(DWT_CTRL_CYCEVTENA_Msk | DWT_CTRL_CYCCNTENA_Msk);
}




void _delay_ms(uint32_t ms) {
  uint32_t delay_threshold = HAL_GetTick() + ms;
  while (delay_threshold >= HAL_GetTick()) {__asm volatile("nop");};
}




__STATIC_INLINE void print_dspl(Display_TypeDef* screen) {

  Font_TypeDef font = {
    .Bgcolor      = COLOR_BLACK,
    .Color        = COLOR_LIME,
    .Font         = (uint8_t*)&font_dot_5x7,
    .Height       = 8,
    .Width        = 6,
    .BytesPerGlif = 6,
  };

  #define PRINTF_Y_POS 310
  #define PRINTF_X_POS 10


  Display_PrintString(screen, PRINTF_X_POS, (PRINTF_Y_POS - (print_dspl_line_cnt * font.Height)), &font, print_dspl_buf);

  if (print_dspl_line_cnt++ > 32) {
    print_dspl_line_cnt = 0;
  } else {
    Display_FillRectangle(
      screen, 
      PRINTF_X_POS,
      (PRINTF_Y_POS - (print_dspl_line_cnt * font.Height)), 
      (sizeof(print_dspl_buf) * font.Width),
      font.Height, 
      font.Bgcolor, 
      FRONT
    );
  }

}

