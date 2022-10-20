# Prerequisites for deploying

## Hardware:

* a device based on an ESP32 SoC, e.g.
	* [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3) (programmable via micro-USB)
	* [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) (programmable via USB-C)
* a matching USB cable 
* a PC with a USB port

## Software:

* [Python v3.8 or above](https://www.python.org/downloads/) installed; make sure to select `Add Python <version number> to PATH` so you can use the Python commands we document below from a command prompt
* [Esptool](https://github.com/espressif/esptool) installed, the Espressif SoC serial bootloader utility
* [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/), a serial monitor utility (if your you are also developing, you may use the serial monitor utility in your IDE, instead)
* Some devices, such as the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3), are based on the  CH340 USB to serial converter, which may not be recognized by your OS. You may need to install a specific driver:
	* [Windows driver](http://www.wch.cn/download/CH341SER_EXE.html)
	* [Mac OSX driver](http://www.wch.cn/download/CH341SER_MAC_ZIP.html) (see also [this additional info](https://kig.re/2014/12/31/how-to-use-arduino-nano-mini-pro-with-CH340G-on-mac-osx-yosemite.html))
	* [Linux driver](http://www.wch.cn/download/CH341SER_LINUX_ZIP.html)
	* [Android driver](http://www.wch.cn/download/CH341SER_ANDROID_ZIP.html)

