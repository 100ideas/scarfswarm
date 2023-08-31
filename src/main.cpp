#include "SPI.h"
#include <Arduino.h>
#include <NRFLite.h>
// #include <ESP32Encoder.h>
// #include <Bounce2.h>
#include "MyKnob.h"

MyKnob knob;

///////////////////////////////////////////////////////////////////////////////////////////
// init FastLED
/* FastLED ESP32 Hardware SPI Driver
 * https://github.com/FastLED/FastLED/blob/master/src/platforms/esp/32/fastspi_esp32.h
 *
 * This hardware SPI implementation can drive clocked LEDs from either the
 * VSPI or HSPI bus (aka SPI2 & SPI3). No support is provided for SPI1, because it is 
 * shared among devices and the cache for data (code) in the Flash as well as the PSRAM.
 *
 * To enable the hardware SPI driver, add the following line *before* including
 * FastLED.h:
 *
 * #define FASTLED_ALL_PINS_HARDWARE_SPI
 *
 * This driver uses the VSPI bus by default (GPIO 18, 19, 23, & 5). To use the 
 * HSPI bus (GPIO 14, 12, 13, & 15) add the following line *before* including
 * FastLED.h:
 * 
 * #define FASTLED_ESP32_SPI_BUS HSPI
 * 
 */

// may not be necessary, but FastLED.h kicks off an include to led_sysdefs.h, which checks for ESP32,
// then includes https://github.com/FastLED/FastLED/blob/master/src/led_sysdefs.h#L41
#ifndef ESP32
#define ESP32
#endif

#define FASTLED_ALL_PINS_HARDWARE_SPI // <-- must be defined BEFORE FastLED.h included
#define FASTLED_ESP32_SPI_BUS VSPI
#include <FastLED.h>

/* https://github.com/FastLED/FastLED/blob/master/src/platforms/esp/32/fastspi_esp32.h#L58C1-L61C30
 *  static uint8_t spiClk = 18;
 *  static uint8_t spiMiso = 19;
 *  static uint8_t spiMosi = 23;
 *  static uint8_t spiCs = 5;
 */
// #TODO want to find a clean way to pull these defs from where they are already defined in fastspi_esp32.h
#define LED_spiClk 18
#define LED_spiMosi 23
#define NUMPIXELS 60
#define FRAMES_PER_SECOND 60
CRGB leds[NUMPIXELS];

// #define buttonPin 21
// #define rotary1 17
// #define rotary2 16


///////////////////////////////////////////////////////////////////////////////////////////
// init radio
/* https://github.com/dparson55/NRFLite/blob/4e425d742ca8879d654a270c7c02c13440476e7a/examples/Basic_RX_ESP32/Basic_RX_ESP32.ino
 * 
 * Demonstrates simple RX operation with an ESP32.
 * Any of the Basic_TX examples can be used as a transmitter.
 * 
 * ESP's require the use of '__attribute__((packed))' on the RadioPacket data structure
 * to ensure the bytes within the structure are aligned properly in memory.
 * 
 * The ESP32 SPI library supports configurable SPI pins and NRFLite's mechanism to support this is shown.
*/
struct __attribute__((packed)) RadioPacket_NRFLiteExample // Note the packed attribute.
{
    uint8_t  SenderId;
    uint32_t OnTimeMillis;
    uint32_t FailedTxCount;
};

struct __attribute__((packed)) RadioPacket  // Any packet up to 32 bytes can be sent.
{                                            //  index[width]:bytes so far - 256 bits max packet size
  uint8_t  SHARED_SECRET;                    //  0[8]:1
  uint8_t  senderId;                         //  8[8]:1
  // TODO shouldn't encoderPosition be signed int16?
  uint32_t encoderPosition;                  //  16[32]:4
  // int32_t encoderPosition;                  //  16[32]:4
  uint8_t  animationId;                      //  48[8]:1
  // uint32_t keyframe;                      //  56[?]:1-25
                                             // 255[0]:0
};


NRFLite _radio;
RadioPacket _radioData;
RadioPacket_NRFLiteExample _radioDataExample;

// ezscb.com esp32 feather ~v1 SPI2/HSPI 
const static uint8_t PIN_RADIO_CE = 27;
const static uint8_t PIN_RADIO_CSN = 15;
const static uint8_t PIN_RADIO_MOSI = 13;
const static uint8_t PIN_RADIO_MISO = 12;
const static uint8_t PIN_RADIO_SCK = 14;
// PIN_RADIO_IRQ = 33

const static uint8_t RADIO_ID = (uint8_t)random();
const static uint8_t SHARED_RADIO_ID = 1; // from after litewarm master 3ea81e3f4b1211809066e6f9649927cf23428956
const static uint8_t SHARED_SECRET = 42;  // bikelight scarves use this & radio_id = 1



///////////////////////////////////////////////////////////////////////////////////////////
// init rotary encoder
// ESP32Encoder encoder;
// uint32_t encoderPosition = 0;

// Bounce2::Button button_debouncer = Bounce2::Button();



///////////////////////////////////////////////////////////////////////////////////////////
// setup
void setup() {
    // Initialize serial communication at 115200 bits per second:
    Serial.begin(115200);

    knob.setup();


    // FastLED
    // https://github.com/FastLED/FastLED/blob/master/src/FastLED.h#L246
    FastLED.addLeds<APA102, LED_spiMosi, LED_spiClk, BGR>(leds, NUMPIXELS);  // BGR ordering is typical
    FastLED.setBrightness(84);
    Serial.println("main.setup(): FastLED.addLeds() complete\n");
    fill_solid(leds, NUMPIXELS, CRGB::Green);
    FastLED.show();



    // nrf24 radio 
    // https://nrf24.github.io/RF24/index.html#autotoc_md45
    SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
    // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
    uint8_t callSpiBegin = 0;
    // init radio w common ID so all are in "broadcast" mode
    // TODO NRFLite::BITRATE250KBPS compat w/ bikelites, any improvements?
    if (!_radio.init(SHARED_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, callSpiBegin))
    {
        Serial.println("Cannot communicate with radio");
        // while (1); // Wait here forever.
    }
    Serial.println("main.setup(): _radio.init() complete");
    _radioData.senderId = RADIO_ID;
    _radioData.encoderPosition = 255; // 255 for now to indicate init but invalid
    _radioData.animationId = 255;     // 255 for now to indicate init but invalid
    _radioDataExample.FailedTxCount = 0;  // TODO remove
    // sanity check delay - allows reprogramming if accidently blowing power w/leds
    delay(1000);



    // CALL MyKnob.Setup()

    // Rotary Encoder Knob
    // https://github.com/madhephaestus/ESP32Encoder/blob/master/examples/Encoder/Encoder.ino
    // ESP32Encoder::useInternalWeakPullResistors=UP;
    // // use pin 19 and 18 for the first encoder
    // encoder.attachHalfQuad(17, 16);
    // // clear the encoder's raw count and set the tracked count to zero
    // encoder.clearCount();
    // Serial.println("Encoder Start = " + String((int32_t)encoder.getCount()));


    // button_debouncer.attach(buttonPin, INPUT_PULLUP);
    // button_debouncer.interval(25);



    Serial.printf("finished setup\n");
    // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}


void blink(){  // Turn the LED on, then pause
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
};



///////////////////////////////////////////////////////////////////////////////////////////
// main loop
// TODO refactor loop() so millis() inner-loop state not needed - ugly
uint32_t now = 0;
bool localUpdate;
bool weBlinkin = true;
int animIndex = 1;

void loop() {

  FastLED.show();
  knob.check(&animIndex);

  // tracking updates in superloop (not concurrent friendly!!!)
  localUpdate = false;
  // (millis() will always not always be called w/ sub-ms frequency)
  now = millis();
  
  // if(weBlinkin){
  //   blink();
  // };


  // #TODO refactor w/ Knob.h
  // currently encoder.getCount() can return negative and cause encoderPosition to roll over
  // 50 hz check if encoder position changed
  // if(!(now % 20)){
  //   if (encoderPosition != encoder.getCount()){
  //     // TODO caution, encoder getCount() may return SIGNED int64?
  //     // casting incorrectly (int32_t) caused lock up
  //     encoderPosition = (uint32_t)encoder.getCount(); 
  //     _radioData.encoderPosition = encoderPosition;
  //     localUpdate = true;
  //     Serial.println("\nEncoder count = " + String(encoderPosition) + "\n");
  //     // Serial.println("2. Encoder count = " + String((int32_t)encoder.getCount()));
  //   }
  // };

  // enter SEND mode AT MOST every ~5 sec or so (currently 5 sec heartbeat)
  if (now % 5000 == 0 || localUpdate)
  {
      String msg = "<== SENT [";
      msg += RADIO_ID;
      msg += "=>";
      msg += SHARED_RADIO_ID;
      msg += "]: ";
      msg += _radioData.animationId;
      msg += " -- ";
      msg += _radioData.encoderPosition; 
      Serial.println(msg);

      // Serial.print(" encoder.getCount(): ");
      // Serial.println(encoder.getCount());
      // Serial.print(" (int32_t)encoder.getCount(): ");
      // Serial.println((int32_t)encoder.getCount());
      
      // liteswarm#master/Radio.h#72
      // _radio.send(SHARED_RADIO_ID, &_outboundRadioPacket, sizeof(_outboundRadioPacket), NRFLite::NO_ACK);
      if (_radio.send(SHARED_RADIO_ID, &_radioData, sizeof(_radioData)), NRFLite::NO_ACK)
      {
          // Serial.println("...Success");
      }
      else
      {
          Serial.println("...Failed");
          _radioDataExample.FailedTxCount++;
      }
  }
  // SIDE-EFFECT: hasData() leaves radio in receive mode
  while(_radio.hasData())
  {
      _radio.readData(&_radioData);
      
      String msg = "==> RCVD [";
      msg += _radioData.senderId;
      msg += "=>";
      msg += SHARED_RADIO_ID;
      msg += "]: ";
      msg += _radioData.animationId;
      msg += " -- ";
      msg += _radioData.encoderPosition;
      msg += " (";
      msg += _radioDataExample.FailedTxCount;
      msg += " Failed TX)";

      Serial.println(msg);

      if(_radioData.encoderPosition < 100){
        weBlinkin = false;
        fill_solid(leds, NUMPIXELS, CRGB::Red);
      } else {
        weBlinkin = true;
        fill_solid(leds, NUMPIXELS, CRGB::White);
      };
  }
}