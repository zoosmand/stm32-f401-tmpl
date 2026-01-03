
#include "st7796.h"


volatile bool st7796_dma_busy = false;



// st7796.c
static inline void CS_L(void){ HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET); }
static inline void CS_H(void){ HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET); }

static inline void DC_CMD(void){ HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET); }
static inline void DC_DATA(void){ HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET); }

static void ST7796_WriteCmd(uint8_t cmd)
{
  CS_L();
  DC_CMD();
  HAL_SPI_Transmit(&ST7796_SPI, &cmd, 1, HAL_MAX_DELAY);
  CS_H();
}

static void ST7796_WriteData(const uint8_t *data, uint32_t len)
{
  CS_L();
  DC_DATA();
  HAL_SPI_Transmit(&ST7796_SPI, (uint8_t*)data, len, HAL_MAX_DELAY);
  CS_H();
}

static void ST7796_Reset(void)
{
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120);
}



void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi == &ST7796_SPI) {
    CS_H();
    st7796_dma_busy = false;
  }
}

void ST7796_WriteData_DMA(uint8_t *data, uint32_t len)
{
  while (st7796_dma_busy);

  st7796_dma_busy = true;

  CS_L();
  DC_DATA();

  HAL_SPI_Transmit_DMA(&ST7796_SPI, data, len);
}



void ST7796_Init(void)
{
  // Backlight on (if controlled)
//   HAL_GPIO_WritePin(TFT_BL_GPIO_Port, TFT_BL_Pin, GPIO_PIN_SET);

  ST7796_Reset();

  ST7796_WriteCmd(0x01);           // SWRESET
  HAL_Delay(150);

  ST7796_WriteCmd(0x11);           // SLPOUT
  HAL_Delay(120);

  uint8_t pixfmt = 0x55;           // 16-bit RGB565
  ST7796_WriteCmd(0x3A);
  ST7796_WriteData(&pixfmt, 1);

  // MADCTL (0x36) - orientation / RGB order
  // Typical: 0x48 or 0x28 depending on panel (RGB/BGR + row/col exchange)
  uint8_t madctl = 0x48;           // start with this; if colors swapped try 0x40/0x08 toggles
  ST7796_WriteCmd(0x36);
  ST7796_WriteData(&madctl, 1);

  ST7796_WriteCmd(0x21);           // INVON (often improves colors; if weird, try 0x20)
  HAL_Delay(10);

  ST7796_WriteCmd(0x29);           // DISPON
  HAL_Delay(50);


  ST7796_WriteCmd(0xB2); // Porch control
  uint8_t porch[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
  ST7796_WriteData(porch, 5);

  ST7796_WriteCmd(0xB7); // Gate control
  uint8_t gate = 0x35;
  ST7796_WriteData(&gate, 1);

  ST7796_WriteCmd(0xBB); // VCOM
  uint8_t vcom = 0x28;
  ST7796_WriteData(&vcom, 1);

  ST7796_WriteCmd(0xC0); // LCM control
  uint8_t lcm = 0x2C;
  ST7796_WriteData(&lcm, 1);

  ST7796_WriteCmd(0xC2); // VDV/VRH enable
  uint8_t vdv_en = 0x01;
  ST7796_WriteData(&vdv_en, 1);

  ST7796_WriteCmd(0xC3); // VRH set
  uint8_t vrh = 0x13;
  ST7796_WriteData(&vrh, 1);

  ST7796_WriteCmd(0xC4); // VDV set
  uint8_t vdv = 0x20;
  ST7796_WriteData(&vdv, 1);

  ST7796_WriteCmd(0xC6); // Frame rate
  uint8_t fr = 0x0F;
  ST7796_WriteData(&fr, 1);

}



static void ST7796_SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
//   uint8_t data[4];

//   ST7796_WriteCmd(0x2A); // CASET
//   data[0] = x0 >> 8; data[1] = x0 & 0xFF;
//   data[2] = x1 >> 8; data[3] = x1 & 0xFF;
//   ST7796_WriteData(data, 4);

//   ST7796_WriteCmd(0x2B); // RASET
//   data[0] = y0 >> 8; data[1] = y0 & 0xFF;
//   data[2] = y1 >> 8; data[3] = y1 & 0xFF;
//   ST7796_WriteData(data, 4);

//   ST7796_WriteCmd(0x2C); // RAMWR

  uint8_t data[4];

  ST7796_WriteCmd(0x2A);
  data[0] = x0 >> 8; data[1] = x0;
  data[2] = x1 >> 8; data[3] = x1;
  ST7796_WriteData_DMA(data, 4);
  while (st7796_dma_busy);

  ST7796_WriteCmd(0x2B);
  data[0] = y0 >> 8; data[1] = y0;
  data[2] = y1 >> 8; data[3] = y1;
  ST7796_WriteData_DMA(data, 4);
  while (st7796_dma_busy);

  ST7796_WriteCmd(0x2C);


}

void ST7796_FillColor(uint16_t color565)
{
  const uint16_t W = 320; // or 480 depending on your wiring/orientation
  const uint16_t H = 480;

  ST7796_SetAddrWindow(0, 0, W-1, H-1);

  // stream pixels
  CS_L();
  DC_DATA();

  uint8_t hi = color565 >> 8, lo = color565 & 0xFF;
  for (uint32_t i = 0; i < (uint32_t)W * H; i++) {
    uint8_t px[2] = {hi, lo};
    HAL_SPI_Transmit(&ST7796_SPI, px, 2, HAL_MAX_DELAY);
  }

  CS_H();
}



#define PIX_BUF_SZ  512  // bytes (256 pixels)

static uint8_t pixbuf[PIX_BUF_SZ];


static void ST7796_PrepareColor(uint16_t color)
{
  for (uint32_t i = 0; i < PIX_BUF_SZ; i += 2) {
    pixbuf[i]   = color >> 8;
    pixbuf[i+1] = color & 0xFF;
  }
}


void ST7796_Fill(uint16_t color)
{
  ST7796_SetAddrWindow(0, 0, 319, 479);

  ST7796_PrepareColor(color);

  uint32_t total = 320UL * 480UL * 2;
  while (total) {
    uint32_t chunk = (total > PIX_BUF_SZ) ? PIX_BUF_SZ : total;
    ST7796_WriteData_DMA(pixbuf, chunk);
    while (st7796_dma_busy);
    total -= chunk;
  }
}
