#pragma once

#include <esp_err.h>

#define BUTTON_WIFI_RESET GPIO_NUM_0
#define LED_WIFI_RESET GPIO_NUM_22

namespace ESP32Dev
{
    /**
     * Initialize GPIO for platform ESPDEV.
     */
    esp_err_t InitializeGPIO();
} // namespace ESP32Dev
