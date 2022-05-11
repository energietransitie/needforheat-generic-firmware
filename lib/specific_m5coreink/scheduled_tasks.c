#include "scheduled_tasks.h"
#include <generic_esp_32.h>
#include <scheduler.h>
#include <upload.h>
#include <cJSON.h>

#define PPK2_NUM_D0 GPIO_NUM_25
#define PPK2_NUM_D1 GPIO_NUM_26
#define PPK2_NUM_D2 GPIO_NUM_23
#include <driver/gpio.h>

// test task A
void taskA(void *arg)
{
    gpio_set_level(PPK2_NUM_D1,1);
    ESP_LOGD("taskA", "I am task A and i am running");
    ESP_LOGD("taskA", "I wait 10 seconds");
    vTaskDelay(pdMS_TO_TICKS(10000));
    ESP_LOGD("taskA", "I have done my purpuse, bye");
    gpio_set_level(PPK2_NUM_D1,0);

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}

// test task B
void taskB(void *arg)
{
    gpio_set_level(PPK2_NUM_D2,1);
    ESP_LOGD("taskB", "I am task B and i am running");
    ESP_LOGD("taskB", "I wait 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGD("taskB", "I have done my purpuse, bye");
    gpio_set_level(PPK2_NUM_D2,0);

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}

// heartbeat task
void heartbeatv2_task(void *arg) {
  static int hbcounter = 0;
  char hbcounter_str[80];
  cJSON *measurement_object = NULL;
  cJSON *property_object = NULL;
  cJSON *measurements = NULL;
    
    // create property_object for heartbeat
    property_object = upload_create_property("heartbeat",&measurements);

    // add heartbeat measurement data
    sprintf(hbcounter_str, "%d",hbcounter);
    measurement_object = upload_create_measurement(time(NULL),cJSON_CreateString(hbcounter_str));
    cJSON_AddItemToArray(measurements,measurement_object);

    // add heartbeat to the upload queue
    xQueueSend(upload_queue,(void *) &property_object,portMAX_DELAY);

    // increase hearthbeat counter
    hbcounter++;

    // tell that heartbeat task is completed
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}
