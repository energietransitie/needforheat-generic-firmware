#include <generic_esp_32.hpp>

#include <esp_log.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <wifi_provisioning/manager.h>
#include <driver/gpio.h>
#include <esp_sntp.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#include <cJSON.h>

#include <util/error.hpp>
#include <util/nvs.hpp>
#include <util/format.hpp>
#include <util/delay.hpp>
#include <util/buttons.hpp>
#include <util/buzzer.hpp>
#include <util/screen.hpp>
#include <specific_m5coreink/powerpin.h>
#include <specific_m5coreink/rtc.h>
#include <scheduler.hpp>
#include <generic_tasks.hpp>
#include <measurements.hpp>
#include <secure_upload.hpp>
#include <ota_firmware_updater.hpp>

#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
#include <wifi_provisioning/scheme_ble.h>
#endif // CONFIG_EXAMPLE_PROV_TRANSPORT_BLE

#ifdef ESP32DEV
#include "platform_esp32_dev.hpp"
#endif // ESP32DEV
#if M5STACK_COREINK
#include "platform_m5stack_coreink.hpp"
#include <specific_m5coreink/power_off_timeout.hpp>
#endif // M5STACK_COREINK

#define WIFI_CONNECTED_EVENT BIT0
#define WIFI_DISCONNECTED_EVENT BIT1

#ifdef CONFIG_TWOMES_TEST_SERVER
constexpr const char *TWOMES_SERVER_HOSTNAME = "api.tst.energietransitiewindesheim.nl";
constexpr const char *TWOMES_SERVER = "https://api.tst.energietransitiewindesheim.nl";
#endif

#ifdef CONFIG_TWOMES_PRODUCTION_SERVER
constexpr const char *TWOMES_SERVER_HOSTNAME = "api.energietransitiewindesheim.nl";
constexpr const char *TWOMES_SERVER = "https://api.energietransitiewindesheim.nl";
#endif

constexpr int WIFI_MAX_CONNECT_TRIES = 10;
constexpr int WIFI_MAX_WAIT_MS = 30 * 1000; // 30 seconds.

constexpr int NTP_MAX_SYNC_TRIES = 10;
constexpr int NTP_MAX_WAIT_MS = 2 * 1000; // 2 seconds.

constexpr const char *TAG = "Twomes generic ESP32 firmware library";
constexpr const char *NVS_NAMESPACE = "twomes_storage";
constexpr const char *DEVICE_SERVICE_NAME_PREFIX = "TWOMES-";
constexpr const char *QR_CODE_PAYLOAD_TEMPLATE = "{\n\"ver\":\"v1\",\n\"name\":\"%s\",\n\"pop\":\"%u\",\n\"transport\":\"ble\"\n}";
constexpr const char *POST_DEVICE_PAYLOAD_TEMPLATE = "{\n\"name\":\"%s\",\n\"device_type\":\"%s\",\n\"activation_token\":\"%u\"\n}";
constexpr const char *ACTIVATION_POST_REQUEST_TEMPLATE = "{\"name\":\"%s\"}";
constexpr const char *POST_PROVISIONING_INFO_LINK = "https://edu.nl/4pujw";
constexpr const char *POST_PROVISIONING_INFO_TEXT = "Scan voor info";

constexpr int LONG_BUTTON_PRESS_DURATION = 10 * 2; // Number of half seconds to wait: (10 s * 2 halfseconds)

constexpr int PRE_PROVISIONING_POWER_OFF_TIMEOUT_S = 15 * 60; // 15 minutes.

// Screen and QR definitions
constexpr int SCREEN_WIDTH = 200;
constexpr int SCREEN_HEIGHT = 200;
constexpr int QR_PADDING = 16;

namespace GenericESP32Firmware
{
    namespace
    {
        static std::string s_deviceTypeName;

        static EventGroupHandle_t s_wifiEventGroup;
        static bool s_wifiInitialized = false;
        static int s_wifiConnectRetries = 0;

        static bool s_postProvisioningNeeded = false;

#ifdef M5STACK_COREINK
        static Screen s_screen;

        static M5CoreInkSpecific::PowerOffTimeout s_powerOffTimeout(PRE_PROVISIONING_POWER_OFF_TIMEOUT_S);
#endif // M5STACK_COREINK

        /**
         * Blink an LED on the device.
         *
         * @param gpioNum GPIO number where LED is connected.
         * @param amount Amount of times to flash the LED.
         */
        void BlinkLED(gpio_num_t gpioNum, int amount)
        {
            auto level = gpio_get_level(gpioNum);

            for (int i = 0; i < amount * 2; i++)
            {
                // Flip level
                level ^= 1;

                gpio_set_level(gpioNum, level);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }

#ifdef M5STACK_COREINK
        void PowerOff()
        {
            // Buzz the buzzer 500ms to signal power off.
            Buzzer::Buzz(500);

            s_screen.Clear();

            powerpin_reset();
        }
#endif // M5STACK_COREINK

        /**
         * Reset wireless settings and delete the bearer to force re-activation.
         */
        void ResetWireless()
        {
            BlinkLED(LED_WIFI_RESET, 5);

            auto err = NVS::Erase(NVS_NAMESPACE, "bearer");
            Error::CheckAppendName(err, TAG, "An error occured when erasing bearer");

            esp_wifi_restore();
            esp_restart();
        }

        /**
         * Event handler for WiFi and provisioning events.
         */
        void WifiEventHandler(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData)
        {
            if (eventBase == WIFI_EVENT)
            {
                switch (eventID)
                {
                case WIFI_EVENT_STA_START:
                    esp_wifi_start();
                    esp_wifi_connect();
                    break;
                case WIFI_EVENT_STA_DISCONNECTED:
                    if (s_wifiConnectRetries < WIFI_MAX_CONNECT_TRIES)
                    {
                        ESP_LOGI(TAG, "Connect to AP retry.");
                        esp_wifi_connect();
                        s_wifiConnectRetries++;
                    }
                    else
                    {
                        ESP_LOGD(TAG, "Disconnected.");
                        xEventGroupSetBits(s_wifiEventGroup, WIFI_DISCONNECTED_EVENT);
                    }
                    break;
                default:
                    break;
                }
            }
            else if (eventBase == IP_EVENT)
            {
                switch (eventID)
                {
                case IP_EVENT_STA_GOT_IP:
                    ESP_LOGD(TAG, "Connected with IP Address:" IPSTR,
                             IP2STR(&((ip_event_got_ip_t *)eventData)->ip_info.ip));
                    xEventGroupSetBits(s_wifiEventGroup, WIFI_CONNECTED_EVENT);
                    s_wifiConnectRetries = 0;
                    break;
                default:
                    break;
                }
            }
            else if (eventBase == WIFI_PROV_EVENT)
            {
                switch (eventID)
                {
                case WIFI_PROV_START:
                    ESP_LOGD(TAG, "Provisioning started");
                    break;
                case WIFI_PROV_CRED_RECV:
                    ESP_LOGD(TAG, "Received Wi-Fi credentials");
                    break;
                case WIFI_PROV_CRED_FAIL:
                    ESP_LOGE(TAG,
                             "Provisioning failed!\n\tReason : %s"
                             "\n\tRestarting in 5 seconds and try again...",
                             (*(wifi_prov_sta_fail_reason_t *)eventData == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                    vTaskDelay(Delay::Seconds(5));
                    ResetWireless();
                    break;
                case WIFI_PROV_CRED_SUCCESS:
                    ESP_LOGD(TAG, "Provisioning successful");
                    break;
                case WIFI_PROV_END:
                    // De-initialize manager once provisioning is finished.
                    wifi_prov_mgr_deinit();
                    break;
                default:
                    break;
                }
            }
        }

        /**
         * Get dat from NVS.
         * If the dat does not exist, it will create it.
         *
         * @returns dat.
         */
        uint32_t GetDat()
        {
            uint32_t dat;
            auto err = NVS::Get(NVS_NAMESPACE, "dat", dat);
            if (err == ESP_OK)
            {
                // dat was found.
                return dat;
            }

            if (err != ESP_ERR_NVS_NOT_FOUND)
            {
                // Something went wrong besides the key not existing.
                // Log it.
                Error::Check(err, TAG);
                return dat;
            }

            // If we get here, the key does not exist.
            // Create a new dat.

            ESP_LOGD(TAG, "'dat' was not found in NVS. Creating it now.");

            dat = esp_random();
            err = NVS::Set(NVS_NAMESPACE, "dat", dat);
            Error::Check(err, TAG);

            return dat;
        }

        /**
         * Get bearer from NVS.
         *
         * @returns bearer.
         */
        std::string GetBearer()
        {
            std::string bearer;
            auto err = NVS::Get(NVS_NAMESPACE, "bearer", bearer);
            if (err == ESP_ERR_NVS_NOT_FOUND)
            {
                // The key does not exist.
                ESP_LOGD(TAG, "The bearer has not been initialized in NVS yet.");
                return bearer;
            }

            if (err != ESP_OK)
            {
                // Something went wrong besides the key not existing.
                Error::Check(err, TAG);
                return bearer;
            }

            return bearer;
        }

        /**
         * Set bearer in NVS.
         *
         * @returns bearer.
         */
        void SetBearer(const std::string &bearer)
        {
            auto err = NVS::Set(NVS_NAMESPACE, "bearer", bearer);
            Error::CheckAppendName(err, TAG, "An error occured when setting bearer");
        }

        /**
         * Initialize wireless (WiFi).
         * This function does not turn the wireless radio on.
         *
         * @returns ESP error.
         */
        esp_err_t InitializeWireless()
        {
            ESP_LOGD(TAG, "Initializing wireless.");

            s_wifiEventGroup = xEventGroupCreate();

            auto err = esp_netif_init();
            if (Error::CheckAppendName(err, TAG, "An error occured when initializing netif"))
                return err;

            err = esp_event_loop_create_default();
            if (Error::CheckAppendName(err, TAG, "An error occured when creating event loop"))
                return err;

            err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiEventHandler, nullptr);
            if (Error::CheckAppendName(err, TAG, "An error occured when registering WIFI_EVENT handler"))
                return err;
            err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiEventHandler, nullptr);
            if (Error::CheckAppendName(err, TAG, "An error occured when registering IP_EVENT handler"))
                return err;
            err = esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &WifiEventHandler, nullptr);
            if (Error::CheckAppendName(err, TAG, "An error occured when registering WIFI_PROV_EVENT handler"))
                return err;

            // This function does not return an error.
            esp_netif_create_default_wifi_sta();

#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
            esp_netif_create_default_wifi_ap();
#endif // CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP

            wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
            err = esp_wifi_init(&wifiInitConfig);
            if (Error::CheckAppendName(err, TAG, "An error occured when initializing wifi"))
                return err;

            err = esp_wifi_set_mode(WIFI_MODE_STA);
            if (Error::CheckAppendName(err, TAG, "An error occured when setting wifi mode"))
                return err;

            return ESP_OK;
        }

        /**
         * Initialize GPIO based on defined platform.
         * Supported platforms:
         *   - ESP32DEV
         *   - M5STACK_COREINK
         *
         * @returns ESP error.
         */
        esp_err_t InitializeGPIO()
        {
#ifndef CONFIG_TWOMES_CUSTOM_GPIO
            ESP_LOGD(TAG, "Initializing GPIO.");

#ifdef ESP32DEV
            auto err = ESP32Dev::InitializeGPIO();
#endif // ESP32DEV

#ifdef M5STACK_COREINK
            auto err = M5StackCoreInk::InitializeGPIO();

            err = Buttons::ButtonPressHandler::AddButton(BUTTON_ON_OFF, "On-off button", 0, nullptr, PowerOff);
            if (Error::CheckAppendName(err, TAG, "An error occured when adding BUTTON_WIFI_RESET to handler"))
                return err;
#endif // M5STACK_COREINK
            if (Error::CheckAppendName(err, TAG, "An error occured when initializing GPIO"))
                return err;

            err = Buttons::ButtonPressHandler::AddButton(BUTTON_WIFI_RESET, "Wi-Fi reset", 0, nullptr, ResetWireless);
            if (Error::CheckAppendName(err, TAG, "An error occured when adding BUTTON_WIFI_RESET to handler"))
                return err;
#endif // CONFIG_TWOMES_CUSTOM_GPIO

            return ESP_OK;
        }

        /**
         * Initialize provisioning based on defined configuration.
         *
         * @returns ESP error.
         */
        esp_err_t InitializeProvisioning()
        {
            ESP_LOGD(TAG, "Initializing provisioning.");

            wifi_prov_mgr_config_t wifiProvConfig;
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
            wifiProvConfig.scheme = wifi_prov_scheme_ble;
#endif // CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
            wifiProvConfig.scheme = wifi_prov_scheme_softap;
#endif // CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
            wifiProvConfig.scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE;
            wifiProvConfig.app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE;

            auto err = wifi_prov_mgr_init(wifiProvConfig);
            if (Error::Check(err, TAG))
                return err;

            auto dat = GetDat();

            // Log the QR code.
            auto deviceServiceName = GetDeviceServiceName();
            auto qrCodePayload = Format::String(QR_CODE_PAYLOAD_TEMPLATE, deviceServiceName.c_str(), dat);
            ESP_LOGI(TAG,
                     "QR Code Payload: \n"
                     "\n\n%s\n\n",
                     qrCodePayload.c_str());

            return ESP_OK;
        }

        /**
         * Initialize and set the timezone.
         */
        void InitializeTimezone(const char *timezone)
        {
            ESP_LOGD(TAG, "Initializing timezone.");

            setenv("TZ", timezone, 0);
            tzset();
        }

        /**
         * Callback that is called when a timesync event happens.
         */
        void TimeSyncNotificationCallback(struct timeval *tv)
        {
            ESP_LOGI(TAG, "Time sync event notification received. The current time is: %s", ctime(&tv->tv_sec));

#ifdef M5STACK_COREINK
            // Update the RTC time.
            rtc_syncronize_rtc_time();
#endif // M5STACK_COREINK
        }

        /**
         * This function will run only when the device is just provisioned.
         */
        void PostProvisioning()
        {
            // Immediately sync time.
            InitializeTimeSync();

            // Log the booted firmware version to the backend.
            auto appDescription = esp_ota_get_app_description();
            OTAFirmwareUpdater::LogFirmwareToBackend("booted_fw", appDescription->version);

            ESP_LOGD(TAG, "Sending a heartbeat once.");
            // Add a measurement formatter for the heartbeat property.
            Measurements::Measurement::AddFormatter("heartbeat", "%d");

            Measurements::Measurement measurement("heartbeat", 0, time(nullptr));
            SecureUpload::Queue::GetInstance().AddMeasurement(measurement);
            GenericTasks::UploadTask(nullptr);

            // Run all tasks in the scheduler once.
            Scheduler::RunAll();
        }

        /**
         * Start Wi-Fi.
         *
         * @returns ESP error.
         */
        esp_err_t StartWireless()
        {
            auto err = esp_wifi_start();
            Error::CheckAppendName(err, TAG, "An error occured when starting Wi-Fi.");

            if (s_wifiInitialized)
                return ESP_OK;

            for (int tries = 1; tries <= WIFI_MAX_CONNECT_TRIES; tries++)
            {
                auto bits = xEventGroupWaitBits(s_wifiEventGroup,
                                                WIFI_CONNECTED_EVENT,
                                                true,
                                                true,
                                                Delay::MilliSeconds(WIFI_MAX_WAIT_MS));
                // If Wi-Fi is connected, we are done.
                if (bits & WIFI_CONNECTED_EVENT)
                    break;

                ESP_LOGD(TAG,
                         "Failed to connect to Wi-Fi (%d/%d) at %s",
                         tries,
                         WIFI_MAX_CONNECT_TRIES,
                         esp_log_system_timestamp());

                if (tries == WIFI_MAX_CONNECT_TRIES)
                {
                    // Restart device after maximum tries.

                    ESP_LOGD(TAG, "Minimum free heap size: %d bytes", esp_get_minimum_free_heap_size());

                    ESP_LOGE(TAG, "Could still not get connection; restarting now.");

                    fflush(stdout);
                    esp_restart();
                }
            }

            return err;
        }

        /**
         * Start provisioning based on defined configuration.
         *
         * @returns ESP error.
         */
        esp_err_t StartProvisioning()
        {
            ESP_LOGD(TAG, "Starting provisioning.");

            // Prov manager was initialized in InitializeProvisioning().

            bool isProvisioned = false;
            auto err = wifi_prov_mgr_is_provisioned(&isProvisioned);
            if (Error::Check(err, TAG))
                return err;

            if (isProvisioned)
            {
                // Manager is no longer needed. De-initialize it.
                wifi_prov_mgr_deinit();

                ESP_LOGD(TAG, "Device is already provisioned.");

                return ESP_OK;
            }

#if M5STACK_COREINK
            auto qrCodePayload = Format::String(QR_CODE_PAYLOAD_TEMPLATE, GetDeviceServiceName().c_str(), GetDat());
            s_screen.DisplayQR(qrCodePayload, QR_PADDING);

            s_powerOffTimeout.Start();
#endif // M5STACK_COREINK

#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
            /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
             * set a custom 128 bit UUID which will be included in the BLE advertisement
             * and will correspond to the primary GATT service that provides provisioning
             * endpoints as GATT characteristics. Each GATT characteristic will be
             * formed using the primary service UUID as base, with different auto assigned
             * 12th and 13th bytes (assume counting starts from 0th byte). The client side
             * applications must identify the endpoints by reading the User Characteristic
             * Description descriptor (0x2901) for each characteristic, which contains the
             * endpoint name of the characteristic.
             * LSB <---> MSB */
            uint8_t serviceUUID[] = {0xb4, 0xdf, 0x5a, 0x1c,
                                     0x3f, 0x6b, 0xf4, 0xbf,
                                     0xea, 0x4a, 0x82, 0x03,
                                     0x04, 0x90, 0x1a, 0x02};
            err = wifi_prov_scheme_ble_set_service_uuid(serviceUUID);
            if (Error::Check(err, TAG))
                return err;
#endif // CONFIG_TWOMES_PROV_TRANSPORT_BLE

            wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
            auto datStr = std::to_string(GetDat());
            auto serviceName = GetDeviceServiceName();
            err = wifi_prov_mgr_start_provisioning(security, datStr.c_str(), serviceName.c_str(), datStr.c_str());
            if (Error::Check(err, TAG))
                return err;

            // Wait until Wi-Fi connection is made.
            xEventGroupWaitBits(s_wifiEventGroup, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);

            // WiFi was initialized during provisioning.
            s_wifiInitialized = true;

            // Signal that provisioning just happened.
            s_postProvisioningNeeded = true;

            return ESP_OK;
        }

        /**
         * Activate the device with the backend server.
         * If a bearer exists, this function returns immediately.
         */
        void ActivateDevice()
        {
            ESP_LOGD(TAG, "Activating device.");

            auto bearer = GetBearer();
            if (!bearer.empty())
            {
                // The bearer already exists.
                ESP_LOGD(TAG, "Existing bearer was found. Device is activated.");
                return;
            }

            HTTPUtil::buffer_t deviceActivationRequestData = Format::String(ACTIVATION_POST_REQUEST_TEMPLATE, GetDeviceServiceName().c_str());
            HTTPUtil::headers_t headersSend;
            headersSend["Authorization"] = Format::String("Bearer %s", std::to_string(GetDat()).c_str());

            HTTPUtil::buffer_t dataReceive;
            HTTPUtil::headers_t headersReceive;
            auto statusCode = PostHTTPSToBackend(ENDPOINT_DEVICE_ACTIVATION,
                                                 deviceActivationRequestData,
                                                 headersSend,
                                                 dataReceive,
                                                 headersReceive,
                                                 false);
            if (statusCode != 200)
            {
                // Statuscode was not 200 (OK). Something went wrong
                ESP_LOGE(TAG, "Statuscode %u was received while attempting device activation.", statusCode);
                return;
            }

            if (dataReceive.empty())
            {
                // No bearer was received as a response.
                ESP_LOGE(TAG, "No bearer was received while attempting device activation.");
                return;
            }

            ESP_LOGD(TAG, "HTTP response:\n%s", dataReceive.c_str());

            auto json = cJSON_Parse(dataReceive.c_str());
            if (json == nullptr)
            {
                ESP_LOGE(TAG, "An error occured when parsing JSON.");
            }

            auto jsonAuthorizationToken = cJSON_GetObjectItem(json, "authorization_token");
            if (json == nullptr)
            {
                ESP_LOGE(TAG, "An error occured when parsing JSON.");
            }

            auto authorizationToken = cJSON_GetStringValue(jsonAuthorizationToken);
            std::string authorizationTokenStr(authorizationToken);
            SetBearer(authorizationTokenStr.c_str());
            cJSON_Delete(json);
        }
    } // namespace

    void Initialize(const std::string &deviceTypeName)
    {
#ifdef M5STACK_COREINK
        // The first thing we need to do is enable battery power,
        // otherwise booting by the RTC signal will not succeed.
        powerpin_set();
#endif // M5STACK_COREINK

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
        OTAFirmwareUpdater::CheckUpdateFinishedSuccessfully();
#endif // CONFIG_TWOMES_OTA_FIRMWARE_UPDATE

        s_deviceTypeName = deviceTypeName;

        ESP_LOGD(TAG, "Initializing Generic ESP32 Firmware, Version: %s", esp_ota_get_app_description()->version);

        NVS::Initialize();

        InitializeTimezone("UTC");

#ifdef M5STACK_COREINK
        rtc_initialize();
        rtc_syncronize_sys_time();
#endif // M5STACK_COREINK

        auto err = InitializeWireless();
        Error::CheckAppendName(err, TAG, "An error occured inside GenericFirmware::<unnamed>::InitializeWireless()");

#ifndef CONFIG_TWOMES_CUSTOM_GPIO
        err = InitializeGPIO();
        Error::CheckAppendName(err, TAG, "An error occured inside GenericFirmware::<unnamed>::InitializeGPIO()");
#endif // CONFIG_TWOMES_CUSTOM_GPIO

        err = InitializeProvisioning();
        Error::CheckAppendName(err, TAG, "An error occured inside GenericFirmware::<unnamed>::InitializeProvisioning()");

        err = StartProvisioning();
        Error::CheckAppendName(err, TAG, "An error occured inside GenericFirmware::<unnamed>::StartProvisioning()");

#ifdef M5STACK_COREINK
        // Show information about what this device does on the screen.
        s_screen.DisplayQR(POST_PROVISIONING_INFO_LINK, QR_PADDING, POST_PROVISIONING_INFO_TEXT);

        // Cancel power off timeout since provisioning is done.
        s_powerOffTimeout.Cancel();
#endif // M5STACK_COREINK

        err = StartWireless();
        Error::CheckAppendName(err, TAG, "An error occured inside GenericFirmware::<unnamed>::StartWireless()");

#ifdef ESP32DEV
        // Sync time immediately because there is no RTC.
        InitializeTimeSync();
#endif

        ActivateDevice();

        if (s_postProvisioningNeeded)
            PostProvisioning();

        ESP_LOGI(TAG, "Finished initialization.");
    }

    std::string GetDeviceServiceName()
    {
        // TODO: hash device type + add mac.

        uint8_t eth_mac[6];
        auto err = esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
        Error::CheckAppendName(err, TAG, "An error occured when getting ETH MAC");

        return Format::String("%s%02X%02X%02X", DEVICE_SERVICE_NAME_PREFIX, eth_mac[3], eth_mac[4], eth_mac[5]);
    }

    void InitializeTimeSync()
    {
        ESP_LOGD(TAG, "Synchronizing time.\n");

        auto now = time(nullptr);
        ESP_LOGD(TAG, "Time before synchronization: %s", ctime(&now));

        if (sntp_enabled())
            return;

        ESP_LOGD(TAG, "Initializing SNTP.");

        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_set_time_sync_notification_cb(TimeSyncNotificationCallback);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
        sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
        sntp_init();

        for (int tries = 1; tries <= NTP_MAX_SYNC_TRIES; tries++)
        {
            if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
                break;

            ESP_LOGD(TAG, "Waiting for system time to be set... (%d/%d)",
                     tries,
                     NTP_MAX_SYNC_TRIES);

            vTaskDelay(Delay::MilliSeconds(NTP_MAX_WAIT_MS));
        }
    }

    int PostHTTPSToBackend(const std::string &endpoint,
                           HTTPUtil::buffer_t &dataSend,
                           HTTPUtil::headers_t &headersSend,
                           HTTPUtil::buffer_t &dataReceive,
                           HTTPUtil::headers_t &headersReceive,
                           bool useBearer = false)
    {
        std::string url = TWOMES_SERVER + endpoint;

        esp_http_client_config_t config{};
        config.url = url.c_str();
        config.cert_pem = isrg_root_pem_start;
        config.method = HTTP_METHOD_POST;
        config.transport_type = HTTP_TRANSPORT_OVER_SSL;
        config.is_async = false;

        headersSend["accept"] = "*/*";
        headersSend["Content-Type"] = "application/json";
        if (useBearer)
            headersSend["Authorization"] = "Bearer " + GetBearer();

        ESP_LOGD(TAG, "Sending data to backend with length: %d, data:\n%s", dataSend.size(), dataSend.c_str());

        ESP_LOGD(TAG, "Remaining heap space: %d", esp_get_free_heap_size());

        return HTTPUtil::HTTPRequest(config, headersReceive, dataReceive, headersSend, dataSend);
    }
} // namespace GenericESP32Firmware
