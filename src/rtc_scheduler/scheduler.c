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
    char buffer[128],tmp;
    struct tm rtc;
    time_t unix_rtc;
    do {        
        // print time
        bm8563_read(&bm8563, &rtc);
        strftime(buffer, 128 ,"%c (day %j)" , &rtc);
        ESP_LOGD("print","RTC: %s\n", buffer);

        // get unix time
        unix_rtc = rtc_get_time();
        ESP_LOGD("print","unix time = %li",unix_rtc);
        
        // add 60 seconds
        unix_rtc += 60;
        ESP_LOGD("print","unix time + 60 = %li",unix_rtc);

        // set alarm
        rtc_set_alarm(&unix_rtc);

        // wait for alarm
        ESP_LOGD("print","wait for alarm");
        do{
            vTaskDelay(pdMS_TO_TICKS(500));
            bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &tmp);

        } while (!(tmp & BM8563_AF));
        ESP_LOGD("print","alarm goes off!");
    } while (1);
    // end test

    // wait while tasks are running
}

