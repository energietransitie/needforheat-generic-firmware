# Generic Software For The ESP32 and ESP8266(broken right now)
## Features
This software includes the following features:
* NTP(Time Synchronisation and Timestamping)
* WiFi Provisioning(SoftAP and Bluetooth(ESP32 Only))
* HTTP POST and HTTPS POST with configurable URL, Data and Certificate

## How to use?
Install Visual Studio Code: https://code.visualstudio.com/download
Install PlatformIO for Visual Studio Code: https://platformio.org/install/ide?install=vscode
Clone This GitHub Repo:
  * git@github.com:KevinJan18/esp32genericrtos.git for Git with SSH
  * https://github.com/KevinJan18/esp32genericrtos.git Git without SSH
  * You can use GitHub Desktop Application for Windows: https://desktop.github.com/
  * Or you can download everything as a .zip file: https://github.com/KevinJan18/esp32genericrtos/archive/refs/heads/master.zip and extract it sowewhere.
  
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
  
