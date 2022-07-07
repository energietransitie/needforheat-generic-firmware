#include "scheduled_tasks.h"
#include <generic_esp_32.h>
#include <scheduler.h>
#include <upload.h>
#include <cJSON.h>
#include <rtc.h>

// heartbeat task
void heartbeatv2_task(void *arg) {
  static int hbcounter = 0;
  measurement_t heartbeat_object;
  
    // create measurement struct  
    heartbeat_object.timestamp = time(NULL);
    heartbeat_object.property = PROPERTY_HEARTBEAT;
    heartbeat_object.value._int = hbcounter;
    
    // add heartbeat to the upload queue
    xQueueSend(upload_queue, (void *) &heartbeat_object,portMAX_DELAY);

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