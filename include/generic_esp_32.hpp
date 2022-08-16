#pragma once

#include <string>

#include <util/http_util.hpp>

constexpr const char *ENDPOINT_VARIABLE_UPLOAD = "/device/measurements/variable-interval";
constexpr const char *ENDPOINT_FIXED_INTERVAL_UPLOAD = "/device/measurements/fixed-interval";
constexpr const char *ENDPOINT_DEVICE_ACTIVATION = "/device/activate";

namespace GenericESP32Firmware
{
    /**
     * Initialize generic ESP32 firmware.
     *
     * The firmware enables provisioning with the 'warmtewachter' app
     * and sending measurement data to the twomes backend API.
     *
     * @param deviceTypeName Name of a valid device type.
     */
    void Initialize(const std::string &deviceTypeName);

    /**
     * Get the device's service name; prefixe and last 3 octets of Wi-Fi MAC-address.
     *
     * @returns The device's service name.
     */
    std::string GetDeviceServiceName();

    /**
     * Initialize time synchronization via NTP and sync with internal clock.
     *
     * This will sync every hour by default, controlled by CONFIG_LWIP_SNTP_UPDATE_DELAY.
     */
    void InitializeTimeSync();

    /**
     * Send an HTTPS POST request to the backend API.
     *
     * @param endpoint API endpoint.
     * @param dataSend Data to send in the POST request.
     * @param dataReceive Response data from the POST request.
     * @param useBearer Optional parameter (default true) to use bearer or not.
     *
     * @returns HTTP status code.
     */
    int PostHTTPSToBackend(const std::string &endpoint,
                           HTTPUtil::buffer_t &dataSend,
                           HTTPUtil::buffer_t &dataReceive,
                           bool useBearer = true);
} // namespace GenericESP32Firmware
