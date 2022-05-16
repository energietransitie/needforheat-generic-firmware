#include "scheduled_tasks.h"
#include <generic_esp_32.h>
#include <scheduler.h>
#include <upload.h>
#include <cJSON.h>

// test task A
void taskA(void *arg) {
    ESP_LOGD("taskA", "I am task A and i am running");
    ESP_LOGD("taskA", "I wait 10 seconds");
    vTaskDelay(pdMS_TO_TICKS(10000));
    ESP_LOGD("taskA", "I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}

// test task B
void taskB(void *arg) {
    ESP_LOGD("taskB", "I am task B and i am running");
    ESP_LOGD("taskB", "I wait 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGD("taskB", "I have done my purpuse, bye");

    // tell that is stopped
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}

// test task C
void taskC(void *arg) {
    // wait for other tasks to end
    ESP_LOGD("taskC", "I'am allive, well i wait other tasks to end");
    scheduler_task_finish_last(GET_TASK_BIT_FROM_ARG(arg));
    
    // wait 5 seconds
    ESP_LOGD("taskC", "All tasks are completed now i start to wait 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGD("taskC", "I have done my purpuse, bye");

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

// upload task
void upload_task(void *arg) {
    ESP_LOGD("upload task", "i am going to wait that all tasks are done");
    // wait for other task to end
    scheduler_task_finish_last(GET_TASK_BIT_FROM_ARG(arg));

    // call upload function
    ESP_LOGD("upload task", "call upload function");
    upload_upload();

    // tell that is stopped
    ESP_LOGD("upload task", "i have done my job");
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}
