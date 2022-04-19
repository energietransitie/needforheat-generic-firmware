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
    do {        
        // print time
        bm8563_read(&bm8563, &rtc);
        strftime(buffer, 128 ,"%c (day %j)" , &rtc);
        ESP_LOGD("print","RTC: %s\n", buffer);

        rtc_set_alarm(INTERVAL_1M);

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

