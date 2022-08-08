#include "platform_esp32_dev.hpp"

#include <driver/gpio.h>

namespace ESP32Dev
{
    esp_err_t InitializeGPIO()
    {
        // Configure Wi-Fi reset button.
        gpio_config_t gpioConfigIn = {};
        gpioConfigIn.intr_type = GPIO_INTR_NEGEDGE;
        gpioConfigIn.mode = GPIO_MODE_INPUT;
        gpioConfigIn.pin_bit_mask = 1 << BUTTON_WIFI_RESET;
        gpioConfigIn.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpioConfigIn.pull_up_en = GPIO_PULLUP_ENABLE;

        auto err = gpio_config(&gpioConfigIn);
        if (err != ESP_OK)
            return err;

        // Configure Wi-Fi reset LED.
        gpio_config_t gpioConfigOut = {};
        gpioConfigOut.intr_type = GPIO_INTR_DISABLE;
        gpioConfigOut.mode = GPIO_MODE_OUTPUT;
        gpioConfigOut.pin_bit_mask = 1 << LED_WIFI_RESET;
        gpioConfigOut.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpioConfigOut.pull_up_en = GPIO_PULLUP_DISABLE;

        err = gpio_config(&gpioConfigOut);
        if (err != ESP_OK)
            return err;

        return ESP_OK;
    }
} // namespace ESP32Dev
