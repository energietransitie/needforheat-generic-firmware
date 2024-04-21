#pragma once

#include <esp_err.h>
#include <driver/gpio.h>

#ifndef CONFIG_NFH_CUSTOM_GPIO
#define BUTTON_WIFI_RESET GPIO_NUM_0
#define LED_WIFI_RESET GPIO_NUM_22
#else
#define BUTTON_WIFI_RESET GPIO_NUM_MAX
#define LED_WIFI_RESET GPIO_NUM_MAX
#endif // CONFIG_NFH_CUSTOM_GPIO

namespace ESP32Dev
{
    /**
     * Initialize GPIO for platform ESPDEV.
     */
    esp_err_t InitializeGPIO();
} // namespace ESP32Dev
