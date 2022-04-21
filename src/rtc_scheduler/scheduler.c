#include <time.h>
#include <generic_esp_32.h>
#include "scheduler.h"
#include "rtc.h"

// rtc scheduler
void scheduler_start()
{
    // initialize rtc functions
    rtc_scheduler_init();

    // execute scheduler

    // test code
    char tmp;
    do {        
        // print time
        rtc_print_time();

        // set interval
        rtc_set_alarm(INTERVAL_1M);

        // wait for alarm
        ESP_LOGD("print","wait for alarm");
        do{
            vTaskDelay(pdMS_TO_TICKS(500));

        } while (!rtc_check_flag());
        ESP_LOGD("print","alarm goes off!");
    } while (1);
    // end test

    // wait while tasks are running
}

