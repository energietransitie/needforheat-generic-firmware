#ifndef UPLOAD_H
#define UPLOAD_H
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <cJSON.h>

#define UPLOAD_QUEUE_MAX 10

// public variables
extern QueueHandle_t upload_queue;

void upload_initialize();
void upload_upload();

cJSON *upload_create_property(const char *name, cJSON **measurements);
cJSON *upload_create_measurement(time_t arg_timestamp, cJSON *value);

#endif