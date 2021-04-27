# Generic firmware for Twomes measurement devices based on ESP SoCs
> This repository contains the generic software for the ESP32 and ESP8266 based on ESP-IDF for the Twomes project.

## Table of contents
* [General info](#general-info)
* [Using binary releases](#using-binary-releases)
* [Developing with the source code ](#developing-with-the-source-code) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
With this software we intend to make it easier to switch software implementations between ESP32 and ESP8266 devices, making them more portable, as well as having some features already implemented. This helps in kickstarting any project or module when it comes to development with the ESP32 and ESP8266.

## Using binary releases

### Prerequisites
*	Install Python and make sure to add it to your Path variable so you can use it from the command prompt: https://docs.python.org/3/using/windows.html (I use version 3.9 but 3.8 also works, other versions might work as well.)  
*	Install EspTool: https://github.com/espressif/esptool (use the README on how to install)  
*	Install the usb driver for the ESP32 for windows, found in the WP2 Twomes Folder -> 2020-2021 S1 Werkstudent – Twomes Firmware -> BinariesAndDriver Folder, the .exe name is CH341SER.exe
 
### Uploading Firmware To ESP 32
*	Connect device with a USB cable to the PC.  
*	Download the release that you need from: https://github.com/energietransitie/twomes-generic-esp-firmware/releases and extract it to a directory of your choice
*	Open a comand prompt in that directory and change the directory to the BinariesAndDriver folder and enter:
	```shell
	esptool.py --chip esp32  --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin  
	```
*	This should auto detect the USB port that the device is connected to.
*	If not, then open the Device Manager in Windows (e.g.hold Windows Key, type X, then select Device Manager), go to View and click Show Hidden Devices. Then unfold Ports (COM & LPT). You should find the device in there, named `USB-Serial CH340 *(COM?)` with ? being a single digit.  
*	If COM port is not auto-detected then enter (while replacing `?` with the digit found in the previous step): 
	```shell
	esptool.py --chip esp32  --port "COM?" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin  
	```

### Resetting Provisioning
THe Wi-Fi network chosen and its credentials that are selected during the provisioning process are stored persistently and will NOT be erased by uploading new firmware. To select another WiFi or test the provisioniong proces again, you need to erase the memory where these credentials are stored.
*	Open a command prompt and enter:
	```shell
	esptool.py erase_flash
	```
*	If the port is not detected automatically, enter (while replacing `?  with the digit found earlier):
	```shell
	esptool.py erase_flash --port "COM?" 
	```
After these 

## Developing with the source code 
### Prerequisites
Install Visual Studio Code: https://code.visualstudio.com/download
Install PlatformIO for Visual Studio Code: https://platformio.org/install/ide?install=vscode
Clone This GitHub Repo:
  * git@github.com:energietransitie/twomes-generic-esp-firmware.git for Git with SSH
  * https://github.com/energietransitie/twomes-generic-esp-firmware.git Git without SSH
  * You can use GitHub Desktop Application for Windows: https://desktop.github.com/
  * Or you can download everything as a .zip file: https://github.com/energietransitie/twomes-generic-esp-firmware/archive/refs/heads/master.zip and extract it sowewhere.
### Usage  
Open the project in PlatformIO:
  1. In the top-left corner select File -> Open Folder
  2. Select the folder where you cloned or extracted the repository
  3. In the src folder you will find main.c this is an example application
  4. In the lib folder you will find the library that this repository contains
  5. In the CMakeLists.txt in the folder where you cloned the repo(not the one in src/) you will see a variable: CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP if needed change to CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
  this decides what provisioning method is used.
  6. Edit or replace the src/main.c code with your code(or don't and run for testing)
  7. Click the PlatformIO Icon(The little alien in the left of the screen)
  8. Then unfold esp32dev
  9. Click upload and monitor. NOTE: The first time might take a while because PlatformIO needs to install and build the ESP-IDF before flashing.
  10. When it is done flashing press CTRL+T and then B, then type 115200 so that it sets the right BAUD rate and you see text not gibberish.
  11. If needed run the Espressif BLE Provisioning App or SoftAP provisioning App found in the play store for provisioning of WiFi.
  Apps can be found for Android at:
  * SoftAP: https://play.google.com/store/apps/details?id=com.espressif.provsoftap&hl=en&gl=US
  * BLE: https://play.google.com/store/apps/details?id=com.espressif.provble&hl=en&gl=US
  And for Apple:
  * BLE: https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141
  * SoftAP: https://apps.apple.com/us/app/esp-softap-provisioning/id1474040630  
  In the apps click I don't have a QR code at the bottom of the screen when you've started provisioning.
  
 ### Other Things To Keep In Mind
 * Check the platformio.ini file in the cloned folder, look at the board_upload.flash_size, board_upload.maximum_size and board_build.partitions to check if they are right for your hardware.

## Features
List of features ready and TODOs for future development. Ready:

* Time synchronisation using NTP
* Unified Provirioning over Bluetooth (ESP32 only) and SoftAP
* Secure transport over TLS/SSL (ESP32)
* HTTP
* Example code

To-do:

* Secure transpoort over TLS/SSL (ESP8266)
* Persistent burrering using NVS

## Status
Project is: in-progress

## License
This software is available under the [Apache 2.0 license](./LICENSE.md), Copyright 2021 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits

We use and gratefully aknowlegde the efforts of the makers of the following source code and libraries:

* [ESP-IDF](https://github.com/espressif/esp-idf), by Copyright (C) 2015-2019 Espressif Systems, licensed under [Apache 2.0 license](https://github.com/espressif/esp-idf/blob/73db142403c6e5b763a0e1c07312200e9b622673/LICENSE)

## Contact
<not yet determined; which contact info to include here?>
