#include <util/timer.hpp>

#include <util/error.hpp>

constexpr const char *TAG = "Timer";

constexpr uint32_t TIMER_TASK_STACK_SIZE = 5096;
constexpr UBaseType_t TIMER_TASK_PRIORITY = 9;

namespace Timer
{
    // Task that 'runs' and blocks until the desired timeout and calls the callback.
    void TimerTask(void *pvParams)
    {
        auto timer = reinterpret_cast<Timer *>(pvParams);

        // This task will not run again until the specified timeout.
        vTaskDelay(timer->m_timeout_ms / portTICK_PERIOD_MS);

        ESP_LOGD(TAG, "Timer \"%s\" timed out", timer->m_name.c_str());

        timer->m_callback();

        // Delete this task immediately.
        timer->m_taskHandleValid = false;
        vTaskDelete(nullptr);
    }

    Timer::Timer(const std::string &name, TimerCallback callback, uint64_t timeout_ms)
        : m_name(name), m_callback(callback), m_timeout_ms(static_cast<TickType_t>(timeout_ms)) {}

    void Timer::StartOrReset()
    {
        if (m_taskHandleValid)
            vTaskDelete(m_taskHandle);

        xTaskCreatePinnedToCore(TimerTask,
                                m_name.c_str(),
                                TIMER_TASK_STACK_SIZE,
                                this,
                                TIMER_TASK_PRIORITY,
                                &m_taskHandle,
                                APP_CPU_NUM);
        
        m_taskHandleValid = true;

        ESP_LOGD(TAG, "Timer \"%s\" was (re)started", m_name.c_str());
    }

    void Timer::Stop()
    {
        if (m_taskHandleValid)
            vTaskDelete(m_taskHandle);

        ESP_LOGD(TAG, "Timer \"%s\" was stopped", m_name.c_str());
    }
} // namespace Timer
