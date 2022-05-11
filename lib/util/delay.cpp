#include "delay.h"

namespace Delay
{
    template <typename T>
    constexpr TickType_t MilliSeconds(T amount)
    {
        // amount [ms] /  portTICK_PERIOD_MS [ticks/ms] = [ticks]
        return amount / portTICK_PERIOD_MS;
    }

    template <typename T>
    constexpr TickType_t Seconds(T amount)
    {
        // amount [s] * 1000 [ms/s] /  portTICK_PERIOD_MS [ticks/ms] = [ticks]
        return amount * 1000 / portTICK_PERIOD_MS;
    }

    template <typename T>
    constexpr TickType_t Minutes(T amount)
    {
        // amount [min] * 60 [s/min] * 1000 [ms/s] /  portTICK_PERIOD_MS [ticks/ms] = [ticks]
        return amount * 60 * 1000 / portTICK_PERIOD_MS;
    }
}