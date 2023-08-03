# 2023-08-03 FastLED notes


investigate I2S fastled on esp32 #TODO 

https://github.com/FastLED/FastLED/issues/1517#issuecomment-1618903155
> ### alba-ado commented on Jul 3
> 
> Hello, First of all, thank you for creating this awesome library.
> 
> I am using this library inside the ESP-IDF with the ESP32-S3 board. While testing out some code, I noticed that the library does not work properly when pinned to Core 1 with the xTaskCreate. I am driving merely 16 LEDs and it works fine on Core 0, but when I pin the task on Core 1, the lights either don't work at all or only a few LEDs get updated. I use WS2812B chips with the library. For some reason, when I set the whole frame to a fixed color it doesn't work at all. But when I set the frame with a code similar to Pride2015, just a few LEDs get updated but they work correctly. However, the rest of the LEDs don't work. I thought this was a priority issue of the tasks or because of the buffer size of the task, but changing neither made the library work on Core 1. However, when I switch the Core 0 everything works just fine. Since I have WiFi and ESP-Mesh code running on the Core 0, I don't want to bother with updating the LEDs which is a very low priority for me. This issue might be related to the one mentioned in #1438 but I'm not completely sure. That's why I opened a new issue.
> 
> For me, when I create it like this it works:
> 
> ```
> xTaskCreatePinnedToCore( DeviceInterface::frameTask, "Frame-Task", 2000, NULL, 2, NULL, PRO_CPU_NUM );
> ```
> 
> And when I create it like this, it doesn't work:
> 
> ```
> xTaskCreatePinnedToCore( DeviceInterface::frameTask, "Frame-Task", 2000, NULL, 2, NULL, APP_CPU_NUM );
> ```
> 
> I can't pinpoint the exact source of the issue since the code doesn't throw any errors, but I think the show function does not update the LEDs properly for some reason as mentioned in the other issue. The rest of the code seems to be working fine.
> 
> My task looks like this:
> 
> ```
> void DeviceInterface::frameTask( void * args ) {
>     InterfaceState lastState = INTERFACE_STATE_UNKNOWN;
> 
>     while( true ) {
>         
>         if( currentState == INTERFACE_IDLE ) {
>             LightFrame::frameRainbow();
>         }
>         
>         delay(INTERFACE_LIGHT_FRAME_UPDATE_INTERVAL);
>     }
> }
> ```
> 
> But even if I create a task only for updating the LEDs, the same issue occurs.
> 
> Waiting for your help, best regards.
>
>
> ---
>
> ### toybuilder commented on Jul 3 
>
> There's other tasks running and competing for CPU cycles. Try using I2S mode by prepending the #define before the #include. This offloads the output work to the I2S peripheral so that it does not need the CPU cycles.
> 
> ```
> #define FASTLED_ESP32_I2S 1
> #include <FastLED.h>
> ```
> 
> In my own applications, I've found that FastLED does not work well without using I2S mode. Downside is that if you need I2S hardware for other aspects of your system, you cannot use it for FastLED.
> 
> Oh- it looks like I2S support is broken on the S2 and possibly the S3? My experience is with the ESP32, not the -S2 or -S3.

