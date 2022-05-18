#include "scheduler.h"
#include <generic_esp_32.h>
#include <stdio.h>
#include <stdint.h>
#include "rtc.h"
#include "powerpin.h"

// calculate the power off threshold
#define BOOT_ENERGY_MJ ((double) 2.54e2)
#define BOOT_DURATION_S ((double) 1.523)
#define SETUP_ENERGY_MJ ((double) 9.91e2)
#define SETUP_DURATION_S ((double) 4.437)
#define BOOT_SETUP_ENERGY_MJ (BOOT_ENERGY_MJ+SETUP_ENERGY_MJ)
#define BOOT_SETUP_DURATION_S (BOOT_DURATION_S+SETUP_DURATION_S)
#define POWER_OFF_MW ((double) 2.33e-3)
#define LIGHT_SLEEP_MW ((double) 1.15e1)
#define POWER_OFF_THRESHOLD_S ( (BOOT_SETUP_ENERGY_MJ - POWER_OFF_MW * BOOT_SETUP_DURATION_S) / (LIGHT_SLEEP_MW - POWER_OFF_MW))

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
time_t private_current_boot_timestamp;
time_t private_next_boot_timestamp;

// initailize scheduler functions
void scheduler_initialize(scheduler_t *sched,int sched_size, interval_t min_tasks_interval_s) {
    // setup schedule
    private_schedule = sched;
    private_schedule_size = sched_size;

    // setup minium tasks interval
    private_min_tasks_interval_s = min_tasks_interval_s;

    // create event group for running tasks
    scheduler_taskevents = xEventGroupCreate();

    // update scheduler
    scheduler_update();

    private_taskbits_mutex = xSemaphoreCreateMutex();
}

// update scheduler
void scheduler_update() {
  time_t current_time = time(NULL);
    // calculate boot times
    private_current_boot_timestamp = current_time - (current_time % private_min_tasks_interval_s);
    private_next_boot_timestamp = private_current_boot_timestamp + private_min_tasks_interval_s;
}

// reads out schedule and execute tasks that are due and calculate sleep time
void scheduler_execute_tasks() {
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
        if( (private_current_boot_timestamp % schedule_item->task_interval_s) == 0) {
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
void scheduler_wait() {
  time_t current_time, inactive_time_s;
    // wait that all task event bits are cleared
    xEventGroupWaitBits(
        scheduler_taskevents,
        private_tasks_waitbits,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY
    );
    // put system in sleep (privated_wake_up_interval)
    ESP_LOGD(TAG,"There are no running tasks anymore");

    // calculate inactive time
    current_time = time(NULL);
    inactive_time_s = private_next_boot_timestamp - current_time;
    ESP_LOGD(TAG,"Inactive : %li seconds",inactive_time_s);

    // Decide what to do in the inactive time 
    if(inactive_time_s <= 0) {
        // go to next interval
    } else if(inactive_time_s <= POWER_OFF_THRESHOLD_S) {
        // delay (in future use light sleep instead)
        vTaskDelay(pdMS_TO_TICKS(inactive_time_s*1000));
    } else {
        // power off
        rtc_set_alarm(inactive_time_s);
        vTaskDelay(pdMS_TO_TICKS(250));
        powerpin_reset();

        // in case that device did not turn off
        vTaskDelay(pdMS_TO_TICKS(inactive_time_s*1000));
    }
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
