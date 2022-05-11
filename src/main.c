#include <generic_esp_32.h>

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
#include <c_ota.h>
#endif

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

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
    twomes_ota_start();
#endif // CONFIG_TWOMES_OTA_FIRMWARE_UPDATE

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

#define PPK2_SEL PPK2_SEL_D0|PPK2_SEL_D1|PPK2_SEL_D2 
#define PPK2_SEL_D0 GPIO_SEL_25
#define PPK2_SEL_D1 GPIO_SEL_26
#define PPK2_SEL_D2 GPIO_SEL_23

#define PPK2_NUM_D0 GPIO_NUM_25
#define PPK2_NUM_D1 GPIO_NUM_26
#define PPK2_NUM_D2 GPIO_NUM_23

#include <scheduler.h>
#include <rtc.h>
#include <scheduled_tasks.h>
#include <powerpin.h>
#include <upload.h>

scheduler_t schedule[] = {
    {heartbeatv2_task, "heartbeat1", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_1M},
    {taskA, "task a", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_1M},
    {heartbeatv2_task, "heartbeat2", 4096, {0, NULL}, 1, SCHEDULER_INTERVAL_1M},
    {taskB, "task b", 4096, {0, NULL}, 1, 120}};
int schedule_size = sizeof(schedule)/sizeof(scheduler_t);
interval_t wakeup_interval = SCHEDULER_INTERVAL_1M;

#include <esp_sleep.h>

void my_deep_sleep(interval_t interval)
{
    // upload
    upload_upload();

    // go in deep sleep
    ESP_LOGD("deep sleep","going in deep sleep for %i seconds",interval);
    //esp_sleep_enable_timer_wakeup(interval*1000000);
    //esp_light_sleep_start();
    vTaskDelay(pdMS_TO_TICKS(interval*1000));
}

#include <driver/gpio.h>
#include <upload.h>

void app_main(void)
{
    upload_initialize();
    powerpin_set();
    ESP_LOGD(TAG, "Target is M5Stack_CoreINK");

    // setup test gpio pins for PPKII
    gpio_config_t config = {
        PPK2_SEL,
        GPIO_MODE_OUTPUT,
        GPIO_PULLUP_DISABLE,
        GPIO_PULLDOWN_DISABLE,
        GPIO_INTR_DISABLE
    };
    gpio_config(&config);
    gpio_set_level(PPK2_NUM_D0,0);
    gpio_set_level(PPK2_NUM_D1,0);
    gpio_set_level(PPK2_NUM_D2,0);


    // connect with server
    gpio_set_level(PPK2_NUM_D0,1);
    twomes_device_provisioning(DEVICE_TYPE_NAME);
    gpio_set_level(PPK2_NUM_D0,0);

    // initailize rtc
    rtc_initialize();
    rtc_syncronize_rtc_time();

    // initialize scheduler
    scheduler_init(schedule,schedule_size,wakeup_interval);

    while (1)
    {
        // start tasks that are due
        scheduler_execute_tasks(time(NULL));

        // wait for the end of all running tasks and then sleep
        scheduler_sleep(my_deep_sleep);
        powerpin_reset();

        // program will never reach this if the system is put in deep sleep or power off mode.
        //vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif
