#include "platform_esp32_dev.hpp"

#include <ctime>

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <delay.hpp>

constexpr const char *TAG = "Scheduler";

namespace ESP32Dev
{
    void WaitUntilNextTaskTime(time_t nextTaskTime_s)
    {
        // Calculate remaining time until nextTaskTime_s.
        time_t inactiveTime_s = nextTaskTime_s - time(nullptr);

        if (inactiveTime_s <= 0)
        {
            // We already passed the next task time. Continue immediately.
            return;
        }

        ESP_LOGD(TAG, "Delaying for %li seconds", inactiveTime_s);

        // Delay until the next task time.
        auto lastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&lastWakeTime, Delay::Seconds(inactiveTime_s));
    }
} // namespace ESP32Dev
