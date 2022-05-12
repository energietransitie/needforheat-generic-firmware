#include "upload.h"
#include "cJSON.h"
#include <generic_esp_32.h>
#include <freertos/queue.h>
#include <stdlib.h>
#include <time.h>

QueueHandle_t upload_queue;

// initialize upload
void upload_initialize() {
    // initialize queue
    upload_queue = xQueueCreate(UPLOAD_QUEUE_MAX, sizeof(cJSON **));
}

// send measurement data to the server
void upload_upload() {
 cJSON *upload_object = NULL;
 cJSON *upload_time = NULL;
 cJSON *property_measurements = NULL;
 char *JSON_str;
 cJSON *item = NULL;
    // create json object
    upload_object = cJSON_CreateObject();

    // add upload time
    upload_time = cJSON_CreateNumber(time(NULL));
    cJSON_AddItemToObject(upload_object,"upload_time", upload_time);

    // add property_measurements array
    property_measurements = cJSON_CreateArray();
    cJSON_AddItemToObject(upload_object, "property_measurements", property_measurements);

    // add object to property_measurements array
    while(xQueueReceive(upload_queue,(void *) &item,0) == pdTRUE) {
        cJSON_AddItemToArray(property_measurements,item);
    }

    // send json to server
    JSON_str = cJSON_Print(upload_object);
    ESP_LOGD("upload","JSON : \n%s",JSON_str);
    upload_data_to_server(VARIABLE_UPLOAD_ENDPOINT, POST_WITH_BEARER, JSON_str, NULL, 0);

    // free cjson data structure
    free(JSON_str);
    cJSON_Delete(upload_object);
}

// create property object
cJSON *upload_create_property(const char *name, cJSON **measurements) {
  cJSON *property_object;
  cJSON *property_name;
    // create property object
    property_object = cJSON_CreateObject();
    
    // add property name 
    property_name = cJSON_CreateString(name);
    cJSON_AddItemToObject(property_object,"property_name",property_name);

    // add measurements
    *measurements = cJSON_CreateArray();
    cJSON_AddItemToObject(property_object,"measurements",*measurements);

  return property_object;
}

// create measurement object
cJSON *upload_create_measurement(time_t arg_timestamp, cJSON *value) {
  cJSON *measurement_object;
  cJSON *timestamp;
    // create measurement object
    measurement_object = cJSON_CreateObject();

    // add time stamp
    timestamp = cJSON_CreateNumber(arg_timestamp);
    cJSON_AddItemToObject(measurement_object,"timestamp",timestamp);

    // add value
    cJSON_AddItemToObject(measurement_object,"value",value);
  return measurement_object;
}