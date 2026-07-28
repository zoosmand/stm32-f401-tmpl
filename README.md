# STM32F401 Health Check

This project is a firmware platform for a standalone network health-check
device based on the STM32F401RCT6 microcontroller.

Its goal is to monitor an Internet resource over HTTP, determine its
availability from the HTTP response status, and present the result locally.
The display provides status and diagnostic output, while the touchscreen
allows the device to receive user input.

The F401 prototype checks `hvm-a.ic.local:3000` once per minute. It resolves
the hostname, sends `HEAD /`, and considers only HTTP status `200` healthy.
DNS, socket, connection, timeout, malformed-response, and non-200 results are
failures.

## Current functionality

- FreeRTOS-based application structure with statically allocated tasks
- ST7796 TFT display driver
- Display-backed `printf()` output
- FT6336U capacitive touchscreen support
- Touch-event processing service
- W5500 Ethernet controller support
- DHCP and DNS client code
- W25Q64 SPI NOR flash driver with startup self-test
- LSE-backed hardware RTC synchronized with `pool.ntp.org`
- Queued passive-buzzer tone generation and startup self-test
- Periodic HTTP resource health monitoring
- Persistent CRC-protected HTTP result log in W25Q64 NOR flash

## HTTP health check

The HTTP monitor uses W5500 socket 4. DNS access is serialized with the SNTP
service because the WIZnet DNS client keeps shared internal state.

Each result is appended to a power-loss-tolerant circular log. A record
contains its sequence number, RTC UTC calendar value when available, resolved
IPv4 address, HTTP status, and result category. The log occupies sectors 0
through 14 of the final 64 KB flash block. Sector 15 remains reserved
exclusively for the startup flash self-test.

## Hardware

- STM32F401RCT6 microcontroller
- ST7796 TFT display on SPI1
- FT6336U touchscreen controller on I2C1
- W5500 Ethernet controller on SPI3
- Winbond W25Q64 flash memory on SPI2
- 32.768 kHz LSE crystal for the STM32 RTC
- Passive buzzer on PA8 through a 2N2222 transistor

## Buzzer

TIM1 channel 1 generates a 50% duty-cycle PWM signal on PA8. The output drives
the base circuit of a 2N2222 transistor rather than powering the passive buzzer
directly from the MCU pin.

The FreeRTOS buzzer service accepts finite-duration tone requests through a
static queue, allowing callers to continue without waiting for a tone to end.
At startup it plays a 2 kHz, 200 ms confirmation tone and then leaves the
transistor switched off.

## Network time

The hardware RTC stores UTC and retains its last valid value in the backup
domain across ordinary resets. After the W5500 network is ready, a FreeRTOS
service resolves `pool.ntp.org`, requests the current time over SNTP, and
updates the RTC only after receiving a valid response.

Failed DNS or SNTP requests leave the current RTC value unchanged and are
retried after one minute. A successful clock is synchronized again every six
hours to limit drift. Local time-zone conversion is intentionally left to the
presentation layer. If the LSE crystal cannot start, the time service reports
the failure and remains inactive without stopping the rest of the device.

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
