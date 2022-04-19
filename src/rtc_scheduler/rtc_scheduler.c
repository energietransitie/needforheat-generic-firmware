#include <driver/gpio.h>
#include <time.h>
#include "bm8563.h"
#include "i2c_hal.h"
#include <generic_esp_32.h>
#include "rtc_scheduler.h"

// private global variables
bm8563_t bm8563;

// private functions
void rtc_scheduler_init();
void powerpin_set();
void powerpin_reset();
time_t rtc_get_time();
void rtc_set_alarm(time_t *alarm);

void rtc_scheduler_init()
{
  time_t current_time;
    // initialize bm8563 functions and i2c hal
    i2c_hal_init();
    bm8563.handle = NULL;
    bm8563.read = i2c_hal_read;
    bm8563.write = i2c_hal_write;
    bm8563_init(&bm8563);

    // set time
    current_time = time(NULL);
    bm8563_write(&bm8563, localtime( &current_time ));
}

// rtc scheduler
void rtc_scheduler_start()
{
    // tell power unit to stay on
    powerpin_set();

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

    // tell power unit to power off
    powerpin_reset();
}

// setup and set GPIO 12
void powerpin_set()
{
    // setup power pin
    gpio_config_t config = {
        .pin_bit_mask = GPIO_SEL_12,
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&config);

    // turn battery power on
    gpio_set_level(GPIO_NUM_12, 1);
}

// reset powerpin
void powerpin_reset()
{
    gpio_set_level(GPIO_NUM_12,0);
}

// get rtc time in seconds 
time_t rtc_get_time()
{
  struct tm rtc_time;
    // read rtc time
    bm8563_read(&bm8563, &rtc_time);

    // convert time to unix time (local time)
    return mktime(&rtc_time);
}

// set rtc alarm
void rtc_set_alarm(time_t *alarm)
{
  struct tm *ptr_rtc_alarm,rtc_alarm;
  uint8_t tmp;
    // clear alarm flag
    bm8563_ioctl(&bm8563,BM8563_CONTROL_STATUS2_READ, &tmp);
    tmp &= ~BM8563_AF;
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &tmp);

    // convert time_t to struct tm
    ptr_rtc_alarm = localtime(alarm);

    // set rtc alarm
    rtc_alarm.tm_wday = BM8563_ALARM_NONE;
    rtc_alarm.tm_mday = BM8563_ALARM_NONE;
    rtc_alarm.tm_min = ptr_rtc_alarm->tm_min;
    rtc_alarm.tm_hour = ptr_rtc_alarm->tm_hour;
    bm8563_ioctl(&bm8563, BM8563_ALARM_SET, &rtc_alarm);
}