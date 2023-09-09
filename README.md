# scarfswarm

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