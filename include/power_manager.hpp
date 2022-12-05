#pragma once

#include <functional>
#include <vector>
#include <string>

#include <util/mutex.hpp>

class PowerManager
{
public:
    // Functions used for power-off hooks.
    using HookFunc = std::function<void()>;

    /**
     * Hook stores the hook name and function.
     */
    struct Hook
    {
        std::string name;
        HookFunc func;
    };

    // This class is a singleton, so copy constructors
    // and assignment operators are deleted.
    PowerManager(const PowerManager &other) = delete;
    PowerManager &operator==(const PowerManager &other) = delete;

    /**
     * Get an instance of the PowerManager singleton.
     */
    static PowerManager &GetInstance();

public:
    /**
     * Add a power off hook.
     * All hooks will be called before powering off.
     */
    void AddPowerOffHook(const std::string &name, HookFunc func);

    /**
     * Power off the device (if possible).
     *
     * All power off hooks will be executed before calling the device-specific power off function.
     */
    void PowerOff();

    /**
     * Restart the device.
     *
     * All power off hooks will be called before restarting the device.
     */
    void Restart();

private:
    // This class is a singleton, so the constructor is private.
    PowerManager() = default;

    /**
     * Run all power off hooks sequintially.
     */
    void RunAllHooks();

private:
    // Storage for the poweroff hooks.
    std::vector<Hook> m_powerOffHooks;

    // Mutex to synchronize writing to m_powerOffHooks.
    Mutex::Mtx m_powerOffHooksWriteMutex;
};
