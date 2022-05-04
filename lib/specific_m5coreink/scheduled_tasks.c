#include "scheduled_tasks.h"
#include <generic_esp_32.h>
#include <scheduler.h>

// test task A
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

// test task B
void taskB(void *in)
{
    ESP_LOGD("taskB", "I am task B and i am running");
    ESP_LOGD("taskB", "I wait 3 seconds");
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_LOGD("taskB", "I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *)in)->id));
    vTaskDelete(NULL);
}