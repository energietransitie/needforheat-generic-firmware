#include <generic_esp_32.h>
#include <scheduler.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <math.h>
#include <rtc.h>

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

void taskA(void *in)
{
    ESP_LOGD("taskA", "I am task A and i am running");
    ESP_LOGD("taskA", "I wait 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGD("taskA", "I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *)in)->id));
    vTaskDelete(NULL);
}

void taskB(void *in)
{
    ESP_LOGD("taskB", "I am task B and i am running");
    ESP_LOGD("taskB", "I wait 2 seconds");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGD("taskB", "I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *)in)->id));
    vTaskDelete(NULL);
}

void tasksleep(void *in)
{
    scheduler_sleep();
    rtc_set_alarm(SCHEDULER_INTERVAL_30S);
    vTaskDelete(NULL);
}

void app_main(void)
{
    twomes_device_provisioning(DEVICE_TYPE_NAME);

    rtc_initialize();
    rtc_syncronize_rtc_time();

    ESP_LOGD(TAG, "platform target %s", TARGET_ENV);

    scheduler_t schedule[] = {
        {taskA, "task a", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_1M},
        {taskB, "task b", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_30S}
    };
    scheduler_init(schedule,sizeof(schedule)/sizeof(scheduler_t),SCHEDULER_INTERVAL_30S);
    

    // twomes_device_provisioning(DEVICE_TYPE_NAME);

    // scheduler_start();

    // xTaskCreate(scheduler_start,"test",6024,NULL,1,NULL);

    // TODO: move tasks to new twomes_device_initialization() function in generic firmware library

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

   rtc_set_alarm(SCHEDULER_INTERVAL_30S);

   uint8_t tmp;
   time_t tijd;
    while (1)
    {
        // simulate rtc
        time_t current = time(NULL), realy_read;
        ESP_LOGD("rtc", "time is %li", current);
        bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &tmp);
        if (tmp & (BM8563_AF | BM8563_TF))
        {
            time(&tijd);
            ESP_LOGD("rtc", "wake up.. on %li",tijd);
            realy_read = current; //+ rand() % SCHEDULER_INTERVAL_30S;
            ESP_LOGD("wake up", "I read the following time %li", realy_read);
            scheduler_execute_tasks(realy_read);
            xTaskCreate(tasksleep, "sleep", 4096, (void *)SCHEDULER_INTERVAL_30S, 3, NULL);

            bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &tmp);
            tmp &= ~(BM8563_AF | BM8563_TF);
            bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &tmp);
        }

        // print rtc time and sys time
        /*
        rtc_print_time();
        char buffer[128];
        time_t test;
        struct tm *systime;
        test = time(NULL);
        systime = localtime(&test);
        strftime(buffer, 128, "%c (day %j)", systime);
        ESP_LOGD(TAG, "sys time: %s\n", buffer);
        */
        
        vTaskDelay(1000 / portTICK_PERIOD_MS); //
    }
}