# Generic firmware for Twomes measurement devices based on ESP SoCs
> This repository contains the generic firmware, with features common to various Twomes measurement devices, based on the ESP32 or ESP8266 SoC.

## Table of contents
* [General info](#general-info)
* [Deploying](#deploying)
* [Developing](#developing) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
Different Twomes measurement devices may have various features in common, including provisioning of home Wi-Fi network credentials via a temporary soft access point (SoftAP) or Bleutooth Low Energy (BLE), device activation, time synchronisation, persistent buffering and secure uploading of measurement data. This software repository provides a shared libary for many of these common features. With this library, we also intend to make it easier to port software between Twomes devices based on an ESP32 or ESP8266 SoC. This facilitaties development of firmware for Twomes measurement devices.

## Deploying

### Prerequisites
*	Install [Python version 3.8 or above](https://docs.python.org/3/using/windows.html) and  make sure to add the path to the Python executable to your PATH variable so you can use Python commands from the command prompt.
*	Install [Esptool](https://github.com/espressif/esptool), the Espressif SoC serial bootloader utility.

### Uploading Firmware to ESP32
*	Connect the device with a USB cable to the PC.
*	Download the [binary release for your device](https://github.com/energietransitie/twomes-generic-esp-firmware/releases) and extract it to a directory of your choice.
*	For some devices, such as the TTGO LilyGo H201 ESP32 device, you may need to install a specific usb driver on your computer before you can upload firmware. For Windows, we included CH341SER.exe in the release; run this executable to in stall the device driver.
*	Open a comand prompt in that directory, change the directory to the BinariesAndDriver subfolder and enter:
	```shell
	esptool.py --chip esp32  --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin  
	```
*	This should automatically detect the USB port that the device is connected to.
*	If not, then open the Device Manager in Windows (e.g., hold Windows Key, type X, then select Device Manager), go to View and click Show Hidden Devices. Then unfold `Ports (COM & LPT)`. You should find the device there, named `USB-Serial CH340 *(COM?)` with `?` being a single digit.  
*	If the COM port is not automatically detected, then enter (while replacing `?` with the digit found in the previous step): 
	```shell
	esptool.py --chip esp32  --port "COM?" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin  
	```
### Uploading Firmware to ESP8266 devices
TO BE DOCUMENTED

### Resetting Provisioning
You can only provision a Twomes measurement device once. The Wi-Fi network chosen and its credentials that are selected during the provisioning process are stored persistently and will NOT be erased by uploading new firmware. To select another Wi-Fi network, change the network credentials and/or test the provisioniong proces again, you need to erase the memory where these credentials are stored.
*	Open a command prompt and enter:
	```shell
	esptool.py erase_flash
	```
*	If the port is not detected automatically, enter (while replacing `?  with the digit found earlier):
	```shell
	esptool.py erase_flash --port "COM?" 
	```
After this command you can perform device provisioning anew.

## Developing 

### Prerequisites

*	Install [Visual Studio Code](https://code.visualstudio.com/download)
*	Install [PlatformIO for Visual Studio Code](https://platformio.org/install/ide?install=vscode)
*	Clone this GitHub reposotory.

### Usage  

Open the project in PlatformIO:
  1. In the top-left corner, select File -> Open Folder.
  2. Select the folder where you cloned or extracted the repository.
  3. In the `src` folder you will find `main.c`; this is an example application.
  4. In the `lib` folder you will find the library that this repository contains.
  5. To change between provisioning via BLE or SoftAp, in the `CMakeLists.txt`file in the folder where you cloned the repo (not the one in src/), change the variable `CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP` to `CONFIG_EXAMPLE_PROV_TRANSPORT_BLE`or vice versa.
  6. Change the code in `src/main.c` as you see fit.
  7. Click the PlatformIO Icon (the little alien in the left of the screen).
  8. Unfold `esp32dev`.
  9. Click `upload and monitor`. 
NOTE: The first time might take a while because PlatformIO needs to install and build the ESP-IDF before flashing.
  10. When it is done flashing, press `CTRL+T` and then `B`, then type `115200` so that it sets the right baud rate and you see text not gibberish.
  11. If needed run the Espressif BLE Provisioning App or SoftAP provisioning App found in the play store to test Wi-Fi provisioning.
  Espressif Unified Provisioning Apps can be found at:
	* [Android Unified Provisioning app for BLE](https://play.google.com/store/apps/details?id=com.espressif.provble&hl=en&gl=US)
	* [Android Unified Provisioning app for SoftAP](https://play.google.com/store/apps/details?id=com.espressif.provsoftap&hl=en&gl=US)
	* [Apple Unified Provisioning app for BLE](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)
  	* [Apple Unified Provisioning app for SoftAP](https://apps.apple.com/us/app/esp-softap-provisioning/id1474040630)
  In the apps, click `I don't have a QR code` at the bottom of the screen after you started the provisioning flow.
  Alternatively, you may also use the [Twomes WarmteWachter app](https://github.com/energietransitie/twomes-app-warmtewachter) to test the full Twomes provisioning flow.
  
 ### Other Things To Keep In Mind
 * Check the platformio.ini file in the cloned folder, look at the board_upload.flash_size, board_upload.maximum_size and board_build.partitions to check if they are right for your hardware.

## Features
Currently ready:

* Time synchronisation using NTP
* Unified Provisioning over Bluetooth Low Energy (BLE; ESP32 only) and SoftAP
* Secure transport over TLS/SSL (ESP32)
* HTTP
* Example code

To-do:

* Secure transport over TLS/SSL (ESP8266)
* Persistent buffering using NVS

## Status
Project is: in-progress

## License
This software is available under the [Apache 2.0 license](./LICENSE.md), Copyright 2021 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits

We use and gratefully aknowlegde the efforts of the makers of the following source code and libraries:

* [ESP-IDF](https://github.com/espressif/esp-idf), Copyright (C) 2015-2019 Espressif Systems, licensed under [Apache 2.0 license](https://github.com/espressif/esp-idf/blob/73db142403c6e5b763a0e1c07312200e9b622673/LICENSE)
