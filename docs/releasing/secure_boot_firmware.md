# Secure boot firmware

This section describes how to sign Twomes firmware to work on an ESP32 with a secure bootloader. There is anonther page that describes [how to build and deploy a secure bootloader for Twomes](secure_boot_bootloader.md). 

For more information, see the [Espressif secure boot documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/secure-boot-v1.html).

## Prerequisites 
Before signing the firmware, you should first [build the firmware](./building.md).

## Sign the firmware
You can use the secure boot signing key of your own organisation to sign the firmware. You need to have the key as a file named `secure_boot_signing_key.pem` in the root of this repository. Run the following command to sign the firmware:
```shell
espsecure.py sign_data -v1 --keyfile secure_boot_signing_key.pem --output .pio/build/esp32dev/firmware-signed.bin .pio/build/esp32dev/firmware.bin
```

## Deploy the signed firmware
Run the following command to deploy the signed firmware:
```shell
esptool.py --chip esp32 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x18000 .pio/build/esp32dev/partitions.bin 0x1d000 .pio/build/esp32dev/ota_data_initial.bin 0x20000 .pio/build/esp32dev/firmware-signed.bin
```

## GitHub Actions workflow
The [GitHub Actions workflow](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/.github/workflows/release.yml) needs a secret with the name `WINDESHEIM_ENERGIETRANSITIE_SIGNING_KEY` in order to sign the firmware. This key needs to be set in `Settings > Secrets > Actions`.

1. Generate a secure boot signing key:
  ```shell
  openssl ecparam -name prime256v1 -genkey -noout -out my_secure_boot_signing_key.pem
  ```
  > KEEP THIS KEY SAFE! 
  > This key is needed to sign new firmware for a device with secure boot enabled. If this key is lost, no more new firmware can be installed on the device. 
  > Copying this key as a secret for the GitHub Actions workflow will make it available for the build workflow to sign new firmware releases.
2. Copy the contents of the key and paste them as a new secret `WINDESHEIM_ENERGIETRANSITIE_SIGNING_KEY` in `Settings > Secrets > Actions` of the repository.


