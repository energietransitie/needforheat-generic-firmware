#include "scheduler.h"
#include <generic_esp_32.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define TAG "SCHEDULER"

#define MASK_ALL_TASKS(n) (~((~0)<< ((uint32_t) (n))))

// public global variables
EventGroupHandle_t scheduler_taskevents;

// private global variables
scheduler_t *private_schedule = NULL;
int private_schedule_size = -1;
interval_t private_wake_up_interval;
uint32_t private_tasks_waitbits;

// initailize scheduler functions
void scheduler_init(scheduler_t *sched,int sched_size, interval_t wake_up) {
  // setup schedule
  private_schedule = sched;
  private_schedule_size = sched_size;

  // setup wake up interva;
  private_wake_up_interval = wake_up;

  // create event group for running tasks
  scheduler_taskevents = xEventGroupCreate();
}

// reads out schedule and execute tasks that are due and calculate sleep time
void scheduler_execute_tasks(time_t current) {
 scheduler_t *schedule_item;
  ESP_LOGD(TAG,"execute tasks that are due ...");
  
  private_tasks_waitbits = 0;

  // for each item on the schedule
  schedule_item = private_schedule;
  for(int i=0; i<private_schedule_size; i++,schedule_item++) {
    // if the task is due
    if( (current % schedule_item->interval) == 0) {
      // give id to task
      schedule_item->parameters.id = i;

      // generate wait MASK
      private_tasks_waitbits |= BIT_TASK(i);

      // and execute task
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
}

// let device sleep after all due tasks are completed
void scheduler_sleep(void *in) {
  // wait that all task event bits are cleared
  xEventGroupWaitBits(
    scheduler_taskevents,
    private_tasks_waitbits,
    pdTRUE,
    pdTRUE,
    portMAX_DELAY
    );

  // put system in sleep (privated_wake_up_interval)
  ESP_LOGD("sleep","there are no running task, put system to sleep");

  vTaskDelete(NULL);
}
