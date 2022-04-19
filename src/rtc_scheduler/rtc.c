#include <time.h>
#include "rtc.h"
#include "bm8563.h"
#include "i2c_hal.h"

// private global variables
bm8563_t bm8563; //(in future private)

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