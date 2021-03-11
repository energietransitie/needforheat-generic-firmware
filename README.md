## Firmware
### Prerequisites
*	Install Python and make sure to add it to your Path variable so you can use it from the command prompt: https://docs.python.org/3/using/windows.html (I use version 3.9 but 3.8 also works, other versions might work as well.)
*	Install EspTool: https://github.com/espressif/esptool (use the README on how to install)
* Download the binaries from: https://github.com/energietransitie/twomes-opentherm-gateway-diyless/releases
*	Install the usb driver for the ESP32 for windows, found in the rar file from releases, CH341SER.exe.
### Uploading Firmware To ESP 32
*	Connect device with a USB cable to the PC.
*	Open CMD on Windows
*	cd to the binaries folder, with extracted files from the downloaded release. (I suggest opening the folder in file explore and copy pasting the path in CMD) so the command would be “cd *binaries path*” where you replace the *binaries path* with the copyed path from the file explorer.
*	Then execute: “esptool.py --chip esp32  --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin” without the quotes at the beginning and the end. It should auto detect the USB port that the device is connected to, if not then you can open Device Manager in Windows, go to View and click Show Hidden Devices. Then unfold Ports (COM & LPT) and you should find the device in there, named USB-Serial CH340(COM *) * being the actual port like COM3.
*	If COM port is not auto-detected use the command:  esptool.py --chip esp32 --port "COM3" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin
And replace COM3 with the COM port found in the previous step.
