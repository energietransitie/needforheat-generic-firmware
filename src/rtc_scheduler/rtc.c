#include <time.h>
#include "rtc.h"
#include "bm8563.h"
#include "i2c_hal.h"

#include <generic_esp_32.h>

// private global variables
bm8563_t bm8563; //(in future private)

void rtc_scheduler_init()
{
  time_t current_time;
  uint8_t reg;
    // initialize bm8563 functions and i2c hal
    i2c_hal_init();
    bm8563.handle = NULL;
    bm8563.read = i2c_hal_read;
    bm8563.write = i2c_hal_write;
    bm8563_init(&bm8563);

    // set time
    current_time = time(NULL);
    bm8563_write(&bm8563, localtime( &current_time ));

    // set enable rtc interupt pin
    bm8563_ioctl(&bm8563,BM8563_CONTROL_STATUS2_READ,&reg);
    reg |= BM8563_AIE|BM8563_TIE;
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);
}

// get rtc time in seconds 
time_t rtc_get_unixtime()
{
  struct tm rtc_time;
    // read rtc time
    bm8563_read(&bm8563, &rtc_time);

    // convert time to unix time (local time)
    return mktime(&rtc_time);
}

// set rtc alarm
void rtc_set_alarm(interval_t alarm) {
 time_t time;
 struct tm rtc_alarm,*ptr_alarm;
 uint8_t tmp;
    // get current rtc time
    time = rtc_get_unixtime();

    // add interval to time
    time += alarm;

    // convert 
    ptr_alarm = localtime(&time);
    
    if(alarm >= INTERVAL_10S && alarm < INTERVAL_1M) {
      // set rtc timer
      bm8563_ioctl(&bm8563, BM8563_TIMER_WRITE, &alarm);
      tmp = 0|(BM8563_TIMER_ENABLE|BM8563_TIMER_1HZ);
      bm8563_ioctl(&bm8563, BM8563_TIMER_CONTROL_WRITE, &tmp);
    } 
    else if(alarm >= INTERVAL_1M && alarm <= INTERVAL_2D) {
      // set rtc alarm
      rtc_alarm.tm_wday = ptr_alarm->tm_wday;
      rtc_alarm.tm_mday = BM8563_ALARM_NONE;
      rtc_alarm.tm_min = ptr_alarm->tm_min;
      rtc_alarm.tm_hour = ptr_alarm->tm_hour;
      bm8563_ioctl(&bm8563, BM8563_ALARM_SET, &rtc_alarm);
    }

    // clear alarm and timer flag
    bm8563_ioctl(&bm8563,BM8563_CONTROL_STATUS2_READ, &tmp);
    tmp &= ~(BM8563_AF|BM8563_TF);
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &tmp);
}

void rtc_print_time() {
 char buffer[128];
 struct tm rtc;
  bm8563_read(&bm8563, &rtc);
  strftime(buffer, 128 ,"%c (day %j)" , &rtc);
  ESP_LOGD("print","RTC: %s\n", buffer);
}

bool rtc_check_AF() {
  uint8_t reg;
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
  return (reg & BM8563_AF);
}

bool rtc_check_TF() {
  uint8_t reg;
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
  return (reg & BM8563_TF);
}