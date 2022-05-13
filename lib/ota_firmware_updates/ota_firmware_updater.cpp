#include "ota_firmware_updater.h"

#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <regex>
#include <cstdio>

// This is necessary for define statements from generic_esp_32 which is C code.
extern "C"
{
#include <generic_esp_32.h>
#include <scheduler.h>
}

#include <http_util.h>
#include <semantic_version.h>
#include <error.h>
#include <delay.h>
#include <measurements.h>
#include <nvs.h>

#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_ota_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace OTAFirmwareUpdater
{
    namespace
    {
        const char *TAG = "OTA Firmware Updater";

        void LogFirmwareToBackend(const std::string propertyName, const std::string &version)
        {
            std::string url = TWOMES_SERVER + std::string(VARIABLE_UPLOAD_ENDPOINT);

            esp_http_client_config_t config{};
            config.url = url.c_str();
            config.cert_pem = isrg_root_pem_start;
            config.method = HTTP_METHOD_POST;
            config.transport_type = HTTP_TRANSPORT_OVER_SSL;
            config.is_async = false;

            HTTPUtil::headers_t headersSend;
            headersSend["user-agent"] = "ota_firmware_updater";
            headersSend["accept"] = "*/*";
            headersSend["Authorization"] = "Bearer " + std::string(get_bearer());
            headersSend["Content-Type"] = "application/json";

            HTTPUtil::buffer_t dataSend = Measurements::CreateRequestBodyVariable(propertyName, Measurements::MeasurementValue(version));

            ESP_LOGD(TAG, "Sending meta measurement value to backend with length: %d, data:\n%s", dataSend.size(), dataSend.c_str());

            HTTPUtil::headers_t headersReceive;
            HTTPUtil::buffer_t dataReceive;

            auto statusCode = HTTPUtil::HTTPRequest(config, headersReceive, dataReceive, headersSend, dataSend);
            ESP_LOGI(TAG, "Status code: %d, Response:\n%s", statusCode, dataReceive.data());
        }

        update_available_t FindUpdates()
        {
            ESP_LOGI(TAG, "Checking for available updates.");

            esp_http_client_config_t config{};
            config.url = UPDATE_CHECK_URL;
            config.cert_pem = github_root_pem_start;
            config.transport_type = HTTP_TRANSPORT_OVER_SSL;
            config.is_async = false;

            HTTPUtil::headers_t headers;
            headers["user-agent"] = USER_AGENT_VALUE;
            headers["accept"] = "*/*";

            HTTPUtil::buffer_t response;

            auto statusCode = HTTPUtil::HTTPRequest(config, headers, response);
            if (statusCode != 200)
            {
                ESP_LOGE(TAG, "Status code of request was not 200 but %d", statusCode);

                if (statusCode == 404)
                {
                    ESP_LOGE(TAG, "Status code was 404. This can mean there were no releases found, or there was another error. Pre-releases can not be found.");
                }

                return std::make_pair(false, "");
            }

            ESP_LOGD(TAG, "Response when checking for new releases:\n%s", response.data());

            std::regex regexString("\"tag_name\":\"([^\"]+)\"");

            std::string responseString(response.begin(), response.end());

            std::smatch matches;
            auto matchFound = std::regex_search(responseString, matches, regexString);
            if (!matchFound)
            {
                ESP_LOGE(TAG, "No version number found in response.");
                return std::make_pair(false, "");
            }

            ESP_LOGI(TAG, "Latest release on GitHub: %s", matches.str(1).c_str());

            auto currentDescription = esp_ota_get_app_description();

            SemanticVersion currentVersion(currentDescription->version);
            SemanticVersion foundVersion(matches.str(1));

            if (foundVersion > currentVersion)
            {
                return std::make_pair(true, foundVersion.String());
            }

            return std::make_pair(false, foundVersion.String());
        }

        void InstallUpdate(const std::string &downloadURL)
        {
            esp_err_t err = ESP_OK;

            esp_http_client_config_t config{};
            config.url = downloadURL.c_str();
            config.cert_pem = github_root_pem_start;
            config.timeout_ms = OTA_RECEIVE_TIMEOUT_MS;
            config.transport_type = HTTP_TRANSPORT_OVER_SSL;
            config.buffer_size = 2048;
            config.buffer_size_tx = 1024;
            config.is_async = false;

            char *endpoint = const_cast<char *>("OTAFirmwareUpdater::InstallUpdate");

            wait_for_wifi(endpoint);
            err = esp_https_ota(&config);
            disconnect_wifi(endpoint);
            if (Error::CheckAppendName(err, TAG, "An error occured while performing HTTP OTA-update"))
                return;

            ESP_LOGI(TAG, "OTA firmware update was successful. New firmware will be booted after reboot.");
        }
    } // namespace

    void Start()
    {
        BaseType_t status = xTaskCreatePinnedToCore(OTAFirmwareUpdaterTask, "ota_firmware_updater_task", TASK_STACK_DEPTH, nullptr, 1, nullptr, APP_CPU_NUM);
        if (status != pdPASS)
        {
            ESP_LOGE(TAG, "The OTA Firmware Updater Task failed to start with error code: %d", status);
        }
    }

    void OTAFirmwareUpdaterTask(void *pvParams)
    {
        ESP_LOGI(TAG, "Task started.");

        Check();

        // Signal that the task is done.
        xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(pvParams));
        vTaskDelete(NULL);
    }

    void Check()
    {
        bool updateFound;
        std::string version;
        std::tie(updateFound, version) = FindUpdates();

        if (!updateFound)
            return;

        std::string storedNewFirmware;
        auto err = NVS::Get("twomes_storage", "new_fw", storedNewFirmware);
        if (err == ESP_OK)
        {
            SemanticVersion storedNew(storedNewFirmware);
            SemanticVersion foundNew(version);

            if (storedNew == foundNew)
            {
                // This version was found before.
                int installCount = 12;
                NVS::Get("twomes_storage", "install_count", installCount);
                if (installCount >= MAX_INSTALL_TRIES)
                {
                    ESP_LOGE(TAG, "Installation of firmware update %s was already tried %d times. Skipping install.", version.c_str(), installCount);
                    return;
                }
                ESP_LOGD(TAG, "Installation of version %s was already tried %d times.", version.c_str(), installCount);
            }
            else
            {
                // Reset install_count.
                err = NVS::Set("twomes_storage", "install_count", 0);
                Error::CheckAppendName(err, TAG, "An error occured when resetting install_count");
            }
        }
        else
        {
            ESP_LOGD(TAG, "Key \"new_fw\" was not set in NVS yet.");
        }

        err = NVS::Set("twomes_storage", "new_fw", version);
        Error::CheckAppendName(err, TAG, "An error occured when writing new_fm in NVS");

        err = NVS::Increment<int32_t>("twomes_storage", "install_count");
        Error::CheckAppendName(err, TAG, "An error occured when incrementing install_count in NVS");

        ESP_LOGI(TAG, "Newer firmware version was found: %s", version.c_str());

        LogFirmwareToBackend("new_fw", version);

        // Insert version number into the URL.
        auto downloadURL = std::regex_replace(UPDATE_DOWNLOAD_URL, std::regex("%s"), version);

        InstallUpdate(downloadURL);
    }

    void CheckUpdateFinishedSuccessfully()
    {
        esp_err_t err;

        auto partition = esp_ota_get_running_partition();
        esp_ota_img_states_t state;

        err = esp_ota_get_state_partition(partition, &state);
        if (Error::CheckAppendName(err, TAG, "An error occured when checking the state of the running parition"))
            return;

        if (state == ESP_OTA_IMG_VALID)
        {
            ESP_LOGD(TAG, "OTA partition is valid. This partition is not pending verification after an OTA firmware update.");

            std::string storedNewFirmware;
            err = NVS::Get("twomes_storage", "new_fw", storedNewFirmware);
            if (Error::CheckAppendName(err, TAG, "An error occured when reading new_fw from NVS"))
                return;

            SemanticVersion storedNew(storedNewFirmware);
            std::string currentVersionString = esp_ota_get_app_description()->version;
            SemanticVersion currentVersion(currentVersionString);

            if (storedNew > currentVersion) // Installation of newer version was attempted, but failed.
            {
                ESP_LOGW(TAG, "It looks like a previously attempted update installation failed.");
                // TODO: Check why function call below creates stack overflow on task main.
                // Calling this function somewhere else works fine, but not here.
                LogFirmwareToBackend("booted_fw", currentVersionString);
            }

            return;
        }
        else if (state == ESP_OTA_IMG_UNDEFINED)
        {
            ESP_LOGW(TAG, "Rollback is not enabled. Skipping check.");
            return;
        }

        ESP_LOGD(TAG, "OTA partition is pending verification.");

        auto appDescription = esp_ota_get_app_description();
        LogFirmwareToBackend("booted_fm", appDescription->version);

        // TODO: Other checks can be added here, but if this function is called after any other tasks have started,
        // this point will not have been reached if one of those tasks crashed the ESP32.

        // If everything works call esp_ota_mark_app_valid_cancel_rollback().
        // If not, call esp_ota_mark_app_invalid_rollback_and_reboot().

        err = esp_ota_mark_app_valid_cancel_rollback();
        if (Error::CheckAppendName(err, TAG, "An error occured when cancelling rollback"))
        {
            // If marking valid fails, mark invalid.
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }
    }
} // namespace OTAFirmwareUpdater
