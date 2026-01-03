


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ST7796_H
#define __ST7796_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// st7796_conf.h
#define ST7796_SPI       hspi1

#define TFT_CS_GPIO_Port GPIOA
#define TFT_CS_Pin       GPIO_PIN_4

#define TFT_DC_GPIO_Port GPIOA
#define TFT_DC_Pin       GPIO_PIN_2

#define TFT_RST_GPIO_Port GPIOA
#define TFT_RST_Pin       GPIO_PIN_3

// #define TFT_BL_GPIO_Port GPIOB
// #define TFT_BL_Pin       GPIO_PIN_10   // optional (or tie to VCC)


void ST7796_Init(void);
// void ST7796_SetAddrWindow(uint16_t, uint16_t, uint16_t, uint16_t);
void ST7796_FillColor(uint16_t);
void ST7796_Fill(uint16_t);



#ifdef __cplusplus
}
#endif

#endif /* __ST7796_H */
