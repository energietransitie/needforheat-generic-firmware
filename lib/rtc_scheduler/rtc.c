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
void rtc_set_alarm(interval_t interval) {
 time_t unix_time;
 struct tm rtc_time,*new_time;
 uint8_t reg;
    // Decide to use timer or alarm based on given interval
    if(interval >= INTERVAL_10S && interval < INTERVAL_1M) { // use timer
      // set timer count register
      bm8563_ioctl(&bm8563, BM8563_TIMER_WRITE, &interval);
      // enable timer and set clock frequency to 1 Hz
      reg = 0|(BM8563_TIMER_ENABLE|BM8563_TIMER_1HZ);
      bm8563_ioctl(&bm8563, BM8563_TIMER_CONTROL_WRITE, &reg);
    } 
    else if(interval >= INTERVAL_1M && interval <= INTERVAL_2D) { // use alarm
      // add interval to currrent time
      unix_time = rtc_get_unixtime();
      unix_time += interval; 
      new_time = localtime(&unix_time);
      
      // set rtc alarm with this new time
      rtc_time.tm_wday = BM8563_ALARM_NONE;
      rtc_time.tm_mday = BM8563_ALARM_NONE;
      rtc_time.tm_min = new_time->tm_min;
      rtc_time.tm_hour = new_time->tm_hour;
      bm8563_ioctl(&bm8563, BM8563_ALARM_SET, &rtc_time);
    }

    // clear alarm and timer flag
    bm8563_ioctl(&bm8563,BM8563_CONTROL_STATUS2_READ, &reg);
    reg &= ~(BM8563_AF|BM8563_TF);
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);
}

// reads out the rtc time and print formated string
void rtc_print_time() {
 char buffer[128];
 struct tm rtc;
  bm8563_read(&bm8563, &rtc);
  strftime(buffer, 128 ,"%c (day %j)" , &rtc);
  ESP_LOGD("print","RTC: %s\n", buffer);
}

// reads out rtc register to detmine if the alarm or timer flag is set
bool rtc_check_flag() {
  uint8_t reg;
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
  return (reg & (BM8563_AF|BM8563_TF));
}