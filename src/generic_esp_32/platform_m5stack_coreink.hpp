#pragma once

#include <esp_err.h>
#include <driver/gpio.h>

#ifndef CONFIG_TWOMES_CUSTOM_GPIO
#define BUTTON_WIFI_RESET GPIO_NUM_5
#define LED_WIFI_RESET GPIO_NUM_10
#define BUTTON_ON_OFF GPIO_NUM_27
#else
#define BUTTON_WIFI_RESET GPIO_NUM_MAX
#define LED_WIFI_RESET GPIO_NUM_MAX
#define BUTTON_ON_OFF GPIO_NUM_MAX
#endif // CONFIG_TWOMES_CUSTOM_GPIO

namespace M5StackCoreInk
{
    /**
     * Initialize GPIO for platform M5STACK_COREINK.
     */
    esp_err_t InitializeGPIO();
} // namespace M5StackCoreInk
