#ifndef DELAY_H
#define DELAY_H

// These function definitions need to be inside the header file, because of the use of constexpr.

#include "freertos/FreeRTOS.h"

namespace Delay
{
    /**
     * Convert milliseconds to FreeRTOS ticks.
     *
     * @returns amount of milliseconds, converted to FreeRTOS ticks.
     */
    constexpr TickType_t MilliSeconds(TickType_t amount)
    {
        // amount [ms] /  portTICK_PERIOD_MS [ticks/ms] = [ticks]
        return amount / portTICK_PERIOD_MS;
    }

    /**
     * Convert seconds to FreeRTOS ticks.
     *
     * @returns amount of seconds, converted to FreeRTOS ticks.
     */
    constexpr TickType_t Seconds(TickType_t amount)
    {
        // amount [s] * 1000 [ms/s] /  portTICK_PERIOD_MS [ticks/ms] = [ticks]
        return amount * 1000 / portTICK_PERIOD_MS;
    }

    /**
     * Convert minutes to FreeRTOS ticks.
     *
     * @returns amount of minutes, converted to FreeRTOS ticks.
     */
    constexpr TickType_t Minutes(TickType_t amount)
    {
        // amount [min] * 60 [s/min] * 1000 [ms/s] /  portTICK_PERIOD_MS [ticks/ms] = [ticks]
        return amount * 60 * 1000 / portTICK_PERIOD_MS;
    }
}

#endif // DELAY_H
