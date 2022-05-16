#include "scheduler.h"
#include <generic_esp_32.h>
#include <stdio.h>
#include <stdint.h>

#define TAG "SCHEDULER"

// public global variables
EventGroupHandle_t scheduler_taskevents;

// private global variables
scheduler_t *private_schedule = NULL;
int private_schedule_size = -1;
interval_t private_min_tasks_interval_s;
uint32_t private_tasks_waitbits;
uint32_t private_blocked_tasks_bits;
SemaphoreHandle_t private_taskbits_mutex;

// initailize scheduler functions
void scheduler_initialize(scheduler_t *sched,int sched_size, interval_t min_tasks_interval_s) {
    // setup schedule
    private_schedule = sched;
    private_schedule_size = sched_size;

    // setup minium tasks interval
    private_min_tasks_interval_s = min_tasks_interval_s;

    // create event group for running tasks
    scheduler_taskevents = xEventGroupCreate();

    private_taskbits_mutex = xSemaphoreCreateMutex();
}

// reads out schedule and execute tasks that are due and calculate sleep time
void scheduler_execute_tasks(time_t current) {
  scheduler_t *schedule_item;
    ESP_LOGD(TAG,"execute tasks that are due ...");

    xSemaphoreTake(private_taskbits_mutex,portMAX_DELAY);

    // clear all task wait bits
    private_tasks_waitbits = 0;
    private_blocked_tasks_bits = 0;

    // for each item on the schedule
    schedule_item = private_schedule;
    for(int i=0; i<private_schedule_size; i++,schedule_item++) {
        // if the task is due
        if( (current % schedule_item->interval) < private_min_tasks_interval_s) {
            // pas task id to task
            schedule_item->parameters.id = i;

            // set wait bit for this task
            private_tasks_waitbits |= BIT_TASK(i);

            // execute task
            ESP_LOGD(TAG,"task %s is due, execute ...",schedule_item->name);
            xTaskCreate(
                schedule_item->task,
                schedule_item->name,
                schedule_item->stack_depth,
                (void *) &schedule_item->parameters,
                schedule_item->priority,
                NULL);
        }
    }
    xSemaphoreGive(private_taskbits_mutex);
}

// put device in sleep after waiting that all due tasks are completed
void scheduler_wait(void (*sleep_function)(interval_t)) {
  time_t current_time,wake_up_time,sleep_period,current_wake_up_time;
    // wait that all task event bits are cleared
    xEventGroupWaitBits(
        scheduler_taskevents,
        private_tasks_waitbits,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY
    );

    // calculate sleep time
    current_time = time(NULL);
    current_wake_up_time = (current_time - (current_time % private_min_tasks_interval_s));
    wake_up_time = current_wake_up_time+private_min_tasks_interval_s;
    sleep_period = wake_up_time - current_time;
    ESP_LOGD("sleep","wake up time: %li",wake_up_time);

    // put system in sleep (privated_wake_up_interval)
    ESP_LOGD("sleep","there are no running task, put system to sleep");
    sleep_function(sleep_period);
}

// function put task in blocked state until the other task are ended
// (Use function only in tasks started by scheduler)
void scheduler_task_finish_last(uint32_t own_task_bit) {
  uint32_t waitbits = 0;
    xSemaphoreTake(private_taskbits_mutex,portMAX_DELAY);

    // adminstrate task bit to prevent other blocked task to wait for blocked tasks
    private_blocked_tasks_bits |= own_task_bit;

    // calculate on which tasks to wait
    waitbits = private_tasks_waitbits & (~private_blocked_tasks_bits);

    xSemaphoreGive(private_taskbits_mutex);

    // put task in blocked state
    xEventGroupWaitBits(
        scheduler_taskevents,
        waitbits,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );
}
