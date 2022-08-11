#pragma once

#include <vector>
#include <memory>
#include <string>

#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace Buttons
{
    // Forward declare so the Button struct can use it.
    class ButtonPressHandler;

    using ButtonCallback_t = void (*)();

    /**
     * Struct that holds information for a button.
     */
    struct Button
    {
        ButtonPressHandler *handler;
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
    class ButtonPressHandler
    {
    public:
        /**
         * Initialize ButtonPressHandler.
         */
        ButtonPressHandler();

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

    private:
        /**
         * FreeRTOS task that is used to handle button presses.
         *
         * @param pvParams Task parameters.
         */
        friend void ButtonPressHandlerTask(void *pvParams);

        /**
         * Interrupt service handler for button presses.
         *
         * @param pvParams Task parameters.
         */
        friend void IRAM_ATTR ISRHandlerGPIO(void *pvParams);

    private:
        /**
         * Event queue used by the ISR and handler task.
         */
        xQueueHandle m_gpioEventQueue;

        /**
         * Vector to keep track of all buttons.
         */
        std::vector<std::unique_ptr<Button>> m_buttons;
    };
} // namespace Buttons
