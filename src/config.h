#pragma once
#include <cstdint>

/*
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

const static uint8_t PIN_RADIO_CE = 4;
const static uint8_t PIN_RADIO_CSN = 5;
const static uint8_t PIN_RADIO_MOSI = 23;
const static uint8_t PIN_RADIO_MISO = 19;
const static uint8_t PIN_RADIO_SCK = 18;
const int CLOCKPIN = 14;
const int DATAPIN = 13;
const int buttonPin = 39;
const uint8_t rotary1 = 34;
const uint8_t rotary2 = 36;
