#ifndef DELAY_H
#define DELAY_H

// These function definitions need to be inside the header file, because of the use of constexpr.

#include "freertos/FreeRTOS.h"

namespace Delay
{
    /**
     * Convert miliseconds to FreeRTOS ticks.
     *
     * @returns The amount of FreeRTOS ticks.
     */
    constexpr TickType_t MiliSeconds(TickType_t amount)
    {
        // amount is already in miliseconds.
        return amount / portTICK_PERIOD_MS;
    }

    /**
     * Convert seconds to FreeRTOS ticks.
     *
     * @returns The amount of FreeRTOS ticks.
     */
    constexpr TickType_t Seconds(TickType_t amount)
    {
        // amount * 1000 (miliseconds) = seconds.
        return amount * 1000 / portTICK_PERIOD_MS;
    }

    /**
     * Convert minutes to FreeRTOS ticks.
     *
     * @returns The amount of FreeRTOS ticks.
     */
    constexpr TickType_t Minutes(TickType_t amount)
    {
        // amount * 60 (seconds) * 1000 (miliseconds) = minutes.
        return amount * 60 * 1000 / portTICK_PERIOD_MS;
    }
}

#endif // DELAY_H
