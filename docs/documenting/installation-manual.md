# Device installation manual

The purpose of a device installation manual is to help the subjects of your measurement campaign to install, power and connect your your specific measurement device. 

The contents of the folder [`/docs/installation-manual/`](https://github.com/energietransitie/needforheat-presence-detector/blob/main/docs/installation-manual) of a NeedForHeat measurement device repository can be used as a template for this. 

## Prerequisites

To edit and preview the device installation manual, you need:

- a NeedForHeat firmware repository cloned on your local machine; 
- a web browser;
- a plain text editor (or your favourite IDE).

## Editing
Adapt the content of `index.html` to fit the needs of your measurement device type. Be sure to use the language of the intended audience of your [NeedForHeat provisioning app](https://github.com/energietransitie/needforheat-gearup-app). After scanning its QR-code, the device installation manual of your device type is presented to the user by your [NeedForHeat provisioning app](https://github.com/energietransitie/needforheat-gearup-app). 

The default styles are:
* `div.video-block` with predefined styling for (embedded videos);
* `div.image-block` with predefined styling for images;
* `div.text-block` with predefined styling for texts.

If you change or add styles in the stylesheet, please make sure this is done consistently across all your device info pages.

## Previewing
Open the file `preview.html` using a regular web browser to preview your device installation manual as shown in the NeedForHeat app. To make the preview resemble the size and aspect ratio of a smartphone screen:
1. Press the `Ctrl + Shift + I` keys (or `F12` ) on your keyboard to access the Developer Tools.
2. Press the `Ctrl + Shift + M` keys on to enter (mobile) device emulation mode.

By default, your preview is now in 'Responsive' mode. Using the dropdown menu (above the preview), you can also select custom dimensions for specific devices, such as the iPhone 6/7/8.

## Deploying

> In a future version of this repository, we plan do automate deploying device installation manual pages to GitHub Pages. Most likely, the deployment will be to the `/<github-repository-name>/installation_manual/` folder of your GitHub Pages server. For example, the NeedForHeat Generic-Test measurement device installation manual will most likely be hosted at [https://energietransitiewindesheim.nl/needforheat-generic-firmware/installation_manual/](https://energietransitiewindesheim.nl/needforheat-generic-firmware/installation_manual/); the NeedForHeat Presence Detection measurement device installation manual will most likely be hosted at [https://energietransitiewindesheim.nl/needforheat-presence-detector/installation_manual/](https://energietransitiewindesheim.nl/needforheat-presence-detector/installation_manual/)

Upload the contents to a webserver, like [GitHub Pages](https://pages.github.com/). Be sure to include all files in the folder `/docs/installation_manual/`: not only the the file `index.html`, but also all resources referred to in this file, like images.

> The information below is deprecated and applies to member of the [Research Group Energy Transition](https://github.com/energietransitie) only.

For the [NeedForHeat WarmteWachter app](https://github.com/energietransitie/needforheat-gearup-app), files must be added in a feature branch in a  subfolder named `<device_type.name>` of the GitHub Pages repository folder [energietransitie.github.io/manuals/](https://github.com/energietransitie/energietransitie.github.io/tree/main/manuals). Then create a Pull Request and ask [@hterhofte](https://github.com/hterhofte) to review and merge. After this, the installation manual pages are available via `https://energietransitiewindesheim.nl/manuals/<device_type.name>/` for the [NeedForHeat WarmteWachter app](https://github.com/energietransitie/needforheat-gearup-app). 

Example: 

- The contents of the installation manual for the `Generic-Test` device type is stored in GitHub Pages repository folder [energietransitie.github.io/manuals/Presence-Detector/](https://github.com/energietransitie/energietransitie.github.io/tree/main/manuals/Presence-Detector/). 
- The [NeedForHeat WarmteWachter app](https://github.com/energietransitie/needforheat-gearup-app) opens the manual via [https://www.energietransitiewindesheim.nl/manuals/Presence-Detector/](https://www.energietransitiewindesheim.nl/manuals/Presence-Detector/).
- To get a [preview](#previewing) on a regular browser, open [https://www.energietransitiewindesheim.nl/manuals/Presence-Detector/preview.html](https://www.energietransitiewindesheim.nl/manuals/Presence-Detector/preview.html).


