#pragma once
#include <cstdint>

// this config works for liteswarm1_esp32-devkit-1_doit_v0.1.0 PCB as of 2024-03-28

/* #TODO OUT OF DATE 2023-08-03

Radio    ESP32 module
CE    -> 4
CSN   -> 5
MOSI  -> 23
MISO  -> 19
SCK   -> 18
IRQ   -> No connection
VCC   -> No more than 3.6 volts
GND   -> GND
*/

#define NUMPIXELS 75
// ray wu braided nylon WS2812B 50 pixels / meter 
// https://www.aliexpress.us/item/3256805646893529.html
// #define NUMPIXELS 250 // - nylon
// #define NUMPIXELS 75 // smallest round rope rope
#define FRAMES_PER_SECOND 60
#define LED_CLOCK 13
#define LED_DATA 12

// TODO use these instead of hardcoading in main
// #define PIN_RADIO_CE 4
// #define PIN_RADIO_CSN 5
// #define PIN_RADIO_MOSI 23
// #define PIN_RADIO_MISO 19
// #define PIN_RADIO_SCK 18

#define buttonPin 34
#define rotary1 36
#define rotary2 39


// TODO setup PCB revision pinout IFDEF blocks to make it easy to program diff hw (see previous commit for other pinouts)

// handmade esp32-devkit-1 (pre-liteswarm1 pcb)
// encoder_knob.attachHalfQuad(17, 16);
// #define buttonPin 21
// #define rotary1 17
// #define rotary2 16