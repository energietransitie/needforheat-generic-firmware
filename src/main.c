#include <generic_esp_32.h>

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
const char*device_type_name = "Presence-Detector";
#else
const char*device_type_name = "Generic-Test";
#endif

static const char *TAG = "Twomes ESP32 generic test device";

char *bearer;
const char *rootCA;

void app_main(void)
{
    twomes_device_provisioning(device_type_name);

    ESP_LOGI(TAG, "Starting heartbeat task");
    xTaskCreatePinnedToCore(&heartbeat_task, "heartbeat_task", 4096, NULL, 1, NULL, 1);

    ESP_LOGI(TAG, "Waiting 5  seconds before initiating next measurement intervals");
    vTaskDelay(5000 / portTICK_PERIOD_MS); // wait 5 seconds before initiating next measurement intervals

    ESP_LOGI(TAG, "Starting timesync task");
    xTaskCreatePinnedToCore(&timesync_task, "timesync_task", 4096, NULL, 1, NULL, 1);

    ESP_LOGI(TAG, "Waiting 5  seconds before initiating next measurement intervals");
    vTaskDelay(5000 / portTICK_PERIOD_MS); // wait 5 seconds before initiating next measurement intervals
    #ifdef CONFIG_TWOMES_PRESENCE_DETECTION
    
    ESP_LOGI(TAG, "Starting presence detection");
    start_presence_detection();
    #endif
    while(1) {
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}