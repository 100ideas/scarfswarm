#include "SPI.h"
#include <Arduino.h>
#include <NRFLite.h>
#include <ESP32Encoder.h>



///////////////////////////////////////////////////////////////////////////////////////////
// FastLED ESP32 Hardware SPI Driver

/* https://github.com/FastLED/FastLED/blob/master/src/platforms/esp/32/fastspi_esp32.h
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

#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS VSPI
/* https://github.com/FastLED/FastLED/blob/master/src/platforms/esp/32/fastspi_esp32.h#L58C1-L61C30
 *  static uint8_t spiClk = 18;
 *  static uint8_t spiMiso = 19;
 *  static uint8_t spiMosi = 23;
 *  static uint8_t spiCs = 5;
 */

// #TODO want to find a clean way to pull these defs from where they are already defined in fastspi_esp32.h
#define LED_spiClk 18
#define LED_spiMosi 23
#include <FastLED.h>

#define NUM_LEDS 60
#define FRAMES_PER_SECOND 60
CRGB leds[NUM_LEDS];




///////////////////////////////////////////////////////////////////////////////////////////
// Task setup
void vMainTaskAnimCylon( void *pvParameters );
void vMainTaskAnalogRead( void *pvParameters );
TaskHandle_t analog_read_task_handle; // You can (don't have to) use this to be able to manipulate a task from somewhere else.
#define ANALOG_INPUT_PIN 39 // Vbatt (A3 on feather)




///////////////////////////////////////////////////////////////////////////////////////////
// Radio

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

ESP32Encoder encoder;
// timer and flag for example, not needed for encoders
unsigned long encoderlastToggled;
bool encoderPaused = false;

struct __attribute__((packed)) RadioPacket // Note the packed attribute.
{
    uint8_t SenderId;
    uint32_t OnTimeMillis;
    uint32_t FailedTxCount;
};

NRFLite _radio;
RadioPacket _radioData;

const static uint8_t RADIO_ID = random();
const static uint8_t SHARED_RADIO_ID = 42;

// ezscb.com esp32 feather ~v1 SPI2/HSPI 
const static uint8_t PIN_RADIO_CE = 27;
const static uint8_t PIN_RADIO_CSN = 15;
const static uint8_t PIN_RADIO_MOSI = 13;
const static uint8_t PIN_RADIO_MISO = 12;
const static uint8_t PIN_RADIO_SCK = 14;




///////////////////////////////////////////////////////////////////////////////////////////
// setup
void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);


  // FastLED
  // https://github.com/FastLED/FastLED/blob/master/src/FastLED.h#L246
  FastLED.addLeds<APA102, LED_spiMosi, LED_spiClk, BGR>(leds, NUM_LEDS);  // BGR ordering is typical
  FastLED.setBrightness(84);
  Serial.println("main.setup(): FastLED.addLeds() complete\n");



  // nrf24 radio 
  // https://nrf24.github.io/RF24/index.html#autotoc_md45
  SPI.begin(PIN_RADIO_SCK, PIN_RADIO_MISO, PIN_RADIO_MOSI, PIN_RADIO_CSN);
  // Indicate to NRFLite that it should not call SPI.begin() during initialization since it has already been done.
  uint8_t callSpiBegin = 0;
  // init radio w common ID so all are in "broadcast" mode
  if (!_radio.init(SHARED_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100, callSpiBegin))
  {
      Serial.println("Cannot communicate with radio");
      // while (1); // Wait here forever.
  }
  Serial.println("main.setup(): _radio.init() complete");
  _radioData.SenderId = RADIO_ID;
  _radioData.FailedTxCount = 0;
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(1000);



  // Rotary Encoder Knob
  // https://github.com/madhephaestus/ESP32Encoder/blob/master/examples/Encoder/Encoder.ino
  ESP32Encoder::useInternalWeakPullResistors=UP;
	// use pin 19 and 18 for the first encoder
	encoder.attachHalfQuad(17, 16);
  // set starting count value after attaching 
	encoder.setCount(37);
	// clear the encoder's raw count and set the tracked count to zero
	// encoder.clearCount();
	Serial.println("Encoder Start = " + String((int32_t)encoder.getCount()));
	// set the lastToggle
	encoderlastToggled = millis();



  // Task creation
  // Set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    vMainTaskAnimCylon
    ,  "Task AnimationCylon" // A name just for humans
    // ,  2048        // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,  4096
    // ,  (void*) &leds // Task parameter which can modify the task behavior. This must be passed as pointer to void.
    ,  NULL
    ,  2  // Priority
    ,  NULL // Task handle is not used here - simply pass NULL
    ,  ARDUINO_RUNNING_CORE // core for task
    );

  // This variant of task creation can also specify on which core it will be run (only relevant for multi-core ESPs)
  xTaskCreate(
    vMainTaskAnalogRead
    ,  "Analog Read"
    ,  2048  // Stack size
    ,  NULL  // When no parameter is used, simply pass NULL
    ,  1  // Priority
    ,  &analog_read_task_handle // With task handle we will be able to manipulate with this task.
    );

  Serial.printf("Basic Multi Threading Arduino Example\n");
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}


// cheesy cylon example - delay() is bad probably
void fadeall() { 
  for(int i = 0; i < NUM_LEDS; i++) { 
    leds[i].nscale8(250); 
  }
  vTaskDelay(20 / portTICK_PERIOD_MS);
}




///////////////////////////////////////////////////////////////////////////////////////////
// main
void loop() {
  if(analog_read_task_handle != NULL){ // Make sure that the task actually exists
    delay(10000);
    vTaskDelete(analog_read_task_handle); // Delete task
    analog_read_task_handle = NULL; // prevent calling vTaskDelete on non-existing task
    Serial.println("vTaskDelete(analog_read_task_handle); // Delete task");
  }

  // #TODO refactor w/ Knob.h
  if(!(millis() % 1000)) Serial.println("Encoder count = " + String((int32_t)encoder.getCount()) );
	delay(100); // #TODO replace w x


  _radioData.OnTimeMillis = millis();
  // enter SEND mode AT MOST every ~1 sec or so 
  // (millis() will always not always be called w/ sub-ms frequency)
  // if (_radioData.OnTimeMillis % 1000 == 0)
  if (_radioData.OnTimeMillis % 100 == 0)
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
  while(_radio.hasData())
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
}




///////////////////////////////////////////////////////////////////////////////////////////
// Tasks
void vMainTaskAnimCylon(void *pvParameters){
  /* broken
  CRGB leds = *((CRGB*)pvParameters);
  */

  int count = 0;
  int uxHighWaterMark;
  static uint8_t hue = 0;

  for (;;){
    // First slide the led in one direction
    for(int i = 0; i < NUM_LEDS; i++) {
      // Set the i'th led to red 
      leds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      FastLED.show(); 
      // now that we've shown the leds, reset the i'th led to black
      leds[i] = CRGB::Black;
      fadeall();
      
      // arduino-esp32 has FreeRTOS configured to have a tick-rate of 1000Hz and portTICK_PERIOD_MS
      // refers to how many milliseconds the period between each ticks is, ie. 1ms.
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    Serial.print("x");

    // Now go in the other direction.  
    for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    	// Set the i'th led to red 
    	leds[i] = CHSV(hue++, 255, 255);
    	// Show the leds
    	FastLED.show();
    	// now that we've shown the leds, reset the i'th led to black
    	leds[i] = CRGB::Black;
    	fadeall();
      
      // Wait a little bit before we loop around and do it again
    	vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    Serial.print("x");

    // if(!(count % 20)) {
    if(!(count % 20)) {
      uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      Serial.printf("\nvMainTaskAnimCylon: %i, ", count);
      Serial.println(uxHighWaterMark);
    }
  count++;
  }

}


void vMainTaskAnalogRead(void *pvParameters){  // This is a task.
  (void) pvParameters;
  int count = 0;
  
  Serial.println("started vMainTaskAnalogRead");
  // Check if the given analog pin is usable - if not - delete this task
  if(!adcAttachPin(ANALOG_INPUT_PIN)){
    Serial.printf("TaskAnalogRead cannot work because the given pin %d cannot be used for ADC - the task will delete itself.\n", ANALOG_INPUT_PIN);
    analog_read_task_handle = NULL; // Prevent calling vTaskDelete on non-existing task
    vTaskDelete(NULL); // Delete this task
  }
  
/*
  AnalogReadSerial
  Reads an analog input on pin A3, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A3, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;){
    // read the input on analog pin:
    int sensorValue = analogRead(ANALOG_INPUT_PIN);
    // delay(100); // 100ms delay
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    count++;
    Serial.print("vMainTaskAnalogRead: ");
    Serial.print(count);
    Serial.print(" - ");
    Serial.println(sensorValue);
    // if(count >= 10) vTaskDelete(NULL);
  }
}