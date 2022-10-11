# Device preparation

## Prepare a device for use with a Twomes server

This section describes how a device that uses the Twomes generic firmware library can be prepared to work together with a Twomes server using the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api).

Any device needs to be created on a Twomes server before it can be used to send data to it.

### Prerequisites

* a device based on an ESP32 SoC, such as the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3)
* a USB to micro-USB cable
* a PC with a USB port
* [Python v3.8 or above](https://www.python.org/downloads/) installed, and make sure to select `Add Python <version number> to PATH` so you can use the Python commands we document below from a command prompt
* [Esptool](https://github.com/espressif/esptool) installed, the Espressif SoC serial bootloader utility
* [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/), a serial monitor utility (if your you are also developing, you may use the serial monitor utility in your IDE, instead)
* Some devices, such as the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3), are based on the  CH340 USB to serial converter, which may not be recognized by your OS. You may need to install a specific driver:
	* [Windows driver](http://www.wch.cn/download/CH341SER_EXE.html)
	* [Mac OSX driver](http://www.wch.cn/download/CH341SER_MAC_ZIP.html) (see also [this additional info](https://kig.re/2014/12/31/how-to-use-arduino-nano-mini-pro-with-CH340G-on-mac-osx-yosemite.html))
	* [Linux driver](http://www.wch.cn/download/CH341SER_LINUX_ZIP.html)
	* [Android driver](http://www.wch.cn/download/CH341SER_ANDROID_ZIP.html)

### Erase all persistenly stored data

Unless you are 100% sure that it is safe to only upload firmware and keep other persistent memory intact, you should always first completely erase the persistent (non-volatile) memory of the device. The procedure below not only erases the firmware, but also any device activation_token, Wi-Fi provisioning data and device session_token  that may reside in the persistent memory of the device and which is needed as bearer token that identifies, authenticates and authorizes the device when uploading measurement data to the server.
*	Open a command prompt and enter:
	```shell
	py -m esptool erase_flash
	```
*	If the port is not detected automatically, enter (while replacing `?`  with the digit found earlier):
	```shell
	py -m esptool erase_flash --port "COM?" 
	```
Should you encounter issues you may try to replace `py -m esptool` in the above commands with `python -m esptool` or `esptool.py`

After this command you should perform the full Twomes device provisioning flow below.

### Step 1: Upload firmware to the ESP32 device

1. Download the [binary release for your device](https://github.com/energietransitie/twomes-generic-esp-firmware/releases) and extract it to a directory of your choice.
2. Connect the device with a USB cable to the PC.
3. If you used the device before, you shoud first [erase all persistenly stored data](#erase-all-persistenly-stored-data)
4.	Open a comand prompt in the directory you chose to download and extract the binary releases in and enter the following:
	```shell
	py -m esptool --chip esp32 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x18000 partitions.bin 0x1d000 ota_data_initial.bin 0x20000 firmware.bin  
	```

	This should automatically detect the USB port that the device is connected to. If not, you need to find the correct port:
	1. Open the Device Manager (in Windows press the `Windows + X` key combination, then select Device Manager)
	2. Go to View and click Show Hidden Devices. 
	3. Unfold `Ports (COM & LPT)`. You should find the device there, named `USB-Serial CH340 *(COM?)` with `?` being a single digit.  
	
	If the COM port was not automatically detected, issue the following command (while replacing `?` with the digit found in the previous step): 
	```shell
	py -m esptool --chip esp32 --port "COM?" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x18000 partitions.bin 0x1d000 ota_data_initial.bin 0x20000 firmware.bin
	```

> Should you encounter issues you may try to replace `py -m esptool` in the above commands with:
>
> - `python -m esptool`, 
> - `python3 -m esptool` or
> - `esptool.py`.

### Step 2: Find a device's name and activation_token

1. Open your serial monitor utility.
	*  For [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/), use the following destination settings (be sure to `Save` them to `Load` them conveniently later):
		* Connection type: `Serial`
		* Speed: `115200`
		* Serial line: `COM?` (replace `?` with the number of the COM-port your device is connected to, e.g., `COM5`). 
2. Once your device is powered up (and running), briefly press the reset button.
3. You will be able to see all the boot logs of the device in the serial monitor. Somewhere in the logs, you will find the JSON payload needed to active the device on the Twomes server using the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api).

	```json5 title="Example JSON payload inside device logs"
	{
		"name":"TWOMES-D3AD48",
		"device_type":"DSMR-P1-gateway-TinTsTrCO2",
		"activation_token":"3375550652"
	}
	```

### Step 3: Create the device on the Twomes server using the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api)

In order to create a device on the Twomes server using the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api), you need the JSON payload which you found in [step 2](#step-2-find-a-devices-name-and-activationtoken).

Read the [Twomes API documentation](https://api.energietransitiewindesheim.nl/docs#/default/device_create_device_post) to see how you can use the Twomes API to create the device. 

You will need an admin bearer session token in order to use this endpoint on the Twomes API. one, refer to [this section on the Twomes API](https://github.com/energietransitie/twomes-backoffice-api#deploying-new-admin-accounts-to-apitstenergietransitiewindesheimnl) on how to obtain one.

## Prepare an existing device

If you want to repurpose and existing device (e.g. use it in another home), you should:

* erase all persistently stored data on the device;
* after the value in the `name` column of the existing device entry in the database, add `-OLD` and click on `SAVE` (for the Twomes test database, you can do this via [CloudBeaver](https://db.energietransitiewindesheim.nl/#/));
*  proceed with regular device preparation.
