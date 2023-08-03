#include <Arduino.h>

#include <ESP32Encoder.h>
ESP32Encoder encoder;
// timer and flag for example, not needed for encoders
unsigned long encoderlastToggled;
bool encoderPaused = false;


// #define ARDUINO_RUNNING_CORE 1

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

void vMainTaskAnimCylon( void *pvParameters );
void vMainTaskAnalogRead( void *pvParameters );
TaskHandle_t analog_read_task_handle; // You can (don't have to) use this to be able to manipulate a task from somewhere else.
#define ANALOG_INPUT_PIN 39 // Vbatt (A3 on feather)



void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // https://github.com/FastLED/FastLED/blob/master/src/FastLED.h#L246
  FastLED.addLeds<APA102, LED_spiMosi, LED_spiClk, BGR>(leds, NUM_LEDS);  // BGR ordering is typical
  FastLED.setBrightness(84);
  Serial.println("main.setup(): FastLED.addLeds() complete\n");


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


void loop() {
  if(analog_read_task_handle != NULL){ // Make sure that the task actually exists
    delay(10000);
    vTaskDelete(analog_read_task_handle); // Delete task
    analog_read_task_handle = NULL; // prevent calling vTaskDelete on non-existing task
    Serial.println("vTaskDelete(analog_read_task_handle); // Delete task");
  }

  // Loop and read the count
	Serial.println("Encoder count = " + String((int32_t)encoder.getCount()) );
	delay(100);
}





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