#include <generic_esp_32.h>

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
const char*device_type_name = "Presence-Detector";
#else
const char*device_type_name = "Generic-Test";
#endif

static const char *TAG = "Twomes ESP32 generic test device";

#define BOOT_STARTUP_INTERVAL_MS (10 * 60) // milliseconds ( 10 s * 1000 ms/s)
#define BOOT_STARTUP_INTERVAL_TXT "Wating 10 seconds before next measurement data series is started"


void app_main(void)
{
    twomes_device_provisioning(device_type_name);

    ESP_LOGI(TAG, "Starting heartbeat task");
    xTaskCreatePinnedToCore(&heartbeat_task, "heartbeat_task", 4096, NULL, 1, NULL, 1);

    ESP_LOGI(TAG, BOOT_STARTUP_INTERVAL_TXT);
    vTaskDelay(BOOT_STARTUP_INTERVAL_MS / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "Starting timesync task");
    xTaskCreatePinnedToCore(&timesync_task, "timesync_task", 4096, NULL, 1, NULL, 1);

    ESP_LOGI(TAG, BOOT_STARTUP_INTERVAL_TXT);
    vTaskDelay(BOOT_STARTUP_INTERVAL_MS / portTICK_PERIOD_MS);

    #ifdef CONFIG_TWOMES_PRESENCE_DETECTION
    ESP_LOGI(TAG, "Starting presence detection");
    start_presence_detection();
    #endif

    while(1) {
        vTaskDelay(1000/ portTICK_PERIOD_MS); //
    }
}