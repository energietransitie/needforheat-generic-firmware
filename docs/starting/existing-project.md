# Integrate with an existing project

For most cases, we recommend to [start with a new project](new-project.md). Should you rather integrate the needforheat-generic-firmware library into code for an existing project, read the steps below.

## PlatformIO project

The needforheat-generic-firmware library can be added to your PlatformIO project by declaring this repository as a dependency in your `platformio.ini` file:

```ini title="platformio.ini"
lib_deps = https://github.com/energietransitie/needforheat-generic-firmware
```

### Configuration

You should also add these lines to your `platformio.ini` file:

```ini title="platformio.ini"
;Build and Debug settings:
build_flags = 
    -DCORE_DEBUG_LEVEL=4                ;Uncommented enables debugging
    -DLOG_LOCAL_LEVEL=4                 ;Uncommented enables debugging
;   -DCONFIG_NFH_CUSTOM_GPIO         ;Uncommented enables custom GPIO mapping 
    -DCONFIG_NFH_PROV_TRANSPORT_BLE  ; uncomment line to support BLE provisioning
;   -DCONFIG_NFH_PROV_TRANSPORT_SOFTAP ; uncomment line to support SoftAP provisioning
;   -DCONFIG_NFH_STRESS_TEST         ;line commented = disabled; line uncommented = enabled
    -DCONFIG_NFH_PRESENCE_DETECTION  ;line commented = disabled; line uncommented = enabled
;   -DCONFIG_NFH_PRESENCE_DETECTION_PARALLEL ;line commented = disabled; line uncommented = enabled; keep disabled for now
    -DCONFIG_NFH_TEST_SERVER         ;line uncommented = use test server; line commented = use other server
;   -DCONFIG_NFH_PRODUCTION_SERVER   ;line uncommented = use production server; line commented = use other server
;   -DCONFIG_NFH_OTA_FIRMWARE_UPDATE ;line commented = disabled; line uncommented = enabled
    -D"$PIOENV"
```

> See [`platformio.ini`](https://github.com/energietransitie/needforheat-generic-firmware/blob/main/platformio.ini) for all settings.
