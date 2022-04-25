#include "delay.h"

namespace Delay
{
    template <typename T>
    constexpr TickType_t MiliSeconds(T amount)
    {
        // amount is already in miliseconds.
        return amount / portTICK_PERIOD_MS;
    }

    template <typename T>
    constexpr TickType_t Seconds(T amount)
    {
        // amount * 1000 (miliseconds) = seconds.
        return amount * 1000 / portTICK_PERIOD_MS;
    }

    template <typename T>
    constexpr TickType_t Minutes(T amount)
    {
        // amount * 60 (seconds) * 1000 (miliseconds) = minutes.
        return amount * 60 * 1000 / portTICK_PERIOD_MS;
    }
}