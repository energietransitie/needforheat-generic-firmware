#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define BIT_TASK(ID) (1 << ID)
#define GET_TASK_BIT_FROM_ARG(ARG) (1 << ((scheduler_parameter_t *)ARG)->id)

typedef enum {
    // intervals for production use
    SCHEDULER_INTERVAL_10S = 10,
    SCHEDULER_INTERVAL_30S = 30,
    SCHEDULER_INTERVAL_1M = 60,
    SCHEDULER_INTERVAL_5M = 300,
    SCHEDULER_INTERVAL_10M = 600,
    SCHEDULER_INTERVAL_30M = 1800,
    SCHEDULER_INTERVAL_1U = 3600,
    SCHEDULER_INTERVAL_2U = 7200,
    SCHEDULER_INTERVAL_6U = 21600,
    SCHEDULER_INTERVAL_12U = 43200,
    SCHEDULER_INTERVAL_1D = 86400,
    // intervals for test use
    SCHEDULER_INTERVAL_2M = 120,
    SCHEDULER_INTERVAL_4M = 240,
    SCHEDULER_INTERVAL_6M = 360
} interval_t;

typedef struct {
    int id;
    void *parameters;
} scheduler_parameter_t;

typedef struct {
    void (*task)(void *);
    char name[30];
    int stack_depth;
    scheduler_parameter_t parameters;
    int priority;
    interval_t task_interval_s;
} scheduler_t;

void scheduler_initialize(scheduler_t *, int, interval_t);
void scheduler_update();
void scheduler_execute_tasks(bool);
void scheduler_wait();
void scheduler_task_finish_last(uint32_t own_task_bit);

extern EventGroupHandle_t scheduler_taskevents;

#endif