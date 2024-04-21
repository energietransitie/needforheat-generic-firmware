# Device info page

The purpose of a device info page is to inform people that encounter your measurement device what the device is doing, via a QR-code that is visible on the device. This information is not su much intended for the persons that installed your measurement device, but other people, like other household members in a residential builing, or colleagues and visitors in a utility builing. 

On a [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) device, after the device was provisioned, the e-ink screen can show a QR-code and a text like "scan for mor info". Your device info page is shown to people that scan the QR-code, typically from their smartphone. 

The contents of the folder [`/docs/device-info/`](https://github.com/energietransitie/needforheat-presence-detector/blob/main/docs/devic-einfo) of a NeedForHeat measurement device repository can be used as the template for your device info page. 

## Prerequisites

To edit and preview the device info page, you need:

- a NeedForHeat firmware repository cloned on your local machine; 
- a web browser;
- a plain text editor (or your favourite IDE).


## Editing
On your local machine, adapt the content of `index.html` to fit the needs of your measurement device type. Be sure to use the language of the people that might scan the device-info-QR-code of your measurement device.

The default styles are:
* `div.video-block` with predefined styling for (embedded videos);
* `div.image-block` with predefined styling for images;
* `div.text-block` with predefined styling for texts.

If you change or add styles in the stylesheet, please make sure this is done consistently across all your device info pages.

## Deploying

> In a future version of this repository, we plan do automate deploying device info pages to GitHub Pages. Most likely, the deployment will be to the `/<github-repository-name>/device_info/` folder of your GitHub Pages server. For example, the NeedForHeat Generic-Test measurement device info page will most likely be hosted at [https://energietransitiewindesheim.nl/needforheat-generic-firmware/device_info/](https://energietransitiewindesheim.nl/needforheat-generic-firmware/device_info/); the NeedForHeat Presence Detection measurement device info page will most likely be hosted at [https://energietransitiewindesheim.nl/needforheat-presence-detector/device_info/](https://energietransitiewindesheim.nl/needforheat-presence-detector/device_info/)

Upload the contents to a webserver, like [GitHub Pages](https://pages.github.com/). Be sure to include all files in the folder `/docs/device_info/`: not only the the file `index.html`, but also all resources referred to in this file, like images.

> The information below is deprecated and applies to member of the [Research Group Energy Transition](https://github.com/energietransitie) only.

Files must be added in a feature branch in a subfolder named `<device_type.name>` of the GitHub Pages repository folder `energietransitie.github.io/<measurement-campaign>/info/<device_type.name>/`. Then create a Pull Request and ask [@hterhofte](https://github.com/hterhofte) to review and merge. After this, the device info pages are available via `https://energietransitiewindesheim.nl/<measurement-campaign>/<device_type.name>/info/`. 

Example

- The contents of the installation manual for the `CO2-meter-SCD4x` device type is stored in GitHub Pages repository folder [energietransitie.github.io/brains4buildings2022/info/CO2-meter-SCD4x/](https://github.com/energietransitie/energietransitie.github.io/tree/main/brains4buildings2022/info/CO2-meter-SCD4x/). 
- We manually created an edu.nl short link [https://edu.nl/4pujw](https://edu.nl/4pujw).
- This edu.nl short link refers to [https://www.energietransitiewindesheim.nl/brains4buildings2022/info/CO2-meter-SCD4x/](https://www.energietransitiewindesheim.nl/brains4buildings2022/info/CO2-meter-SCD4x/).

