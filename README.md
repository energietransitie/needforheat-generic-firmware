# Generic firmware for ESP-based Twomes measurement devices
This repository contains the generic firmware, with features common to various Twomes measurement devices, based on an ESP32 or ESP8266 SoC.

## Table of contents
* [General info](#general-info)
* [Deploying](#deploying)
* [Developing](#developing) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
Different Twomes measurement devices may have various features in common, including device preperation, provisioning of home Wi-Fi network credentials via Bluetooth Low Energy (BLE) or via a temporary software access point (SoftAP), device-backend activation, network time synchronisation, persistent buffering and secure uploading of measurement data. This software repository provides a shared libary for many of these common features. With this library, we also intend to make it easier to port software between Twomes devices based on an ESP32 or ESP8266 SoC. This facilitaties development of firmware for Twomes measurement devices.

## Deploying

### Prerequisites
*	a device based on an ESP32 SoC, such as the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3) or an ESP8266 SoC, such as the [Wemos LOLIN D1 mini](https://www.wemos.cc/en/latest/d1/d1_mini.html);
*	a USB to micro-USB cable;
*	a PC with a USB port;
*	[Python version 3.8 or above](https://docs.python.org/3/using/windows.html) installed, make sure to add the path to the Python executable to your PATH variable so you can use Python commands from the command prompt;
*	[Esptool](https://github.com/espressif/esptool) installed, the Espressif SoC serial bootloader utility;
*	[Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO plugin](https://platformio.org/install/ide?install=vscode) for [Visual Studio Code](https://code.visualstudio.com/download) installed.

### Device Preparation step 1/a: Uploading Firmware to ESP32
*	Connect the device with a USB cable to the PC.
*	Download the [binary release for your device](https://github.com/energietransitie/twomes-generic-esp-firmware/releases) and extract it to a directory of your choice.
*	Some devices, such as the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3), are based on the  CH340 USB to serial converter, which may not be recognized by Windows. You may need to install a specific usb driver on your computer before you can upload firmware. For Windows, we included `CH341SER.exe` in the release; run this executable to in stall the device driver.
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
Should you encounter issues you may try to replace `esptool.py` in the above commands with `python -m esptool`.

### Device Preparation step 1/b: Uploading Firmware to ESP8266 devices
TO BE DOCUMENTED

### Device Preparation step 2: Establishing a unique Proof-of-Possession (pop)
* First, you should open a serial monitor (using Arduino IDE or PlatformIO) with baud rate 115200 to monitor the serial port connected to the Twomes measurement devcie. 
* Then, if your device is powered up (and running), briefly press the reset button. On the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3), this button is labeled 'RST' and can be found if you look 90 degrees clockwise from the micro-USB connector.
* On the serial monitor window, you should see reset information, including the unique Proof-of-Possession id (indicated by `PoP`) that was just established:
	`Twomes Heartbeat Test Application ESP32: The PoP is: 810667973`

### Device Preparation step 3: Creating the device in the Twomes backend using device type and pop  
The pop you just established should be created in the database of the server backend that you're using. If you are using the Twomes test server API, you can do this via a [POST on the /device endpoint](https://api.tst.energietransitiewindesheim.nl/docs#/default/device_create_device_post), using the pop you just established and the device's DeviceType.name as a parameter. If you are using the Twomes test server API, you shuold use a DeviceType.name from the [list of pre-registered device type names in the twomes test server](https://github.com/energietransitie/twomes-backoffice-api/blob/master/src/data/sensors.csv). If you don't have an admin bearer session token, refer to [this section on the Twomes API](https://github.com/energietransitie/twomes-backoffice-api#deployment) how to obtain one.

### Device Preparation step 4: Generating a QR-code
The pop and the [DeviceType.DisplayName](https://github.com/energietransitie/twomes-backoffice-api/blob/master/src/data/sensors.csv) of the device should be encoded in a QR-code that is printed and stuck to the back of the Twomes measurement devcie. In general, we follow [Espressif's QR-code format](https://github.com/espressif/esp-idf-provisioning-android#qr-code-scan). With a few additional conventions: we always use security and currently, support for SoftAp is not yet fully implemented nor fully documented. 

The QR-code payload is a JSON string representing a dictionary with key value pairs listed in the table below. An example payload: `{"ver":"v1","name":"testapparaatje","pop":"810667973","transport":"ble"}`

Payload information : 

| Key       	| Detail                             	| Example                                  	| Required                                                            	|
|-----------	|------------------------------------	|-----------------------------------------	|---------------------------------------------------------------------	|
| ver       	| Version of the QR code.            	| `v1`				               	| Yes                                                                 	|
| name      	| DeviceType.DisplayName of the device 	| `testapparaatje`                             	| Yes                                                                 	|
| pop       	| Proof-of-Possession id               	| `810667973`				   	| Yes								 	|
| transport 	| Wi-Fi provisioning transport type 	| It can be `softap` or `ble`	               	| Yes                                                                 	|
| security  	| Security for device communication 	| It can be `0` or `1`		              	| Optional; considered `1` (secure) if not available in QR-code payload	|
| password  	| Password of SoftAP device.         	| Password to connect with SoftAP device. 	| Optional                                                            	|

To generate a QR-code, you can use any QR-code generator. When generating QR-codes for production use, you MUST use an offline QR-code gerator, such as [this chrome extension offline QR-code generator](https://chrome.google.com/webstore/detail/offline-qr-code-generator/fehmldbcmhbdkofkiaedfejkalnidchm), which also works in the Microsoft Edge browser. A Proof-of-Possession code might constitute personal information since it is used in a process that might link personally identifiable information of subjects to measurement data. Simply encode the example payload you find below. Note: the payload is NOT a URL, so it should NOT start with `http://` nor with `https://`; the QR-code just includes a list of JSON key-value pairs). 

As payload entry you can copy the string below, but you should at least replace the values `testapparaatje` and `810667973` by the values valid for your specific device.
```shell
{"ver":"v1","name":"testapparaatje","pop":"810667973","transport":"ble"}  
```

### Erasing Wi-Fi provisioning data via a long (>10 s) button press 
To change the name and/or associated password of the Wi-Fi network that a device is connected to, users can hold down a specific button for more than 10 seconds. 

On a bare [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3) development board, this button is labeled  'BOOT'. You can find it just above the micro-USB port. On measurement devices that will be deployed in the field, this button is typically covered by a shield complying with the Wemos D1 Mini form factor and an enclosure; their designs should include a recessed button behind a small hole in the enclosure of the measurement device, which makes it hard to press this button accidentally, but easy to press deliberately with tools comonly available at home, such as a a pen or pencil.

When you release the button after holding it down for more than 10 seconds, the Wi-Fi provisioning data is deleted from persistent (non-volatile) memory, the device reboots and starts the Wi-Fi provisioning process again. You can also observe this behaviour via a serial monitor. Before provisioning again, make sure to empty the building_id column of your device in the online database by putting "[NULL]" as a value without the quotes and delete the activated_on value for your device. Otherwise you cannot use Warmtewachter to provision again. Make sure to press save in the bottom bar for cloudbeaver.

Note that this procedure:
* will NOT erase the Proof-of-Possession identifier;
* will NOT erase the bearer authorisation token needed to upload measurement data to the server;
* will not cause a call on the /device/activate endpoit on the server. 

Note also that the Wi-Fi provisioning data is stored in persistent (non-volatile) memory and will NOT be erased when you upload new firmware. 

### Erasing the Proof-of-Possession (pop) identifier (erases Wi-Fi provisioning data as well)
The Proof-of-Posession (pop) identifier is stored in persistent (non-volatile) memory. To erase the pop, use the commands below. Note that these commands erase the entire persistent (non-volatile) memory and hence, this command also erases the Wi-Fi provisioning data and bearer authorisation token needed to upload measurement data to the server. Doing this does not reset the PoP and bearer/authentication token. . .
*	Open a command prompt and enter:
	```shell
	esptool.py erase_flash
	```
*	If the port is not detected automatically, enter (while replacing `?`  with the digit found earlier):
	```shell
	esptool.py erase_flash --port "COM?" 
	```
After this command you can and should perform the entire device privisioning lifecycle (device peraration, device-app activation and device-backend activation anew).

Again, should you encounter issues you may try to replace `esptool.py` in the above commands with `python -m esptool`.

## Developing 

### Prerequisites

Prerequisites for deploying, plus:
*	[Visual Studio Code](https://code.visualstudio.com/download) installed
*	[PlatformIO for Visual Studio Code](https://platformio.org/install/ide?install=vscode) installed
*	This GitHub repository cloned

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
  11. To provision the device with Wi-Fi connectivity, use an app that supports Espressif Unified Provisioning. A list can be found below. 

### Provisioning

Use the [Twomes WarmteWachter app](https://github.com/energietransitie/twomes-app-warmtewachter) to test the full Twomes provisioning flow.

Alternatively, you may use test apps that only support the network part of provisioning using Espressif Unified Provisioning; these can be found at:
  
* [Android Unified Provisioning app for BLE](https://play.google.com/store/apps/details?id=com.espressif.provble&hl=en&gl=US)
* [Android Unified Provisioning app for SoftAP](https://play.google.com/store/apps/details?id=com.espressif.provsoftap&hl=en&gl=US)
* [Apple Unified Provisioning app for BLE](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)
* [Apple Unified Provisioning app for SoftAP](https://apps.apple.com/us/app/esp-softap-provisioning/id1474040630)

In the apps, click `I don't have a QR code` at the bottom of the screen after you started the provisioning flow.
  
 ### Other Things To Keep In Mind
 * Check the platformio.ini file in the cloned folder, look at the board_upload.flash_size, board_upload.maximum_size and board_build.partitions to check if they are right for your hardware.

## Features
Currently ready:

* Unified Provisioning over Bluetooth Low Energy (BLE; ESP32 only) and SoftAP
* Time synchronisation using NTP
* Secure transport over TLS/SSL (ESP32)
* Heartbeats: regularly uploading timestamped measurment data with property `heartbeat`
* Long button press to erase only Wi-Fi provisioning data 
* Example code

To-do:

* Persistent buffering of measurement data using NVS
* Using [ISRG Root X1 certificate](https://crt.sh/?id=3334561879) to check energietransitiewindesheim.nl certificate to ensure working beyond 29-Sep-2021 
* Secure transport over TLS/SSL (ESP8266)

## Status
Project is: in-progress

## License
This software is available under the [Apache 2.0 license](./LICENSE.md), Copyright 2021 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits

We use and gratefully aknowlegde the efforts of the makers of the following source code and libraries:

* [ESP-IDF](https://github.com/espressif/esp-idf), Copyright (C) 2015-2019 Espressif Systems, licensed under [Apache 2.0 license](https://github.com/espressif/esp-idf/blob/73db142403c6e5b763a0e1c07312200e9b622673/LICENSE)
