#ifndef UPLOAD_H
#define UPLOAD_H
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <cJSON.h>

#define UPLOAD_QUEUE_MAX 50

#include "property_format.h"

union value_t {
        int _int;
        uint16_t _uint16;
        uint32_t _uint32;
        float _float;
};

typedef struct {
    property_t property;
    time_t timestamp;
    union value_t value;
} measurement_t;

// public variables
extern QueueHandle_t upload_queue;

void upload_initialize();
void upload_upload();

// helper functions
void upload_measurement(property_t, union value_t);

#endif