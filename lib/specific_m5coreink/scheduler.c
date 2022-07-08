#include "scheduler.h"
#include <generic_esp_32.h>
#include <stdio.h>
#include <stdint.h>
#include "rtc.h"
#include "powerpin.h"
#include <stdbool.h>

// calculate the power off threshold
#define BOOT_ENERGY_mJ ((double) 2.54e2)
#define BOOT_DURATION_s ((double) 1.523)
#define SETUP_ENERGY_mJ ((double) 9.91e2)
#define SETUP_DURATION_s ((double) 4.437)
#define BOOT_SETUP_ENERGY_mJ (BOOT_ENERGY_mJ+SETUP_ENERGY_mJ)
#define BOOT_SETUP_DURATION_s (BOOT_DURATION_s+SETUP_DURATION_s)
#define POWER_OFF_mW ((double) 2.33e-3)
#define LIGHT_SLEEP_mW ((double) 1.15e1)
#define POWER_OFF_THRESHOLD_s ( (BOOT_SETUP_ENERGY_mJ - POWER_OFF_mW * BOOT_SETUP_DURATION_s) / (LIGHT_SLEEP_mW - POWER_OFF_mW))

#define TAG "SCHEDULER"
#define BIT_INTERVAL_ENDED (1 << 23)
#define BIT_RESTART_REQUESTED (1 << 22)

// public global variables
EventGroupHandle_t scheduler_taskevents;

// private functions
void task_detect_next_interval(void *arg);

// private global variables
scheduler_t *private_schedule = NULL;
int private_schedule_size = -1;
interval_t private_min_tasks_interval_s;
uint32_t private_tasks_waitbits;
uint32_t private_blocked_tasks_bits;
SemaphoreHandle_t private_taskbits_mutex;
time_t private_current_boot_timestamp;
time_t private_next_boot_timestamp;
EventBits_t private_still_running;
SemaphoreHandle_t next_interval_sem;

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
    next_interval_sem = xSemaphoreCreateBinary();
}

// update scheduler
void scheduler_update() {
  time_t current_time = time(NULL);
    // calculate boot times
    private_current_boot_timestamp = current_time - (current_time % private_min_tasks_interval_s);
    private_next_boot_timestamp = private_current_boot_timestamp + private_min_tasks_interval_s;
    ESP_LOGD("interval","go to interval : %li <-> %li",private_current_boot_timestamp,private_next_boot_timestamp);
}

// reads out schedule and execute tasks that are due and calculate sleep time
void scheduler_execute_tasks(bool previous_interval_active) {
  scheduler_t *schedule_item;
    ESP_LOGD(TAG,"execute tasks that are due ...");

    xSemaphoreTake(private_taskbits_mutex,portMAX_DELAY);

    // when the previous interval is not active
    if(!previous_interval_active) {
        // clear all task wait bits
        private_tasks_waitbits = 0;
        private_blocked_tasks_bits = 0;
        private_still_running = 0;
    }

    // for each item on the schedule
    schedule_item = private_schedule;
    for(int i=0; i<private_schedule_size; i++,schedule_item++) {
        // if the task is due and not running
        if( (private_current_boot_timestamp % schedule_item->task_interval_s) == 0 && 
            (private_still_running & BIT_TASK(i)) == 0) {
            // pass the task id to task
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

// task that triger scheduler_wait function to start new interval when there are still tasks running.
void task_detect_next_interval(void *arg) {
  time_t current_time, time_left;
    // calculate when current interval ends
    current_time = time(NULL);
    time_left = private_next_boot_timestamp - current_time;

    // if there are still tasks running when the interval has passed
    if(xSemaphoreTake(next_interval_sem,pdMS_TO_TICKS(time_left*1000)) == pdFALSE) {
        ESP_LOGD(TAG,"Start with the next interval");

        // register tasks that are still running
        private_still_running = (~xEventGroupGetBits(scheduler_taskevents)) & private_tasks_waitbits;

        // end current interval
        xEventGroupSetBits(scheduler_taskevents, private_tasks_waitbits | BIT_INTERVAL_ENDED);
    }

    vTaskDelete(NULL);
}

// put device in sleep after waiting that all due tasks are completed
void scheduler_wait() {
  time_t current_time, inactive_time_s;
  TaskHandle_t handle = NULL;

wait_for_end_interval: 
    // start task that detect end of current interval
    xTaskCreate(task_detect_next_interval,"interval ended detection",4024,NULL,2,&handle);

    // wait that all task event bits are cleared
    xEventGroupWaitBits(
        scheduler_taskevents,
        private_tasks_waitbits,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    // if the current interval is ended by task_detect_next_interval
    if(xEventGroupGetBits(scheduler_taskevents) & BIT_INTERVAL_ENDED) { // if next interval is detected
        // start the new due tasks among the still running tasks
        scheduler_update();
        scheduler_execute_tasks(true);
        xEventGroupClearBits(scheduler_taskevents,private_tasks_waitbits | BIT_INTERVAL_ENDED);
        goto wait_for_end_interval;
    } else {
        ESP_LOGD(TAG,"There are no running tasks anymore");

        // tell task_detect_next_interval the current interval is completed and handeled
        xSemaphoreGive(next_interval_sem);
    }

    // Check if restart is requested
    if (xEventGroupGetBits(scheduler_taskevents) & BIT_RESTART_REQUESTED) {
        ESP_LOGD(TAG, "A restart was requested. Restarting now...");
        esp_restart();
    }

    // clear the taskevent bits
    xEventGroupClearBits(scheduler_taskevents,private_tasks_waitbits | BIT_INTERVAL_ENDED);
    
    // calculate inactive time
    current_time = time(NULL);
    inactive_time_s = private_next_boot_timestamp - current_time;
    ESP_LOGD(TAG,"Inactive : %li seconds",inactive_time_s);

    // Decide what to do in the inactive time 
    if(inactive_time_s <= 0) {
        // go to next interval
    } else if(inactive_time_s <= POWER_OFF_THRESHOLD_s) {
        // delay (in future use light sleep instead to save power)
        vTaskDelay(pdMS_TO_TICKS(inactive_time_s*1000));
    } else {
        // power off
        ESP_LOGD("power off","power off for %li seconds",inactive_time_s);
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

void scheduler_request_restart() {
    xEventGroupSetBits(scheduler_taskevents, BIT_RESTART_REQUESTED);
}
