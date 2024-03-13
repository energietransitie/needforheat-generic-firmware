#include <util/mutex.hpp>

#include <esp_log.h>

constexpr const char *TAG = "Mutex";

namespace Mutex
{
    Mtx::Mtx()
        : m_semaphoreHandle(xSemaphoreCreateMutex()) {}

    Mtx::~Mtx()
    {
        if (xSemaphoreTake(m_semaphoreHandle, portMAX_DELAY) == pdTRUE)
            vSemaphoreDelete(m_semaphoreHandle);
    }

    void Mtx::Take()
    {
        ESP_LOGD(TAG, "Trying to take mutex");
        xSemaphoreTake(m_semaphoreHandle, portMAX_DELAY);
        ESP_LOGD(TAG, "Took mutex successfully");
    }

    bool Mtx::Take(TickType_t xBlockTime)
    {
        ESP_LOGD(TAG, "Trying to take mutex");
        auto success = xSemaphoreTake(m_semaphoreHandle, xBlockTime) == pdTRUE;
        
        if (success)
        {
            ESP_LOGD(TAG, "Took mutex successfully");
        }
        else
        {
            ESP_LOGD(TAG, "Unable to take mutex");
        }

        return success;
    }

    void Mtx::Give()
    {
        xSemaphoreGive(m_semaphoreHandle);
        ESP_LOGD(TAG, "Released mutex");
    }

    ScopedLock::ScopedLock(Mtx &mtx)
        : m_mtx(mtx)
    {
        m_mtx.Take();
    }

    ScopedLock::ScopedLock(Mtx &mtx, TickType_t xBlockTime)
        : m_mtx(mtx)
    {
        m_mtxWasTaken = m_mtx.Take(xBlockTime);
    }

    ScopedLock::~ScopedLock()
    {
        m_mtx.Give();
    }
} // namespace Mutex
