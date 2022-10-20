# Building firmware

To build firmware for a Twomes measurement device, first open the root folder of the firmware directory in Visual Studio Code.

## Building firmware
1. Click the PlatformIO Icon (the little alien in the left of the screen).
2. Unfold `M5STACK_COREINK` when building for [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk), for other platforms unfold `ESP32DEV`.
> Do not use the 'ESP32DEV' environment for [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) board! It does work, but some features specific to the [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) will not be included in the firmware, which cause problems during run-time.
3. Click `Upload and Monitor`. 
> The first time might take a while because PlatformIO needs to install and build the ESP-IDF before it can upload the firmwware.
4. When it is done uploading, press `CTRL+T` and then `B`, then type `115200` so that it sets the right baud rate and you see text not gibberish.
5. To provision the device, use [the Twomes app or another app that supports Espressif Unified Provisioning](../provisioning/testing.md).


