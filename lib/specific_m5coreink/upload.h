#ifndef UPLOAD_H
#define UPLOAD_H
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <cJSON.h>

#define UPLOAD_QUEUE_MAX 10

#include "property_format.h"

typedef struct {
    property_t property;
    time_t timestamp;
    union {
        int _int;
        float _float;
    } value;
} measurement_t;

// public variables
extern QueueHandle_t upload_queue;

void upload_initialize();
void upload_upload();

#endif