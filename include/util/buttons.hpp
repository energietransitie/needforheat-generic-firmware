#pragma once

#include <vector>
#include <memory>
#include <string>

#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace Buttons
{
    using ButtonCallback_t = void (*)();

    /**
     * Struct that holds information for a button.
     */
    struct Button
    {
        gpio_num_t gpioNum;
        std::string name;
        int trigger;
        ButtonCallback_t shortPressCallback;
        ButtonCallback_t longPressCallback;
    };

    /**
     * Button press handler.
     *
     * Handles buttons using interrupts.
     * Can detect short or long presses and use callbacks.
     */
    namespace ButtonPressHandler
    {
        /**
         * Add a new button to the handler.
         *
         * @param gpioNum GPIO number.
         * @param name Button name.
         * @param trigger Specify if the signal is HIGH (1) or LOW (0) when the button is pressed.
         * @param cbShort Callback function for short presses.
         * @param cbShort Callback function for long presses.
         *
         * @returns ESP error.
         */
        esp_err_t AddButton(gpio_num_t gpioNum,
                            const std::string &name,
                            int trigger,
                            ButtonCallback_t cbShort,
                            ButtonCallback_t cbLong);
    };
} // namespace Buttons
