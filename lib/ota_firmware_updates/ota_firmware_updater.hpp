#ifndef OTA_FIRMWARE_UPDATER_HPP
#define OTA_FIRMWARE_UPDATER_HPP

#include <string>

namespace OTAFirmwareUpdater
{
    using update_available_t = std::pair<bool, std::string>;

    /**
     * Set the update check and download location.
     * 
     * @param org GitHub organisation name.
     * @param repo GitHub repository name.
     * @param fileName Name of the GitHub release asset to download.
     */
    void SetLocation(const char *org, const char *repo, const char *fileName);

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

#endif // OTA_FIRMWARE_UPDATER_HPP
