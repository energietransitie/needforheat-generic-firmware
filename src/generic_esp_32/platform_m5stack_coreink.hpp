#pragma once

#include <esp_err.h>

#define BUTTON_WIFI_RESET GPIO_NUM_5
#define LED_WIFI_RESET GPIO_NUM_10
#define BUTTON_ON_OFF GPIO_NUM_27

namespace M5StackCoreInk
{
    /**
     * Initialize GPIO for platform M5STACK_COREINK.
     */
    esp_err_t InitializeGPIO();
} // namespace M5StackCoreInk
