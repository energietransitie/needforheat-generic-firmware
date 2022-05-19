# Secure boot
This section describes how to build the firmware for an ESP32 with a secure bootloader. [Click here](bootloader/README.md) to read more about how to build and flash a secure bootloader.

## Prerequisites 
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#manual-installation) installed.

## Build the project
1. Click the PlatformIO Icon (the little alien in the left of the screen).
2. Unfold esp32dev.
3. Click `Build`.

## Sign the firmware
You can use the secure boot signing key to sign the firmware. You need to have the key as a file named `secure_boot_signing_key.pem` in the root of this repository. Run the following command to sign the firmware:
```shell
espsecure.py sign_data -v1 --keyfile secure_boot_signing_key.pem --output .pio/build/esp32dev/firmware-signed.bin .pio/build/esp32dev/firmware.bin
```

## Flash the signed firmware
Run the following command to flash the signed firmware:
```shell
esptool.py --chip esp32 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x18000 .pio/build/esp32dev/partitions.bin 0x1d000 .pio/build/esp32dev/ota_data_initial.bin 0x20000 .pio/build/esp32dev/firmware-signed.bin
```

## GitHub Actions workflow
The [GitHub Actions workflow](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/m5coreink-ota/.github/workflows/release.yml) needs a secret with the name `TWOMES_SECURE_BOOT_SIGNING_KEY` in order to sign the firmware. This key needs to be set in `Settings > Secrets > Actions`.

1. Generate a secure boot signing key:
  ```shell
  openssl ecparam -name prime256v1 -genkey -noout -out my_secure_boot_signing_key.pem
  ```
  > KEEP THIS KEY SAFE! 
  > This key is needed to sign new firmware for a device with secure boot enabled. If this key is lost, no more new firmware can be installed on the device. 
  > Copying this key as a secret for the GitHub Actions workflow will make it available for the build workflow to sign new firmware releases.
2. Copy the contents of the key and paste them as a new secret `TWOMES_SECURE_BOOT_SIGNING_KEY` in `Settings > Secrets > Actions` of the repository.
