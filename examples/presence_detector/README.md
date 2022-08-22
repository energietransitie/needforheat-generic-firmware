# Presence detector example

This example is a PlatformIO project that uses the twomes-generic-esp-firmware library.

## Features
The example features all generic firmware tasks, including presence detection with MAC-addresses loaded from NVS storage.

The MAC-addresses used for presence detection can be defined in [`nvs.csv`](nvs.csv).
- One or more MAC-Addresses can be defined.
- A MAC-address can be written like `AB-CD-EF-01-23-45` or `AB:CD:EF:01:23:45` and may be upper- or lower-case.
- When multiple MAC-addresses are defined, separate them with `;` without spaces.
- Make sure that the csv-file does not contain any unnecessary spaces.

## Supported devices
This example was tested on:
- LilyGO TTGO T7 and 
- M5Stack Core INK.

## Usage

> :warning: Warning! Flashing an NVS partition will clear all other configuration on your device. It will need to re-activated with the backend and provisioned again.

### Prerequisites
The following prerequisites are required to use this example:
- All of the prerequisites described in the [developing section](../../README.md#prerequisites-1) of the main README.
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#installation) installed.

### Steps
Follow the steps below to compile and build this example firmware, generate an NVS partition and flash the example firmware and the NVS partition to a supported device.

1. Compile and flash the firmware using PlatformIO:
    1. Open the example directory in VS Code with PlatformIO.
    2. Use the keyboard shortcut `CTRL+SHIFT+P` and choose `PlatformIO: Switch Project Environment`.
    3. Use the keyboard shortcut `CTRL+SHIFT+P` again and choose `PlatformIO: Upload`.
2. Generate the NVS partition:
    1. Open the example directory in a shell (can be a VS Code shell).
    2. Use the command below to generate the partition:
        ```shell
        python3 $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate nvs.csv nvs.bin 16384
        ```
3. Flash the NVS partition:
    1. Open the example directory in a shell (can be a VS Code shell).
    2. Use the command below to generate the partition:
        ```shell
        python3 -m esptool --chip esp32 --baud 460800 write_flash 0x19000 nvs.bin
        ```
