#pragma once

#include <string>

#include <esp_timer.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace Timer
{
    /**
     * Predefined timouts to pick and possibly multiply to with, to get a desired timeout.
     *
     * For example, if you need a timeout of 2 minutes, use: Timer::Timeout::MINUTE * 2
     */
    namespace Timeout
    {
        constexpr const uint64_t MILLISECOND = 1;
        constexpr const uint64_t SECOND = MILLISECOND * 1000;
        constexpr const uint64_t MINUTE = SECOND * 60;
    } // namespace Timeout

    using TimerCallback = void (*)();

    /**
     * A Timer can run a callback after a specified timeout.
     * The timer can be started, reset or stopped.
     */
    class Timer
    {
    public:
        Timer() = delete;

        /**
         * Create a new Timer.
         *
         * @param name Name for the timer.
         * @param callback Function to call when timer expires. Don't use big stack variables in this function.
         * @param timeout_s Timer timeout in milliseconds. Use Timer::Timeout constants to multiply with (e.g. Timer::Timeout::MINUTE * 2 for 2 minutes).
         */
        Timer(const std::string &name, TimerCallback callback, uint64_t timeout_ms);

        /**
         * Start the timer.
         *
         * If the timer was already started, it will be reset.
         */
        void StartOrReset();

        /**
         * Stop the timer without running callback.
         */
        void Stop();
    
    public:
        friend void TimerTask(void *pvParams);

    private:
        std::string m_name;
        TimerCallback m_callback;
        TickType_t m_timeout_ms;
        TaskHandle_t m_taskHandle = nullptr;
        bool m_taskHandleValid = false;
    };
} // namespace Timer
