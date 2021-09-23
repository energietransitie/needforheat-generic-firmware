# Twomes presence detection library
This library can be used to enable presence detection of occupants in a home in [Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware), by detecting the presence of absence of their smartphones via Bluetooth.

## Table of contents
* [General info](#general-info)
* [Prerequisites](#prerequisites)
* [Deploying](#deploying)
* [Developing](#developing) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
Presence detection of home occupants helps the Twomes research project in three ways:
1. Humans exhale CO₂ and are one of the primary sources of CO₂ in a room or house. Presence detection data provides additional insights in the variations that cause the CO₂-concentration in a room or house to rise. This additional data is expected to increase the precision of algorithms that aim to establish the contribution of voluntary ventilation to heat losses, based on CO₂-concentrations measured with the [Twomes CO₂ meter device](https://github.com/energietransitie/twomes-co_2-meter).
2. Detecting presence in combination with thermostat programs, setpoints or indoor temperatures allow for the detection of inefficient heating behaviour and/or settings, such as heating a home when no one is present (soon).
3. Humans produce (a little) heat: the average Dutch citizen produces about 80 W when sleeping and about 100 W when awake. When present in the home, this heat contributes (a little) to the heating balance of the home. Data about the presence of absence of occupants helps to improve the heating balance (a little), compared to just taking the average sleeping, presence, and absence patterns of Dutch citizens.

Presence detection is performed via Bluetooth by one or more Twomes measurement devices. Since ESP8266 SoCs do not support Bluetooth, only newer ESP devices, such as the ESP32 can be used as a presence detection device. Whether a Twomes measurement device is enabled for presence detection is determined at compile time. If enabled for presence detection, Twomes measurement devices scan the presence of well-known smartphones of home occupants at regular intervals (e.g., every 10 minutes). 

The mechanism used, a Bluetooth [name request](https://www.amd.e-technik.uni-rostock.de/ma/gol/lectures/wirlec/bluetooth_info/lmp.html#Name%20Request), requires knowledge at the Twomes measurement device of the static Bluetooth MAC address of home occupants. The name request is a primitive of the Bluetooth GAP LWP-layer that all smartphones respond to. A more common use of this primitive is after initiation of device discovery by a device, which is responded to only by devices in discoverable mode, with their static Bluetooth address. Subsequently, the initiator issues the name request. We only issue the name request to well-known smartphones whose owners have opted in to their presence being tracked and who have given us their static Bluetooth address as part of this procedure.

We also considered using another approach, just listening for Bluetooth advertisements and counting the number of unique Bluetooth MAC-addresses at any given moment, but decided against this apprach for the following reasons:
* many contemporary smartphones employ MAC address randomization;
* this may cause tracking of the MAC address of people who did not give informed consent;
* the [Dutch Autoriteit Persoonsgegevens considers MAC-addresses to be personal data](https://autoriteitpersoonsgegevens.nl/nl/onderwerpen/internet-telefoon-tv-en-post/internet-en-telecom#verwerk-ik-als-organisatie-persoonsgegevens-met-wifitracking-en-bluetoothtracking-6963) and did not specify an exemption for randomized MAC addresses;
* not just smartphones have a (random) Bluetooth MAC address that is advertised: also smart watches, fitness trackers, laptops, digital photo frames; this may cause a large and unknown overestimation of the number of persons present in the home.

So, ultimately, we chose the approach using targeted Bluetooth name requests to well-known smartphones of persons that gave consent for their presence to be tracked via their smartphone.

## Prerequisites
In addition to [prerequisites of Twomes measurement devices in general](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/README.md#prerequisites), in the current version of the firmware, before [developing](#developing) and [deploying](#deploying), you need to obtain the static Bluetooth MAC addresses of the smartphones of the inhabitants of the home whose presence you want to detect. Currently, there is no automated support for this. Instructions:
* on English Android devices, go to `Settings` > `System` (skip on some models) > `About Phone` > `Status`; the value can be read under `Bluetooth address` (at least, on Android 10, provided that Bluetooth is turned on);
* on Dutch Android devices, go to `Instellingen` > `Systeem` (skip on some models) > `Info telefoon` > `Status`; the value can be read under `Bluetooth-adres` (at least, on Android 10, provided that Bluetooth is turned on);
* on English iPhones, go to `Settings` > `General` > `About`; the value can be read under `Bluetooth` and copied via press-and-hold on the entry (at least, on iOS 12);
* on Dutch iPhones, go to `Instellingen` > `Algemeen` > `Info`; the value can be read under `Bluetooth` and copied via press-and-hold on the entry (at least, on iOS 12).

On all devices, the static Bluetooth MAC address is represented on the screen something like `1A:2B:C3:D4:5E:6F`, i.e. as six groups of two hexadecimal upper case digits, concatenated by colons. When in doubt: the letter `O` is never part of such an address; you're reading the digit `0`.

## Deploying
The current version of the presence-detection-enabled firmware requires that you add the list of static Bluetooth MAC addresses to detect the presence of during compilation. For more information, see the next section, [Developing](#developing).

Deployment of binary releases can be done in the same way as other [Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/README.md#deploying).

## Developing
See [Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/README.md#developing).

In addition, in the current version you have to make changes the source code.

To enable presence detection library, before compiling the sources, make sure you have an uncommented line in the `CMakeLists.txt file` that reads 
````shell
add_compile_definitions(CONFIG_TWOMES_PRESENCE_DETECTION)
```` 

You also have to add the static Bluetooth addresses in the file `presence_detection.c` file. Change the entries in the `presence_addr_list`, add or remove entries if needed and be sure to change the value of `change presence_addr_count` correspondingly.

## Features
List of features ready and TODOs for future development. Ready:
* scanning for well-known static Bluetooth addresses;
* uploading presence data to Twomes server via Wi-Fi.

To-do:
* provisioning of the account-specific list of static Bluetooth addresses during device preparation phase in the lab via a file on non-volatile device storage (most likely, SPIFFS), which is read by the firmware at startup;
* provisioning of the account-specific list of static Bluetooth addresses during device provisioning phase in the home via an API-call.  

## Status
Project is: _in progress_

## License
See [Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/README.md#license).

## Credits
See [Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/README.md#credits).

This work was inspired by the [monitor](https://github.com/andrewjfreyer/monitor) program for Raspberry Pi, by Andrew J Freyer.
