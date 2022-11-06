#include <specific_m5coreink/power_off_timeout.hpp>

#include <esp_log.h>

#include <specific_m5coreink/powerpin.h>
#include <util/delay.hpp>
#include <util/screen.hpp>

constexpr const char *TAG = "PowerOffTimeout";

namespace M5CoreInkSpecific
{
    namespace
    {
        void TimoutFunction(void *pvParams)
        {
            auto timeout = static_cast<int *>(pvParams);

            vTaskDelay(Delay::Seconds(*timeout));

            Screen screen;
            screen.Clear();

            // Allow some time for the screen to clear properly.
            vTaskDelay(Delay::MilliSeconds(300));

            // Power off the M5CoreINK.
            powerpin_reset();

            vTaskDelete(nullptr);
        }
    } // namespace

    PowerOffTimeout::PowerOffTimeout(int timeout_s)
        : m_timeout_s(timeout_s) {}

    void PowerOffTimeout::Start()
    {
        auto err = xTaskCreatePinnedToCore(TimoutFunction,
                                           "PowerOffTimeout",
                                           4096,
                                           static_cast<void *>(&m_timeout_s),
                                           1,
                                           &m_taskHandle,
                                           APP_CPU_NUM);

        if (err != pdPASS)
        {
            ESP_LOGE(TAG, "PowerOffTimeout task failed to start.");
        }
    }

    void PowerOffTimeout::Cancel()
    {
        if (m_taskHandle == nullptr)
        {
            return;
        }

        vTaskDelete(m_taskHandle);
    }
} // namespace M5CoreInkSpecific
