#pragma once

#include <esp_err.h>

namespace Buzzer
{
    /**
     * Configure the buzzer. 
     * This only works on M5STACK_COREINK.
     */
    void Configure();

    /**
     * Buzz the buzzer for specified amount of milliseconds.
     * 
     * @param milliseconds Milliseconds to buzz the buzzer.
     */
    void Buzz(int milliseconds);

    /**
     * Buzz the buzzer for specified amount of milliseconds, a specified amount of times.
     * 
     * @param milliseconds Milliseconds to buzz the buzzer.
     * @param count Amount of times to buzz the buzzer.
     */
    void Buzz(int milliseconds, int count);
} // namespace Buzzer
