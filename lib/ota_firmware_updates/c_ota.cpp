#include "c_ota.h"
#include "ota_firmware_updater.h"

// This is an adapter so this C++ code can be called by C code.

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    
    void twomes_ota_start()
    {
        OTAFirmwareUpdater::Start();
    }

    void twomes_ota_firmware_update_task(void *pvParams)
    {
        OTAFirmwareUpdater::OTAFirmwareUpdaterTask(pvParams);
    }

    void twomes_ota_check()
    {
        OTAFirmwareUpdater::Check();
    }

    void twomes_ota_check_update_finished_successfully()
    {
        OTAFirmwareUpdater::CheckUpdateFinishedSuccessfully();
    }

#ifdef __cplusplus
}
#endif // __cplusplus
