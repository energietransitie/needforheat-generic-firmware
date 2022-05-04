#include <generic_esp_32.h>

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
#define DEVICE_TYPE_NAME "Presence-Detector"
static const char *TAG = "Twomes ESP32 presence detector";
#else
#define DEVICE_TYPE_NAME "Generic-Test"
static const char *TAG = "Twomes ESP32 generic test device";
#endif

#define BOOT_STARTUP_INTERVAL_MS (10 * 1000) // milliseconds ( 10 s * 1000 ms/s)
#define BOOT_STARTUP_INTERVAL_TXT "Wating 10 seconds before next measurement data series is started"

#if defined ESP32DEV
void app_main(void)
{
    twomes_device_provisioning(DEVICE_TYPE_NAME);
    // TODO: move tasks to new twomes_device_initialization() function in generic firmware library

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

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); //
    }
}
#elif defined M5STACK_COREINK

#include <scheduler.h>
#include <rtc.h>
#include <scheduled_tasks.h>

scheduler_t schedule[] = {
    {taskA, "task a", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_30S},
    {taskB, "task b", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_1M}};
int schedule_size = sizeof(schedule)/sizeof(scheduler_t);
interval_t wakeup_interval = SCHEDULER_INTERVAL_30S;

#include <esp_sleep.h>

void my_deep_sleep(interval_t interval)
{
    // go in deep sleep
    ESP_LOGD("deep sleep","going in deep sleep for %i seconds",interval);
    esp_sleep_enable_timer_wakeup(interval*1000000);
    esp_deep_sleep_start();
}

void app_main(void)
{
 time_t start;
    ESP_LOGD(TAG, "Target is M5Stack_CoreINK");

    // connect with server
    twomes_device_provisioning(DEVICE_TYPE_NAME);
    start = time(NULL);

    // initialize scheduler
    scheduler_init(schedule,schedule_size,wakeup_interval);

    while (1)
    {
        // start tasks that are due
        scheduler_execute_tasks(time(NULL));

        // wait for the end of all running tasks and then sleep
        scheduler_sleep(my_deep_sleep,time(NULL)-start);

        // program will never reach this if the system is put in deep sleep or power off mode.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif
