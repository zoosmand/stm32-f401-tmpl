# STM32F401 Health Check

This project is a firmware platform for a standalone network health-check
device based on the STM32F401RCT6 microcontroller.

Its goal is to monitor an Internet resource over HTTPS, determine its
availability from the HTTP response status, and present the result locally.
The display provides status and diagnostic output, while the touchscreen
allows the device to receive user input.

The project is currently at the platform stage. It provides the hardware
drivers and FreeRTOS services needed before HTTPS monitoring is added.

## Current functionality

- FreeRTOS-based application structure with statically allocated tasks
- ST7796 TFT display driver
- Display-backed `printf()` output
- FT6336U capacitive touchscreen support
- Touch-event processing service
- W5500 Ethernet controller support
- DHCP and DNS client code
- W25Q64 SPI NOR flash driver with startup self-test

## Hardware

- STM32F401RCT6 microcontroller
- ST7796 TFT display on SPI1
- FT6336U touchscreen controller on I2C1
- W5500 Ethernet controller on SPI3
- Winbond W25Q64 flash memory on SPI2

## Project structure

- `Core/` — application entry point, STM32 HAL configuration, and interrupts
- `Periph/` — display, touchscreen, and font drivers
- `Srv/` — FreeRTOS application services
- `Ethernet/` — W5500 driver and network protocols
- `FreeRTOS-Kernel/` — FreeRTOS kernel sources
- `docs/` — project conventions and supporting documentation

## Build

The firmware is built with the GNU Arm Embedded toolchain:

```sh
make -j4
```

The generated ELF, HEX, and BIN files are placed in the `build/` directory.

## References

- [STM32F401 SVD description](https://github.com/tinygo-org/stm32-svd/blob/main/svd/stm32f401.svd)
- [ST7796 initialization reference](https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ST7796_Init.h)

---

&copy; 2017-2026 Askug Ltd., Dmitry Slobodchikov
