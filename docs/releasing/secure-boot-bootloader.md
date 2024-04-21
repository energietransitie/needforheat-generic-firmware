# Secure bootloader

This section contains information how to build a secure bootloader for NeedForHeat with ESP-IDF. There is anonther page that describes [how to build NeedForHeat firmware that uses a secure bootloader](secure-boot-firmware.md). For more general information, you may also want to read the [Espressif secure boot documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/secure_boot_v1.html).

You only need to follow these steps for an ESP32 that does not have secure boot enabled yet, or had its bootloader overwritten.

## Prerequisites 
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#manual-installation) installed.

## Build bootloader
Use the following command to build the bootloader from the root of the repository:
```shell
idf.py bootloader -C bootloader/
```

## Burn secure boot key to eFuse
This only needs to be performed once per ESP32.
Use the following command to burn the secure boot key to the eFuse and follow the instructions:
```shell
espefuse.py burn_key secure_boot_v1 bootloader/build/bootloader/secure-bootloader-key-256.bin
```

## Flash bootloader for the first time
Use the following command to flash the bootloader for the first time:
```shell
esptool.py --chip esp32 --before=default_reset --after=no_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader/build/bootloader/bootloader.bin
```

## Flash bootloader after initial flash
Use the following command to flash the bootloader after initial flash:
```shell
esptool.py --chip esp32 --before=default_reset --after=no_reset write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0 bootloader/build/bootloader/bootloader-reflash-digest.bin
```