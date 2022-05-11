#ifndef C_OTA_H
#define C_OTA_H

// This is an adapter so this C++ code can be called by C code.

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /**
     * Start the OTAFirmwareUpdater task.
     *
     * @see OTAFirmwareUpdater::Start().
     */
    void twomes_ota_start();

    /**
     * Check for new firmware versions and install if found.
     *
     * @see OTAFirmwareUpdater::Check().
     */
    void twomes_ota_check();

    /**
     * Check if the booted firmware needs to be veriefied or rolled back.
     *
     * @see OTAFirmwareUpdater::CheckUpdateFinishedSuccessfully().
     */
    void twomes_ota_check_update_finished_successfully();

    /**
     * OTA Firmware Updater task.
     *
     * @see OTAFirmwareUpdater::OTAFirmwareUpdaterTask(void *pvParams).
     */
    void twomes_ota_firmware_update_task(void *pvParams);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // C_OTA_H
