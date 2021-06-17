#include <generic_esp_32.h>

#define HEARTBEAT_UPLOAD_INTERVAL 3600000     //ms, so one hour
#define HEARTBEAT_MEASUREMENT_INTERVAL 600000 //ms, so 10 minutes; not yet in effect

const char*device_type_name = "Presence-Detector";

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
    start_presence_detection();
    while(1) {
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}