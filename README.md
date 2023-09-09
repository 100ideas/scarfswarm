# scarfswarm

## 2023-09-08 NOTE
currently, `master` is based on `BM23` branch (specifically tag [BM23](https://github.com/100ideas/scarfswarm/tree/refs/tags/BM23)), which was refactored from [the 2019-july master branch of this  repo counterbeing/liteswarm, somewhere around here](https://github.com/counterbeing/liteswarm/blob/dc4e0e53ea9b77c02f954a00a81d340e21b89678/src/main.cpp) to work with esp32. BM23 branch compiles on esp32 dev boards and works with the with nrf24 and rotary encoder knobs used in the original project, just attached to esp32 GPIO pins as defined in [`/src/config.h`](src/config.h). 

The GPIO pin settings should be compatible with ESP-WROOM-32 DevKit-v1 style breakout boards. I used these "[5PCS ESP-WROOM-32 ESP32 ESP-32S Development Board...](https://www.amazon.com/dp/B0BK13HWBJ) from amazon, but I want to switch to using one of the boards made by [Unexpected Maker](https://unexpectedmaker.com/shop.html) (tbd)."

## about
continutation of code developed with @counterbeing in https://github.com/counterbeing/liteswarm. `BM23` (and `master` currently) branch here is a minimal refactor of the teensy/arduino-compatible master branch of the liteswarm repo from 2019. Multiple contributors attempted to refactor or rearchitect the liteswarm code for esp32 but I couldn't get any of those branches to run. This branch does run on esp32, but it is not fancy. In fact, the code is ugly. But it works. I will push it to a branch on counterbeing/liteswarm for posterity. This repo is intended to be the home of scarfswarm FOSS firmware and hardware based on esp32, contributed by 100ideas and others.

To compile and upload this code onto an ESP32, use platformio in vscode. Once the platformio extension is activated, it should automagically configure a local gcc toolchain, download the project dependencies (`fastled/FastLED@^3.5.0`, `dparson55/NRFLite@^3.0.3`, `madhephaestus/ESP32Encoder`, `thomasfredericks/Bounce2@^2.55`, and all the magic contained in the `framework = arduino` line of `platformio.ini`), and compile them for esp32. If you encounter trouble building, try disabling `gcc` warnings by using a `platform.ini` similar to below (specifically `build_unflags` line):

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = espidf
upload_port = /dev/cu.SLAB*
monitor_port = /dev/cu.SLAB_USBtoUART
monitor_speed = 115200
; esp32_exception_decoder is an add-on that parses kernel panic messages on serial such that error codes
;   and memory locations / program counter numbers are replaces with src code coordinates - handy!
monitor_filters = default, esp32_exception_decoder
; fix vendor-default extreme error setting in GCC (example code won't compile)
;   by disabling all gcc warnings
;   https://community.platformio.org/t/disable-warning-errors/13280/3
build_unflags = -Werror=all
; in extreme cases you may need to mess with how the linker/compiler resolves dependencies, try:
; lib_ldf_mode = chain+
```
