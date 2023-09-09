# 2023-06-29_esp-idf-Rotary-Encoder-Example-Notes

## [esp-idf/examples/peripherals/pcnt/rotary_encoder/](https://github.com/espressif/esp-idf/tree/v4.3/examples/peripherals/pcnt/rotary_encoder)

esp-idf Rotary Encoder Example hellowolrd

observations:

first attempt at integrated rotary knob w esp32 resulted in erratic/unexpected output
- numbers wobble when manually moving knob cable assembly, reminds me of non-common ground behavior
- manually twisting knob causes output values to rapidly latch up to KNOB_MAX or KNOB_MIN - then latch there. counterotations do not reduce number

prior setup:

- tried two 3rd party rotary encoder package 
  - https://github.com/madhephaestus/ESP32Encoder
  - https://github.com/igorantolic/ai-esp32-rotary-encoder
- platformio esp32dev w/ arduino framework
- tried to INPUT_PULLUP GPIO pins going to knob ```pinMode(rotary1, INPUT_PULLUP);```

---

goal: isolate source of faulty behavior.

approach: try minimal reference implementation of pcnt rotary controller example from espressiv esp-idf: ttps://github.com/espressif/esp-idf/tree/v4.3/examples/peripherals/pcnt/rotary_encoder

setup dev environment w/ vscode & platformio based on following tutorial: 
- https://docs.platformio.org/en/latest/tutorials/espressif32/espidf_debugging_unit_testing_analysis.html
  - didn't work off - compile errors
  - fixed partly by turning off default gcc warnings in `platformio.ini` by adding line
    - ```build_unflags = -Werror=all```
- see https://docs.platformio.org/en/latest/frameworks/espidf.html

errors when trying to build pio esp-idf example (https://docs.platformio.org/en/latest/tutorials/espressif32/espidf_debugging_unit_testing_analysis.html#id2)

```shell
 *  The terminal process "platformio 'run'" terminated with exit code: 1. 
 *  Terminal will be reused by tasks, press any key to close it. 

 *  Executing task: platformio run 

Processing esp32dev (platform: espressif32; board: esp32dev; framework: espidf)
---------------------------------------------------------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/esp32dev.html
PLATFORM: Espressif 32 (6.3.1) > Espressif ESP32 Dev Module
HARDWARE: ESP32 240MHz, 320KB RAM, 4MB Flash
DEBUG: Current (cmsis-dap) External (cmsis-dap, esp-bridge, esp-prog, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa)
PACKAGES: 
 - framework-espidf @ 3.50002.230601 (5.0.2) 
 - tool-cmake @ 3.16.4 
 - tool-esptoolpy @ 1.40501.0 (4.5.1) 
 - tool-ninja @ 1.9.0 
 - toolchain-esp32ulp @ 1.23500.220830 (2.35.0) 
 - toolchain-xtensa-esp32 @ 11.2.0+2022r1
Reading CMake configuration...
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 0 compatible libraries
Scanning dependencies...
No dependencies
Building in release mode
Compiling .pio/build/esp32dev/src/main.o
In file included from src/main.c:18:
src/main.c: In function 'wifi_event_handler':
src/main.c:36:30: error: expected ')' before 'MACSTR'
   36 |     ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
      |                              ^~~~~~
/Users/100ideas/.platformio/packages/framework-espidf/components/log/include/esp_log.h:282:88: note: in definition of macro 'LOG_FORMAT'
  282 | #define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%" PRIu32 ") %s: " format LOG_RESET_COLOR "\n"
      |                                                                                        ^~~~~~
/Users/100ideas/.platformio/packages/framework-espidf/components/log/include/esp_log.h:432:41: note: in expansion of macro 'ESP_LOG_LEVEL'
  432 |         if ( LOG_LOCAL_LEVEL >= level ) ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
      |                                         ^~~~~~~~~~~~~
/Users/100ideas/.platformio/packages/framework-espidf/components/log/include/esp_log.h:342:38: note: in expansion of macro 'ESP_LOG_LEVEL_LOCAL'
  342 | #define ESP_LOGI( tag, format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO,    tag, format, ##__VA_ARGS__)
      |                                      ^~~~~~~~~~~~~~~~~~~
src/main.c:36:5: note: in expansion of macro 'ESP_LOGI'
   36 |     ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
      |     ^~~~~~~~
```

possible solution: 
- update espressif32 https://community.platformio.org/t/esp-idf-pyparsing-expecting-end-of-text/23562/10
- open shell with platformio command (sets up env)

```shell
espidf-examples-rotary_encoder on  main [!?] via △ v3.26.4 
❯ pio pkg -h
Usage: pio pkg [OPTIONS] COMMAND [ARGS]...

Options:
  -h, --help  Show this message and exit.

Commands:
  exec       Run command from package tool
  install    Install the project dependencies or custom packages
  list       List installed packages
  outdated   Check for outdated packages
  pack       Create a tarball from a package
  publish    Publish a package to the registry
  search     Search for packages
  show       Show package information
  uninstall  Uninstall the project dependencies or custom packages
  unpublish  Remove a pushed package from the registry
  update     Update the project dependencies or custom packages

espidf-examples-rotary_encoder on  main [!?] via △ v3.26.4 
❯ pio pkg outdated
Checking  [####################################]  100%          

Semantic Versioning color legend:
<Major Update>  backward-incompatible updates
<Minor Update>  backward-compatible features
<Patch Update>  backward-compatible bug fixes

Package      Current    Wanted    Latest    Type      Environments
-----------  ---------  --------  --------  --------  --------------
espressif32  6.3.1      6.3.2     6.3.2     Platform  esp32dev

espidf-examples-rotary_encoder on  main [!?] via △ v3.26.4 took 10s 
❯ pio pkg update
Resolving esp32dev dependencies...
Platform Manager: Updating espressif32 @ 6.3.1
Platform Manager: Removing espressif32 @ 6.3.1
Platform Manager: espressif32@6.3.1 has been removed!
Platform Manager: Installing platformio/espressif32 @ 6.3.2
Downloading  [####################################]  100%          
Unpacking  [####################################]  100%
Platform Manager: espressif32@6.3.2 has been installed!
```

esp-idf v5.1 source code for example code used above from platformio
https://github.com/espressif/esp-idf/blob/release/v5.1/examples/wifi/getting_started/softAP/main/softap_example_main.c

platform.io documentation (uses outdated esp-idf not compat w v5.1)
https://docs.platformio.org/en/latest/tutorials/espressif32/espidf_debugging_unit_testing_analysis.html#id2

esp-idf v5 migration guide explains breaking changes in this case
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/5.0/system.html#esp-system

does it build? **YES**

OK! next:
- [ ] try espidf knob example via cutnpaste
- [ ] try out esp registry components
  - [ ] knob
  - [ ] led_indicator
  - [ ] led_strip
- [ ] maybe open PR on outdated platformio doc example code







---

# esp components
lots of interesting additional official esp examples at https://github.com/espressif/idf-extra-components
- online search at https://components.espressif.com/components?q=target:esp32
- "*This repository is used only for Espressif components, it is not mean to store third party components.*"
- docs https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html

led_strip
- https://components.espressif.com/components/espressif/led_strip

led_indicator
- https://components.espressif.com/components/espressif/led_indicator
- docs https://docs.espressif.com/projects/espressif-esp-iot-solution/en/latest/esp32/display/led_indicator.html
- #TODO figure out GPIO of onboard LED and drive via led_indicator to indicate state

knob
- https://components.espressif.com/components/espressif/knob

Asio 
- https://components.espressif.com/components/espressif/asio
- https://think-async.com/Asio/
- "*Asio is a cross-platform C++ library for network and low-level I/O programming that provides developers with a consistent asynchronous model using a modern C++ approach.*"
- study the examples 
  - https://think-async.com/Asio/asio-1.28.0/doc/asio/examples/cpp03_examples.html
  - mostly used for ip-networking distributed computation?

libcoap
- https://components.espressif.com/components/espressif/coap
- https://libcoap.net 
- https://coap.space
- rfc7252 CoAP
- https://datatracker.ietf.org/doc/html/rfc7252 
  > The Constrained Application Protocol (CoAP) is a specialized web
  > transfer protocol for use with constrained nodes and constrained
  > (e.g., low-power, lossy) networks.  The nodes often have 8-bit
  > microcontrollers with small amounts of ROM and RAM, while constrained
  > networks such as IPv6 over Low-Power Wireless Personal Area Networks
  > (6LoWPANs) often have high packet error rates and a typical
  > throughput of 10s of kbit/s.  The protocol is designed for machine-
  > to-machine (M2M) applications such as smart energy and building
  > automation.
  > 
  > CoAP provides a request/response interaction model between
  > application endpoints, supports built-in discovery of services and
  > resources, and includes key concepts of the Web such as URIs and
  > Internet media types.  CoAP is designed to easily interface with HTTP
  > for integration with the Web while meeting specialized requirements
  > such as multicast support, very low overhead, and simplicity for
  > constrained environments.


esp-now
- https://components.espressif.com/components/espressif/esp-now

mesh-lite
- https://components.espressif.com/components/espressif/mesh_lite

arduino-core
- https://components.espressif.com/components/espressif/arduino-esp32