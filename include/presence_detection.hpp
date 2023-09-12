#pragma once

#include <string>

#include <esp_bt_defs.h>
#include <mac_address.hpp>

namespace PresenceDetection
{
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
} // namespace PresenceDetection
