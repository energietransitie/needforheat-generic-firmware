#pragma once

#include <string>

#include <esp_bt_defs.h>
#include <esp_err.h>
#include <mac_address.hpp>

namespace PresenceDetection
{
    /**
     * InitializeOptions define options that you can specify when calling InitializeBluetooth().
     */
    struct InitializeOptions
    {
        /**
         * Enable A2DP sink.
         * This should be enabled if you want an iPhone to be able to pair.
         */
        bool EnableA2DPSink;

        /**
         * Enable discoverable.
         * Make sure the device is discoverable.
         */
        bool EnableDiscoverable;
    };

    /**
     * Initialize bluetooth for presence detection.
     *
     * @param options InitializeOptions to use when initializing.
     *
     * @returns ESP error.
     */
    esp_err_t InitializeBluetooth(InitializeOptions options);

    /**
     * DenitializeOptions define options that you can specify when calling DeinitializeBluetooth().
     */
    struct DeinitializeOptions
    {
        /**
         * Disable A2DP sink.
         * This should have been enabled if you wanted an iPhone to be able to pair.
         */
        bool DisableA2DPSink;

        /**
         * Disable discoverable.
         * The device will no longer be discoverable.
         */
        bool DisableDiscoverable;

        /**
         * Disable Bluetooth;
         * The entire Bluetooth stack will be disabled.
         * Only use this if you no longer want to detect presence.
         */
        bool DisableBluetooth;
    };

    /**
     * Deinitialize bluetooth for presence detection.
     *
     * @returns ESP error.
     */
    esp_err_t DeinitializeBluetooth(DeinitializeOptions options);

    /**
     * UseBluetooth will initialize bluetooth with options you specify.
     * We keep track of a usage count. When there is no more usage, functions will be deinitialized.
     */
    class UseBluetooth
    {
    public:
        UseBluetooth() = delete;

        /**
         * Create a new UseBluetooth object.
         *
         * This object will make sure Bluetooth is enabled.
         * When this object is destroyed (goes out of scope),
         * Bluetooth will be released if not used elsewhere.
         *
         * @param options InitializeOptions to use when initializing.
         */
        UseBluetooth(InitializeOptions options);

        /**
         * Destructor for UseBluetooth object.
         *
         * This will decrement the usage counters.
         * If there are no more users for a counter, the function will be disabled.
         */
        ~UseBluetooth();

    private:
        // Keep track of how many BT users there are.
        static int s_useCount;
        // Keep track of how many users want A2DP to be enabled.
        static int s_a2dpCount;
        // Keep track how many users want the device to be discoverable.
        static int s_discoverableCount;
        // Keep track of the options used for this specific UseBluetooth object.
        InitializeOptions m_optionsUsed;
    };

    /**
     * Wait if Bluetooth is active.
     * 
     * If BT is active, this function will block.
     * Once BT is no longer active, it will return.
     * It will return immediately if BT is not active.
     */
    void WaitIfBluetoothActive();

    /**
     * Add a Bluetooth MAC-address to scan for presence detection.
     *
     * @param mac Bluetooth MAC-address as esp_bd_addr_t.
     */
    void AddMacAddress(const esp_bd_addr_t &mac);

    /**
     * Add a Bluetooth MAC-address to scan for presence detection.
     *
     * Takes in a string that looks like "AB:FF:0C:FC:BA:5C",
     * "AB-FF-0C-FC-BA-5C" or "ab:ff:0c:fc:ba:5c".
     *
     * @param mac Bluetooth MAC-address as a string.
     */
    void AddMacAddress(const std::string &mac);

    /**
     * Presence detection task.
     *
     * This task will scan known Bluetooth MAC-addresses
     * and report which ones responded.
     */
    void PresenceDetectionTask(void *taskInfo);

    /**
     * Return the device name.
     */
    std::string getDevName();
} // namespace PresenceDetection
