# Configuring presence detection

If your measurement device should support presence detection, for each device, after deploying the firmware to the device, you should perform additional steps:

1. edit the nsv.csv file;
2. generate the nvs.bin partition file
3. deploy the nvs.bin partition to a specific measurement device

## 1. Edit the nvs.csv file

The MAC-addresses used for presence detection should be defined in the file `nvs.csv`;

- In a line that starts with `mac_addresses,data,string,`, one or more Bluetooth MAC-addresses can be defined.
- A MAC-address can be written like `AB:CD:EF:01:23:45` or `AB-CD-EF-01-23-45` and may be upper- or lower-case.
- When multiple MAC-addresses are defined, separate them with `;` without spaces.
- Make sure that the csv-file does not contain any unnecessary spaces.

> WARNING: Bluetooth MAC Addresses constitute personal data. NEVER push an nvs.csv file that contains true Bluetooth MAC addresses from (devices of) real persons to a (Public) GitHub respository.

## 2. Generate the nvs.bin partition file

1. Open a comand prompt in the directory of the nvs.csv file you just edited.
2. Use the command below to generate the partition:
    ```shell
    py -m $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen generate nvs.csv nvs.bin 16384
    ```
## 3. Deploy the nvs.bin partition to a specific measurement device

1. Open a comand prompt in the directory of the nvs.bin file you just generated.
2. Use the command below to generate the partition:
    ```shell
    py -m esptool --chip esp32 --baud 460800 write_flash 0x19000 nvs.bin
    ```
> Should you encounter issues you may try to replace `py -m <command>` in the above commands with:
>
> - `python -m <command>` 
> - `python3 -m <command>`
> - `<command>.py`