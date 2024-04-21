# Device preparation

This section describes how a device running firmware based on the needforheat-generic-firmware library should be prepared to work together with a [NeedForHeat server](https://github.com/energietransitie/needforheat-server-configuration) using the [NeedForHeat API](https://github.com/energietransitie/needforheat-server-api).

Currently, each device instance must be registered on a [NeedForHeat server](https://github.com/energietransitie/needforheat-server-configuration) via the [NeedForHeat API](https://github.com/energietransitie/needforheat-server-api) before the server accepts data send data to it from that device.

## Erase all persistenly stored data

Unless you are 100% sure that it is safe to only upload firmware and keep other persistent memory intact, you should always first completely erase the persistent (non-volatile) memory of the device. The procedure below not only erases the firmware, but also any device activation_token, Wi-Fi provisioning data and device session_token that may reside in the persistent memory of the device and which is needed as bearer token that identifies, authenticates and authorizes the device when uploading measurement data to the server:

*	Open a command prompt and enter:
	```shell
	py -m esptool erase_flash
	```
*	If the port is not detected automatically, enter (while replacing `?`  with the digit found earlier):
	```shell
	py -m esptool erase_flash --port "COM?" 
	```

> Should you encounter issues you may try to replace `py -m esptool` in the above commands with `python -m esptool` or `esptool.py`

After this command you should perform the full NeedForHeat device preparation flow below:

1. Upload firmware to the ESP32 device
2. Find the device name and activation_token
3. Register the device on the [NeedForHeat server](https://github.com/energietransitie/needforheat-server-configuration) using the [NeedForHeat API](https://github.com/energietransitie/needforheat-server-api)
4. Generating a QR-code and print the QR_code<br>
> For measurement devices that use the [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk), this last step can be skipped, since the firmware automatically displays the QR-code on the e-ink screen of the device.
 

## Step 1: Upload firmware to the ESP32 device

1. Download the [binary release for your device](https://github.com/energietransitie/needforheat-generic-firmware/releases) and extract it to a directory of your choice.
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

## Step 2: Generating a QR-code
> For measurement devices that use the [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk), this step can be skipped, since the firmware automatically displays the QR-code on the e-ink screen of the device.<

The `device.name` and `device.activation_token` of the device should be encoded in a QR-code that visible to the subject that receives the NeedForHeat measurement device. When printed on a sticker, we recomend attaching it the back of the measurement device.

In general, we follow [Espressif's QR-code format](https://github.com/espressif/esp-idf-provisioning-android#qr-code-scan). With a few additional conventions: we always use security and currently, support for SoftAP is not yet fully implemented nor fully documented. Watch this space for changes in the way the `name` key of the QR-code payload is used.

The QR-code payload is a JSON string representing a dictionary with key value pairs listed in the table below.

Payload information : 

| Key       	| Value used                           	| Example                                  	| Required                                                            	|
|-----------	|------------------------------------	|-----------------------------------------	|---------------------------------------------------------------------	|
| ver       	| Version of the QR code.            	| `v1`				               	| Yes                                                                 	|
| name      	| device.name	 		 	| `NFH-0D45DF`                             	| Yes                                                                 	|
| pop       	| device.activation_token              	| `810667973`				   	| Yes								 	|
| transport 	| Wi-Fi provisioning transport type 	| Either `softap` or `ble`	               	| Yes                                                                 	|
| security  	| Security during Wi-Fi provisioning  	| Either `0` or `1`		              	| Optional; considered `1` (secure) if not available in QR-code payload	|
| password  	| device.activation_token         	| Password to connect with SoftAP device. 	| Optional                                                            	|

To generate a QR-code, you can use any QR-code generator. When generating QR-codes for production use, you SHOULD use an offline QR-code gerator, such as [this chrome extension offline QR-code generator](https://chrome.google.com/webstore/detail/offline-qr-code-generator/fehmldbcmhbdkofkiaedfejkalnidchm), which also works in the Microsoft Edge browser. A device activation_token might constitute personal information since it is used in a process that might link personally identifiable information of subjects to measurement 
> The Espressif QR-code payload is NOT a URL, so it should NOT start with `http://` nor with `https://`; the QR-code just includes a list of JSON key-value pairs).

## Prepare an existing device

If you want to repurpose and existing device (e.g. use it in another home), you should:

* erase all persistently stored data on the device;
* after the value in the `name` column of the existing device entry in the database, append something like `-OLD` and click on `SAVE` (for the NeedForHeat test database, you can do this via [CloudBeaver](https://db.energietransitiewindesheim.nl/#/));
*  proceed with regular device preparation.
