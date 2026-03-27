#pragma once
#include <cstdint>

// this config works for liteswarm1_esp32-devkit-1_doit_v0.1.0 PCB as of 2024-03-28

/* #TODO OUT OF DATE 2023-08-03
now set in main.cpp ~L79

ezsbc on blue perf board (white wire tie corner):
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

// #define NUMPIXELS 75
// ray wu braided nylon WS2812B 50 pixels / meter 
// https://www.aliexpress.us/item/3256805646893529.html
// #define NUMPIXELS 250 // - nylon, 5m
// #define NUMPIXELS 125 // - nylon, cut in half into 2x 2.5m
// #define NUMPIXELS 75 // smallest round rope rope
// #define NUMPIXELS 40 // - strand of balls 12v
#define NUMPIXELS 144 // macs apa102 2-meter v0.5 scarf // ~24" COB
#define FRAMES_PER_SECOND 60
// #define LED_CLOCK 13
// #define LED_DATA 12

#define LEDTYPE_WS28x // sets FastLED for WS2811 WS2812 WS2812B leds (note serial update slow clock limits led length vs framerate)


// TODO use these instead of hardcoading in main
// #define PIN_RADIO_CE 4
// #define PIN_RADIO_CSN 5
// #define PIN_RADIO_MOSI 23
// #define PIN_RADIO_MISO 19
// #define PIN_RADIO_SCK 18


// PCB v liteswarm1-20240404_v0.1.0 - BROKEN
// BAD DO NOT USE GPIX (input-only) pins for buttons/knobs
// INTERNAL PULL-UP Resistors not present, will be nousy w/o
// external LC filter
//
// USE FOR BATTERY Board
// #define LEDTYPE_APA102 true
// #define buttonPin 34
// #define rotary1 36
// #define rotary2 39


// PCB v liteswarm1-20240404_v0.1.0 - BROKEN
// USE with manually fixed PCBs
//
// knob 1 (top) - v0.1.1
// on this pcb version this knob req 3 rework wires
// #define buttonPin 14
// #define rotary1 27
// #define rotary2 26

// // knob 2 (side) - v0.1.1 
// // this one only needs one, so using it
// #define buttonPin 33 
// #define rotary1 25
// #define rotary2 32



// TODO setup PCB revision pinout IFDEF blocks to make it easy to program diff hw (see previous commit for other pinouts)

// handmade esp32-devkit-1 (pre-liteswarm1 pcb)
// ezSBC
// #define buttonPin 21
// #define rotary1 17
// #define rotary2 16
// #define LED_CLOCK 18
// #define LED_DATA 23

// DOIT esp32 devkit 1
#define buttonPin 21
#define rotary1 17
#define rotary2 16
#define LED_CLOCK 18
#define LED_DATA 23