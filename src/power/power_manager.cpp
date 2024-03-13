#include <power_manager.hpp>

#include <esp_log.h>

#include <specific_m5coreink/powerpin.h>

constexpr const char *TAG = "PowerManager";

PowerManager &PowerManager::GetInstance()
{
    static PowerManager pm;
    return pm;
}

void PowerManager::AddPowerOffHook(const std::string &name, HookFunc func)
{
    ESP_LOGD(TAG, "Adding power off hook \"%s\"", name.c_str());

    Hook hook{name, func};

    Mutex::ScopedLock lock(m_powerOffHooksWriteMutex);
    m_powerOffHooks.push_back(hook);
}

void PowerManager::PowerOff()
{
    RunAllHooks();

    ESP_LOGI(TAG, "Powering off");

    fflush(stdout);
#if defined ESP32_DEV
    ESP_LOGI(TAG, "Power off requested for ESP32DEV, but platform does not support power off");
#elif defined M5STACK_COREINK
    powerpin_reset();
#endif // ESP32_DEV or M5STACK_COREINK
}

void PowerManager::Restart()
{
    RunAllHooks();

    ESP_LOGI(TAG, "Restarting");

    fflush(stdout);
    esp_restart();
}

void PowerManager::RunAllHooks()
{
    for (const auto &hook : m_powerOffHooks)
    {
        ESP_LOGD(TAG, "Running power off hook \"%s\"", hook.name.c_str());
        hook.func();
    }
}
