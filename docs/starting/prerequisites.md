# Prerequisites for developing

Prerequisites are [the prerequisites for deploying](../deploying/prerequisites.md), plus:

*	[Visual Studio Code](https://code.visualstudio.com/download) installed
*	[PlatformIO for Visual Studio Code](https://platformio.org/install/ide?install=vscode) installed
*	[CMake](https://cmake.org/download/) installed
*	[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extension for Visual Studio Code installed
*	In [Visual Studio Code](https://code.visualstudio.com/download), open a PlaformIO Command Line Interface and enter
	```shell
	pio platform update
	```
*	If your user directory name contains a whitespace, you may need to change the platformio.ini file. To avoid the error message "Error: Detected a whitespace character in project paths" when building an ESP-IDF project with PlatformIO, add the following lines to the platformio.ini file, as documented in [https://docs.platformio.org/en/latest/frameworks/espidf.html#limitations](https://docs.platformio.org/en/latest/frameworks/espidf.html#limitations):  
	```
	[platformio]
	core_dir = <path_without_whitespaces>
	```
*	This GitHub repository cloned

> Refer to [developing using a devcontainer ](devcontainer.md) should you prefer that way of developing.


## Coding and uploading
We recommend using a device without [secure boot](../releasing/secure-boot-firmware.md) for developing. Follow the steps below to build the firmware and flash it to a device:

1. In the top-left corner, select File -> Open Folder.
2. Select the folder where you cloned or extracted the repository.
3. Wait for PlatformIO to finish initialization. This step is done until you no longer see "PlatformIO: Loading tasks..." in the VS Code status bar.
	> ⚠️ This step is important! This may take up to ±10 minutes. If you do not wait for this to finish, the project will not be able to build correctly.
4. In the `src` folder you will find `main.cpp`; this is an example application.
5. In the `lib` folder you will find the library that this repository contains.
6. To change between provisioning via BLE or SoftAP, in the `platformio.ini`file, comment the line including `-DCONFIG_EXAMPLE_PROV_TRANSPORT_BLE` and uncomment the line including `-DCONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP` or vice versa.
7. Change the code in `src/main.c` as you see fit.
8. Build the firmware. See also our documentation on how to [build NeedForHeat firmware](../releasing/building.md)
