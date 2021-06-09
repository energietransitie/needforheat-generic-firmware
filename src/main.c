#include <generic_esp_32.h>

#define DEVICE_TYPE_NAME "Generic-Test"
#define HEARTBEAT_UPLOAD_INTERVAL 3600000     //ms, so one hour
#define HEARTBEAT_MEASUREMENT_INTERVAL 600000 //ms, so 10 minutes; not yet in effect
static const char *TAG = "Twomes ESP32 generic test device";

const char *device_activation_url = TWOMES_TEST_SERVER "/device/activate";
const char *variable_interval_upload_url = TWOMES_TEST_SERVER "/device/measurements/variable-interval";

char *bearer;
const char *rootCA;

void app_main(void)
{
    initialize_nvs();
    initialize();
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_prov_mgr_config_t config = initialize_provisioning();

    //Make sure to have this here otherwise the device names won't match because
    //of config changes made by the above function call.
    prepare_device();
    //Starts provisioning if not provisioned, otherwise skips provisioning.
    //If set to false it will not autoconnect after provisioning.
    //If set to true it will autonnect.
    start_provisioning(config, true);
    //Initialize time with timezone UTC; building timezone is stored in central database
    initialize_time("UTC");

    //Gets time as epoch time.
    ESP_LOGI(TAG, "Getting time!");
    uint32_t now = time(NULL);
    ESP_LOGI(TAG, "Time is: %d", now);

    char *bearer = get_bearer();
    if (strlen(bearer) > 1)
    {
        ESP_LOGI(TAG, "Bearer read: %s", bearer);
    }
    else if (strcmp(bearer, "") == 0)
    {
        rootCA = get_root_ca();
        ESP_LOGI(TAG, "Bearer not found, activating device!");
        activate_device(device_activation_url, DEVICE_TYPE_NAME, rootCA);
        bearer = get_bearer();
    }
    else if (!bearer)
    {
        ESP_LOGE(TAG, "Something went wrong whilst reading the bearer!");
    }

    // Example Message Check generic_esp_32.c upload_hearbeat function to see a real example of this being filled.
    // char *msg_plain = "{\"upload_time\": \"%d\",\"property_measurements\":[    {"
    //                   "\"property_name\": %s,"
    //                   "\"measurements\": ["
    //                    "{ \"timestamp\":\"%d\","
    //                    "\"value\":\"1\"}"
    //                   "]}]}";

    /* Start main application now */
    while (1)
    {
        enable_wifi();
        //Wait to make sure Wi-Fi is enabled.
        vTaskDelay(500 / portTICK_PERIOD_MS);
        //Upload heartbeat
        upload_heartbeat(variable_interval_upload_url, rootCA, bearer);
        //Wait to make sure uploading is finished.
        vTaskDelay(500 / portTICK_PERIOD_MS);
        //Disconnect WiFi
        disable_wifi();
        //Wait HEARTBEAT_UPLOAD_INTERVAL(currently 1 hour)
        vTaskDelay(HEARTBEAT_UPLOAD_INTERVAL / portTICK_PERIOD_MS);
    }
}