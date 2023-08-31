Add hardware SPI support on ESP32 #1047
https://github.com/FastLED/FastLED/pull/1047#issuecomment-1664495804


@gitTinker, I think the pins you are using for SPI do not interface with the higher-speed `IO_MUX` bus. Yes you can define non-default SPI pins, but if any of them DO NOT connect with the `IO_MUX` bus then the entire SPI channel will run via the `GPIO matrix` which puts more load on the CPU core ("software mode"). You can get better performance by choosing a set of pins for your SPI connection that are all supported by `IO_MUX` (maybe worthwhile if you are driving a lot of LEDs or reacting to interrupts).

> GPIO Matrix and IO_MUX
> 
> Most of ESP32’s peripheral signals have a direct connection to their dedicated IO_MUX pins. However, the signals can also be routed to any other available pins using the less direct > GPIO matrix. If at least one signal is routed through the GPIO matrix, then all signals will be routed through it.
> 
> The GPIO matrix introduces flexibility of routing but also brings the following disadvantages:
> - Increases the input delay of the MISO signal, which makes MISO setup time violations more likely. If SPI needs to operate at high speeds, use dedicated IO_MUX pins.
> - Allows signals with clock frequencies only up to 40 MHz, as opposed to 80 MHz if IO_MUX pins are used.
>
> source: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html#gpio-matrix-and-io-mux

It's kinda tedious to figure out what pins are supported by IO_MUX, what their names are, and how accessable they are on each particular esp32 board. This header file is helpful [`esp-idf/components/soc/esp32/include/soc/spi_pins.h`](https://github.com/espressif/esp-idf/blob/master/components/soc/esp32/include/soc/spi_pins.h) (notice legacy naming conventions):

```C
#pragma once

#define SPI_FUNC_NUM            1
#define SPI_IOMUX_PIN_NUM_MISO  7
#define SPI_IOMUX_PIN_NUM_MOSI  8
#define SPI_IOMUX_PIN_NUM_CLK   6
#define SPI_IOMUX_PIN_NUM_CS    11
#define SPI_IOMUX_PIN_NUM_WP    10
#define SPI_IOMUX_PIN_NUM_HD    9

//For D2WD and PICO-D4 chip
#define SPI_D2WD_PIN_NUM_MISO  17
#define SPI_D2WD_PIN_NUM_MOSI  8
#define SPI_D2WD_PIN_NUM_CLK   6
#define SPI_D2WD_PIN_NUM_CS    16
#define SPI_D2WD_PIN_NUM_WP    7
#define SPI_D2WD_PIN_NUM_HD    11

#define SPI2_FUNC_NUM           HSPI_FUNC_NUM
#define SPI2_IOMUX_PIN_NUM_MISO HSPI_IOMUX_PIN_NUM_MISO
#define SPI2_IOMUX_PIN_NUM_MOSI HSPI_IOMUX_PIN_NUM_MOSI
#define SPI2_IOMUX_PIN_NUM_CLK  HSPI_IOMUX_PIN_NUM_CLK
#define SPI2_IOMUX_PIN_NUM_CS   HSPI_IOMUX_PIN_NUM_CS
#define SPI2_IOMUX_PIN_NUM_WP   HSPI_IOMUX_PIN_NUM_WP
#define SPI2_IOMUX_PIN_NUM_HD   HSPI_IOMUX_PIN_NUM_HD

#define SPI3_FUNC_NUM           VSPI_FUNC_NUM
#define SPI3_IOMUX_PIN_NUM_MISO VSPI_IOMUX_PIN_NUM_MISO
#define SPI3_IOMUX_PIN_NUM_MOSI VSPI_IOMUX_PIN_NUM_MOSI
#define SPI3_IOMUX_PIN_NUM_CLK  VSPI_IOMUX_PIN_NUM_CLK
#define SPI3_IOMUX_PIN_NUM_CS   VSPI_IOMUX_PIN_NUM_CS
#define SPI3_IOMUX_PIN_NUM_WP   VSPI_IOMUX_PIN_NUM_WP
#define SPI3_IOMUX_PIN_NUM_HD   VSPI_IOMUX_PIN_NUM_HD

//Following Macros are deprecated. Please use the Macros above
#define HSPI_FUNC_NUM           1
#define HSPI_IOMUX_PIN_NUM_MISO 12
#define HSPI_IOMUX_PIN_NUM_MOSI 13
#define HSPI_IOMUX_PIN_NUM_CLK  14
#define HSPI_IOMUX_PIN_NUM_CS   15
#define HSPI_IOMUX_PIN_NUM_WP   2
#define HSPI_IOMUX_PIN_NUM_HD   4

#define VSPI_FUNC_NUM           1
#define VSPI_IOMUX_PIN_NUM_MISO 19
#define VSPI_IOMUX_PIN_NUM_MOSI 23
#define VSPI_IOMUX_PIN_NUM_CLK  18
#define VSPI_IOMUX_PIN_NUM_CS   5
#define VSPI_IOMUX_PIN_NUM_WP   22
#define VSPI_IOMUX_PIN_NUM_HD   21
```

So looking at the espressif docs for [ESP-PICO-KIT-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-pico-kit-1.html#header-j2), [ESP-PICO-KIT-V4](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-pico-kit.html#header-j2) pin description list, it indicates that

| NAME | Function |
| ------ | ---------- |
| IO14 | ADC2_CH6, TOUCH6, RTC_GPIO16, MTMS, **HSPICLK**, HS2_CLK, SD_CLK, EMAC_TXD2 |
| IO12 | ADC2_CH5, TOUCH5, RTC_GPIO15, MTDI (See 3), **HSPIQ**, HS2_DATA2, SD_DATA2, EMAC_TXD3 |
| IO13 | ADC2_CH4, TOUCH4, RTC_GPIO14, MTCK, **HSPID**, HS2_DATA3, SD_DATA3, EMAC_RX_ER | 
| IO15 | ADC2_CH3, TOUCH3, RTC_GPIO13, MTDO, **HSPICS0**, HS2_CMD, SD_CMD, EMAC_RXD3 |
| ...  |  ... |

the venerable @kolban asked what these unusual Qs and Ds indicate on an esp32 forum (see bottom) and the response was: 

```
"GPIO pin for spi_d (=MOSI), spi_q (=MISO) signal". 
```

So then, if you wanted to try IO_MUX enabled SPI definitions, you might try:

```C
#define HSPI_MISO   12
#define HSPI_MOSI   13    #this is the only IO pin used in this example (master out, slave in) 
#define HSPI_SCLK   14
#define HSPI_SS     15
```

I could have this backwards, I've just been doing a lot of research into how to run dual `IO_MUX` enabled SPI channels in parallel. So far I think this is possible by using the designated `HSPI/SPI3` and `VSPI/SPI3` GPIO groups.

One thing I'm not sure about is what happens if `SPI2` and `SPI3` channels are run on the same cpu core? I think I read in the espressif docs that it can work, but each SPI channel needs to be run in a separate RTOS task...

---

**Regarding espressif's confusing new SPI names (`VSPIQ`, `HSPID`, `HSPIWP` etc):**
(quoting from https://esp32.com/viewtopic.php?t=918)

> kolban » Tue Jan 10, 2017 5:24 am:
>
> Studying the SPI master driver, I find that we must configure a data structure called "spi_bus_config_t". Reading the documentation on it found here:
> 
> [http://esp-idf.readthedocs.io/en/latest ... s_config_t](http://esp-idf.readthedocs.io/en/latest/api/spi_master.html#_CPPv216spi_bus_config_t)
> 
> I find 5 pins need be configured:
> ```
> spid_io_num
> spiq_io_num
> spiclk_io_num
> spiwp_io_num
> spihd_io_num
> ```
> Here comes the list of questions related to these names:
> 
> 1) What does "spi_d" mean? I understand the SPI concept of "MOSI" ... but the concept of "d" means nothing to me.
> 2) What does "spi_q" mean? I understand the SPI concept of "MISO" ... but the concept of "q" means nothing to me.
> 3) What does "spi_wp" mean? I can't relate that to anything I am familiar with in the concept of SPI.
> 4) What does "spi_hd" mean? I can't relate that to anything I am familiar with in the concept of SPI.
>
> .....................
> 
> ESP_Sprite » Tue Jan 10, 2017 6:21 am:
>
> The SPI flash peripheral actually was initially created to interface with SPI flash chips, which have a d, q, clk, wp and hd pin. I can imagine the SPI port pins are somewhat weird if you aren't familiar with that.
> 
> For MISO and MOSI, it is explained in the page you quoted:
> ```
> int spid_io_num
> 
> GPIO pin for spi_d (=MOSI)signal, or -1 if not used.
> 
> int spiq_io_num
> 
> GPIO pin for spi_q (=MISO) signal, or -1 if not used.
> ```
> spi_hd is the hold pin pf the flash chip, while spi_wp is for write protect. These pins aren't really used in the 1-bit SPI mode you'd normally use, but the 4-bit mode uses them as > data lines.
> 
> I agree that the documentation can be slightly better wrt explaining this.