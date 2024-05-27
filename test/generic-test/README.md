# NeedForHeat Generic-Test example

This example is a PlatformIO project that uses the [needforheat-generic-firmware](https://github.com/energietransitie/needforheat-generic-firmware) library. To work with this example, (clone this reposotory and) open the root directory of the NeedForHeat Generic-Test example in Visual Studio Code.

## Table of contents
* [General info](#general-info)
* [Deploying](#deploying)
* [Developing](#developing) 
* [Supported devices](#supported-devices)
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
The NeedForHeat Generic-Test example only sends [generic data sent by any NeedForHeat measurement device](https://github.com/energietransitie/needforheat-generic-firmware#readme) a NeedForHeat server.

## Deploying
To deploy this software, see the [deploying section in the needforheat-generic-firmware library documentation](https://www.energietransitiewindesheim.nl/needforheat-generic-firmware/deploying/prerequisites/)

## Developing
To develop software for, or based on this software, see the [developing section in the needforheat-generic-firmware library documentation](https://www.energietransitiewindesheim.nl/needforheat-generic-firmware/getstarted/prerequisites/)

## Supported devices
This example was tested on:
- [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk)

This example still needs to be tested n:
- [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3)

## Features
The example features generic firmware tasks, excluding Bluetooth-based presence detection. For an example that includes presence detection, we refer to the NeedForHeat Presence-Detector example](../presence_detector/README.md).

Ready:
* Generic firmware tasks
* Use [NeedForHeat generic ESP32 firmware library](https://github.com/energietransitie/needforheat-generic-firmware)

To-do:
* Move manual page(s) from needforheat-generic-firmware folder to this example
* Move device info page(s) from energietransitie-github-io repository to this example

## Status
Project is: _in progress_

## License
This software is available under the [Apache 2.0 license](./LICENSE), Copyright 2022 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits
This software was created by:
* Nick van Ravenzwaaij · [@n-vr](https://github.com/n-vr)

Product owners:
* Henri ter Hofte · [@henriterhofte](https://github.com/henriterhofte) · Twitter [@HeNRGi](https://twitter.com/HeNRGi)

We use and gratefully acknowlegde the efforts of the makers of the following source code and libraries:
* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache License 2.0](https://github.com/espressif/esp-idf/blob/9d34a1cd42f6f63b3c699c3fe8ec7216dd56f36a/LICENSE)
* [NeedForHeat Generic ESP Firmware](https://github.com/energietransitie/needforheat-generic-firmware), by Research group Energy Transition, Windesheim University of Applied Sciences, licensed under [Apache License 2.0](https://github.com/energietransitie/needforheat-generic-firmware/blob/main/LICENSE.md)