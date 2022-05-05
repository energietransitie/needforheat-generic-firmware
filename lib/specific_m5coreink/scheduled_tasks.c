#include "scheduled_tasks.h"
#include <generic_esp_32.h>
#include <scheduler.h>

#define PPK2_NUM_D0 GPIO_NUM_25
#define PPK2_NUM_D1 GPIO_NUM_26
#define PPK2_NUM_D2 GPIO_NUM_23
#include <driver/gpio.h>

// test task A
void taskA(void *in)
{
    gpio_set_level(PPK2_NUM_D1,1);
    ESP_LOGD("taskA", "I am task A and i am running");
    ESP_LOGD("taskA", "I wait 10 seconds");
    vTaskDelay(pdMS_TO_TICKS(10000));
    ESP_LOGD("taskA", "I have done my purpuse, bye");
    gpio_set_level(PPK2_NUM_D1,0);

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *)in)->id));
    vTaskDelete(NULL);
}

// test task B
void taskB(void *in)
{
    gpio_set_level(PPK2_NUM_D2,1);
    ESP_LOGD("taskB", "I am task B and i am running");
    ESP_LOGD("taskB", "I wait 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGD("taskB", "I have done my purpuse, bye");
    gpio_set_level(PPK2_NUM_D2,0);

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, BIT_TASK(((scheduler_parameter_t *)in)->id));
    vTaskDelete(NULL);
}