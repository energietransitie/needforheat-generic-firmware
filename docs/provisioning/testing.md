# Testing NeedForHeat provisoning

To test NeedForHeat provisioning fully, you should: 

- use a test instance of the [NeedForHeat API](https://github.com/energietransitie/needforheat-server-api) connected with a test instance of a [NeedForHeat server](https://github.com/energietransitie/needforheat-server-configuration); 
- compile firmware using swiches that target a test version of the [NeedForHeat API](https://github.com/energietransitie/needforheat-server-api) connected with a test instance of a [NeedForHeat server](https://github.com/energietransitie/needforheat-server-configuration);
- [prepare a test device](../deploying/device-preparation.md);
- perform device provisioning with a test version of the NeedForHeat WarmteWachter app (see below).

> For a future version of the NeedForHeat [firmware](https://github.com/energietransitie/needforheat-generic-firmware), [API](https://github.com/energietransitie/needforheat-server-api), [server](https://github.com/energietransitie/needforheat-server-configuration) and [app](https://github.com/energietransitie/needforheat-gearup-app), we consider allowing the provisioning app to determine whether the (preconfigured) test server URI or production server URI is targeted. 

## Testing device provisioning using WarmteWachter
Use a test version of the [NeedForHeat WarmteWachter app](https://github.com/energietransitie/needforheat-gearup-app) to test the full NeedForHeat device installation flow, part of which is based on using Espressif Unified Provisioning. 

## Testing device provisioning using a Unified Provisioning app
Alternatively, you may use test apps that only support Wi-Fi provisioning using Espressif Unified Provisioning; these can be found at:

* [Android Unified Provisioning app for BLE](https://play.google.com/store/apps/details?id=com.espressif.provble&hl=en&gl=US)
* [Android Unified Provisioning app for SoftAP](https://play.google.com/store/apps/details?id=com.espressif.provsoftap&hl=en&gl=US)
* [Apple Unified Provisioning app for BLE](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)
* [Apple Unified Provisioning app for SoftAP](https://apps.apple.com/us/app/esp-softap-provisioning/id1474040630)

> Using one of the above Espressif Unified Provisioning apps, you cannot test NeedForHeat device provisioning fully: you can only test the Wi-Fi provisioning part.
