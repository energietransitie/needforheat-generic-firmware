#include <generic_esp_32.h>

#define HEARTBEAT_UPLOAD_INTERVAL 3600000     //ms, so one hour
#define HEARTBEAT_MEASUREMENT_INTERVAL 600000 //ms, so 10 minutes; not yet in effect

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

    xTaskCreatePinnedToCore(&heartbeat_task, "heartbeat_task", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(&timesync_task, "timesync_task", 4096, NULL, 1, NULL, 1);
    //Temporary solution
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    #ifdef CONFIG_TWOMES_PRESENCE_DETECTION
    start_presence_detection();
    #endif
    while(1) {
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}