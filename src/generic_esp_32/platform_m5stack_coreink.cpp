#include "platform_m5stack_coreink.hpp"

#include <specific_m5coreink/powerpin.h>
#include <util/buzzer.hpp>

constexpr gpio_num_t PIN_BUZZER = GPIO_NUM_2;

namespace M5StackCoreInk
{
    esp_err_t InitializeGPIO()
    {
        // Setup power pin.
        powerpin_set();

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
        err = gpio_config(&gpioConfigOut);
        if (err != ESP_OK)
            return err;
        // LED is off when level is 1.
        gpio_set_level(LED_WIFI_RESET, 1);

        // Configure power-off button.
        gpio_config_t gpioConfigPwrOff = {};
        gpioConfigPwrOff.intr_type = GPIO_INTR_NEGEDGE;
        gpioConfigPwrOff.mode = GPIO_MODE_INPUT;
        gpioConfigPwrOff.pin_bit_mask = 1 << BUTTON_ON_OFF;
        gpioConfigPwrOff.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpioConfigPwrOff.pull_up_en = GPIO_PULLUP_ENABLE;
        err = gpio_config(&gpioConfigPwrOff);
        if (err != ESP_OK)
            return err;

        // Configure the buzzer that is used to signal powering off.
        Buzzer::Configure();

        return ESP_OK;
    }
} // namespace M5StackCoreInk
