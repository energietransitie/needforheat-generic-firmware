#include <string>
#include <utility>

#ifndef OTA_FIRMWARE_UPDATER_H
#define OTA_FIRMWARE_UPDATER_H

#define UPDATE_CHECK_URL "https://api.github.com/repos/energietransitie/twomes-generic-esp-firmware/releases/latest"
#define UPDATE_DOWNLOAD_URL "https://github.com/energietransitie/twomes-generic-esp-firmware/releases/download/%s/firmware.bin"

#define TASK_STACK_DEPTH 16384
#define UPDATE_CHECK_INTERVAL_MINUTES 5

#define MAX_INSTALL_TRIES 10
#define OTA_RECEIVE_TIMEOUT_MS 10000 // 10 seconds

namespace OTAFirmwareUpdater
{
    using update_available_t = std::pair<bool, std::string>;

    /**
     * Start the OTAFirmwareUpdater task.
     */
    void Start();

    /**
     * OTA Firmware Updater task.
     *
     * This is the FreeRTOS task that can be started.
     */
    void OTAFirmwareUpdaterTask(void *pvParams);

    /**
     * Check for new firmware versions and install if found.
     */
    void Check();

    /**
     * Check if the booted firmware needs to be veriefied or rolled back.
     */
    void CheckUpdateFinishedSuccessfully();
} // namespace OTAFirmwareUpdater

#endif // OTA_FIRMWARE_UPDATE_H
