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

    /**
     * Log firmware version to needforheat server.
     * 
     * @param propertyName The name of the property. Can be "booted_fw" or "new_fw".
     * @param version The version as a string.
     */
    void LogFirmwareToBackend(const std::string propertyName, const std::string &version);
} // namespace OTAFirmwareUpdater

#endif // OTA_FIRMWARE_UPDATER_HPP
