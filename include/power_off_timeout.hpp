#pragma once

#include <string>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace M5CoreInkSpecific
{
    /**
     * PowerOffTimeout will power off the M5CoreInk after a specified timeout.
     */
    class PowerOffTimeout
    {
    public:
        PowerOffTimeout() = delete;
        PowerOffTimeout(const PowerOffTimeout &) = delete;
        PowerOffTimeout(PowerOffTimeout &&) = delete;

        /**
         * Create a new PowerOffTimeout object.
         * The device will power off after the timeout.
         *
         * @param timeout_s Timout in seconds.
         */
        PowerOffTimeout(int timeout_s);

        /**
         * Start the timout.
         * When the timeout ends before it is canceled,
         * the device will be powered off.
         *
         * When start is called more than once, the timer keeps running. It is not reset.
         */
        void Start();

        /**
         * Cancel the timout.
         *
         * If the timout is not running, nothing will happen.
         */
        void Cancel();

    private:
        int m_timeout_s;

        TaskHandle_t m_taskHandle;
    };
} // namespace M5CoreInkSpecific
