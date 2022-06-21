#include "scheduled_tasks.h"
#include <generic_esp_32.h>
#include <scheduler.h>
#include <upload.h>
#include <cJSON.h>
#include <twomes_scd41.h>
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

// task that measure with SCD41
void twomes_scd41_task(void *arg) {
  measurement_t object;
  time_t timestamp;
    ESP_LOGD("SCD41 task", "I go to work");
    // initailize co2
    co2_init(SCD41_ADDR);

    // perform measurements
    uint16_t measurement_data[3] = {};
    for(int i=0; i < 3; i++) {
        co2_read(SCD41_ADDR,measurement_data);
    }

    // add CO2 measurement to the upload queue
    timestamp = time(NULL);
    object.property = CO2_CONCENTRATION;
    object.timestamp = timestamp;
    object.value._uint16 = measurement_data[0];
    xQueueSend(upload_queue, (void *) &object,portMAX_DELAY); 

    // add room temprature measurement to the upload queue
    object.property = ROOM_TEMP_CO2;
    object.value._float = scd41_temp_raw_to_celsius(measurement_data[1]);
    xQueueSend(upload_queue, (void *) &object,portMAX_DELAY);

    // add humidity measurement to the upload queue
    object.property = HUMIDITY;
    object.value._float = scd41_rh_raw_to_percent(measurement_data[2]);
    xQueueSend(upload_queue, (void *) &object,portMAX_DELAY);

    // tell that is stopped
    ESP_LOGD("SCD41 task", "I have done my job");
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}