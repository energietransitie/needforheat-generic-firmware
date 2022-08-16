#pragma once

#include <esp_err.h>

#define BUTTON_WIFI_RESET GPIO_NUM_0
#define LED_WIFI_RESET GPIO_NUM_22

namespace ESP32Dev
{
    /**
     * Wait for the next measurement interval for platform ESP32DEV.
     */
    void WaitUntilNextTaskTime(time_t nextTaskTime_s);
} // namespace ESP32Dev
