#include <generic_esp_32.h>
#include <scheduler.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

extern interval_t private_wake_up_interval;

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
#define DEVICE_TYPE_NAME "Presence-Detector"
static const char *TAG = "Twomes ESP32 presence detector";
#else
#define DEVICE_TYPE_NAME "Generic-Test"
static const char *TAG = "Twomes ESP32 generic test device";
#endif


#define BOOT_STARTUP_INTERVAL_MS (10 * 1000) // milliseconds ( 10 s * 1000 ms/s)
#define BOOT_STARTUP_INTERVAL_TXT "Wating 10 seconds before next measurement data series is started"

void taskA(void *in) {
    ESP_LOGD("taskA", "I am task A and i am running");
    ESP_LOGD("taskA", "I wait 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGD("taskA","I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *) in)->id));
    vTaskDelete(NULL);
}

void taskB(void *in) {
    ESP_LOGD("taskB", "I am task B and i am running");
    ESP_LOGD("taskB", "I wait 2 seconds");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGD("taskB","I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *) in)->id));
    vTaskDelete(NULL);
}

void tasksleep(void *in) {
    scheduler_sleep();
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGD(TAG, "platform target %s", TARGET_ENV);

    scheduler_t schedule[] = {
        {taskA, "task a", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_1M},
        {taskB, "task b", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_30S}
    };
    scheduler_init(schedule,sizeof(schedule)/sizeof(scheduler_t),SCHEDULER_INTERVAL_30S);

    //twomes_device_provisioning(DEVICE_TYPE_NAME);

    //scheduler_start();

    //xTaskCreate(scheduler_start,"test",6024,NULL,1,NULL);

    //TODO: move tasks to new twomes_device_initialization() function in generic firmware library

    /*
    ESP_LOGD(TAG, "Starting heartbeat task");
    xTaskCreatePinnedToCore(&heartbeat_task, "heartbeat_task", 4096, NULL, 1, NULL, 1);

    ESP_LOGD(TAG, BOOT_STARTUP_INTERVAL_TXT);
    vTaskDelay(BOOT_STARTUP_INTERVAL_MS / portTICK_PERIOD_MS);
    
    ESP_LOGD(TAG, "Starting timesync task");
    xTaskCreatePinnedToCore(&timesync_task, "timesync_task", 4096, NULL, 1, NULL, 1);

    ESP_LOGD(TAG, BOOT_STARTUP_INTERVAL_TXT);
    vTaskDelay(BOOT_STARTUP_INTERVAL_MS / portTICK_PERIOD_MS);

    #ifdef CONFIG_TWOMES_PRESENCE_DETECTION
    ESP_LOGD(TAG, "Starting presence detection");
    start_presence_detection();
    #endif
    */

    while(1) {
        time_t current = time(NULL);
        ESP_LOGD("rtc","time is %li",current);
        if((current % private_wake_up_interval) == 0) {
            ESP_LOGD("rtc","wake up..");
            scheduler_execute_tasks(current);
            xTaskCreate(tasksleep,"sleep",4096, (void *) SCHEDULER_INTERVAL_30S,3,NULL);
            //scheduler_sleep(SCHEDULER_INTERVAL_30S);
        }
        vTaskDelay(1000/ portTICK_PERIOD_MS); //
        
    }
}