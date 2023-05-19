#include <Arduino.h>
#include <FastLEDHub.h>
#include <SPI.h>
#include "Animations/Spectrogram.h"

// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
#define VSPI_MISO   19
#define VSPI_MOSI   23  // apa102 logic input ("Master Out Slave In")
#define VSPI_SCLK   18  // apa102 clock input
#define VSPI_SS     5
// #define FASTLED_ALL_PINS_HARDWARE_SPI
// #define FASTLED_ESP32_SPI_BUS VSPI

#define NUM_LEDS 100
// #define LIGHTSTRIP_PIN 5
// #define LED_TYPE WS2812B
#define LED_TYPE DOTSTAR
// #define DATAPIN 13
// #define CLOCKPIN 14

CRGB leds[NUM_LEDS];

void setup()
{
  FastLEDHub.initialize("Spectrogram Example");
  // FastLEDHub.addLeds<LED_TYPE, LIGHTSTRIP_PIN, GRB>(leds, NUM_LEDS);
  FastLEDHub.addLeds<LED_TYPE, VSPI_MOSI, VSPI_SCLK, BGR>(leds, NUM_LEDS);

  FastLEDHub.registerAnimation(new Spectrogram("Spectrogram"));
}

void loop()
{
  FastLEDHub.handle();
}