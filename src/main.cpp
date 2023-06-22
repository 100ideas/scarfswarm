/*
https://github.com/dparson55/NRFLite/blob/4e425d742ca8879d654a270c7c02c13440476e7a/examples/Basic_RX_ESP32/Basic_RX_ESP32.ino

Demonstrates simple RX operation with an ESP32.
Any of the Basic_TX examples can be used as a transmitter.

ESP's require the use of '__attribute__((packed))' on the RadioPacket data structure
to ensure the bytes within the structure are aligned properly in memory.

The ESP32 SPI library supports configurable SPI pins and NRFLite's mechanism to support this is shown.

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

#include "SPI.h"
#include "NRFLite.h"
#include "config.h"
#include "Knob.h"

struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
    uint8_t SenderId;
    uint32_t OnTimeMillis;
    uint32_t FailedTxCount;
};

NRFLite _radio;
RadioPacket _radioData;

const static uint8_t RADIO_ID = random();
const static uint8_t SHARED_RADIO_ID = 0;



///////////////////////////////////////////////////////////////////////////////////////////
//
// FastLED hw SPI pull request
// https://github.com/FastLED/FastLED/pull/1047
//
// // https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
// // https://lastminuteengineers.com/esp32-pinout-reference/
// #define VSPI_MISO   19
// #define VSPI_MOSI   23  // apa102 logic input ("Master Out Slave In")
// #define VSPI_SCLK   18  // apa102 clock input
// #define VSPI_SS     5
// // #define FASTLED_ALL_PINS_HARDWARE_SPI
// // #define FASTLED_ESP32_SPI_BUS VSPI

// Use if you want to force the software SPI subsystem to be used for some reason (generally, you don't)
// #define FASTLED_FORCE_SOFTWARE_SPI
// Use if you want to force non-accelerated pin access (hint: you really don't, it breaks lots of things)
// #define FASTLED_FORCE_SOFTWARE_SPI
// #define FASTLED_FORCE_SOFTWARE_PINS

// TODO figure out why PIO GCC is unhappy with Fx declarations out of lexical order...
// ... possibly related to platformio.ini lib_ldf_mode = deep 

#include <FastLED.h>

// #define DATA_PIN 18
// #define CLOCK_PIN 5
#define DATA_PIN 13
#define CLOCK_PIN 14
#define NUM_LEDS 60
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// Knob * knob{};
Knob knob{};

// This function sets up the ledsand tells the controller about them
void setup() {
    Serial.begin(115200);
    Serial.println("setup()...");

    // Configure SPI pins.
    SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
    
    // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
    uint8_t callSpiBegin = 0;
    
    // init radio w common ID so all are in "broadcast" mode
    if (!_radio.init(SHARED_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, callSpiBegin))
    {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here forever.
    }
    Serial.println("setup()... _radio.init() complete");

    _radioData.SenderId = RADIO_ID;
    _radioData.FailedTxCount = 0;

	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);

    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);  // BGR ordering is typical
    Serial.println("setup()... FastLED.addLeds() complete\n");

    knob.setup();
    Serial.println("setup()... knob.setup() complete\n");
}

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012()
{
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}


void loop()
{
    _radioData.OnTimeMillis = millis();

    // enter SEND mode AT MOST every ~1 sec or so 
    // (millis() will always not always be called w/ sub-ms frequency)
    if (_radioData.OnTimeMillis % 1000 == 0)
    {
        String msg = "<== SENT [";
        msg += RADIO_ID;
        msg += "=>";
        msg += SHARED_RADIO_ID;
        msg += "]: ";
        msg += _radioData.OnTimeMillis;
        msg += " ms (";
        msg += _radioData.FailedTxCount;
        msg += " Failed TX)";
        Serial.print(msg);
        
        // liteswarm#master/Radio.h#72
        // _radio.send(SHARED_RADIO_ID, &_outboundRadioPacket, sizeof(_outboundRadioPacket), NRFLite::NO_ACK);
        if (_radio.send(SHARED_RADIO_ID, &_radioData, sizeof(_radioData)), NRFLite::NO_ACK)
        {
            Serial.println("...Success");
        }
        else
        {
            Serial.println("...Failed");
            _radioData.FailedTxCount++;
        }
    }

    // SIDE-EFFECT: hasData() leaves radio in receive mode
    while (_radio.hasData())
    {
        _radio.readData(&_radioData);
        
        String msg = "==> RCVD [  ";
        msg += _radioData.SenderId;
        msg += "=>";
        msg += SHARED_RADIO_ID;
        msg += "]: ";
        msg += _radioData.OnTimeMillis;
        msg += " ms (";
        msg += _radioData.FailedTxCount;
        msg += " Failed TX)";

        Serial.println(msg);
    }


  // Add entropy to random number generator; we use a lot of it.
  // random16_add_entropy( random());

  if( millis() < 5000 )
  {
      Fire2012(); // run simulation frame
  } else {
    FastLED.clear( true );
  }
  
  
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}