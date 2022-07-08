#include "rtc.h"
#include <time.h>
#include <sys/time.h>
#include "i2c_hal.h"
#include "esp_log.h"

// private global variables
bm8563_t bm8563;

// initialize rtc library
void rtc_initialize() {
  // initialize bm8563 functions and i2c hal
  i2c_hal_init();
  bm8563.handle = NULL;
  bm8563.read = i2c_hal_read;
  bm8563.write = i2c_hal_write;
  bm8563_init(&bm8563);
}

// copy system time to rtc
void rtc_syncronize_rtc_time() {
 time_t current_time;
  // set rtc time with system time
  current_time = time(NULL);
  bm8563_write(&bm8563, localtime(&current_time));
}

// copy rtc time to system
void rtc_syncronize_sys_time() {
 struct tm rtc_time;
 struct timeval tv = {0, 0};
 struct timezone tz = {0, 0}; // UTC+0
  // retrieve rtc time
  bm8563_read(&bm8563, &rtc_time);
  tv.tv_sec = mktime(&rtc_time);

  // set system time
  settimeofday(&tv, &tz);
}

// reads out the rtc time and print formated string
void rtc_print_time() {
  char buffer[128];
  struct tm rtc;
  bm8563_read(&bm8563, &rtc);
  strftime(buffer, 128, "%c (day %j)", &rtc);
  ESP_LOGD("print", "RTC: %s\n", buffer);
}

// set rtc alarm for next wake up
void rtc_set_alarm(interval_t interval) {
 time_t unix_time;
 struct tm rtc_time, *new_time;
  uint8_t reg;
  // clear alarm and timer flag
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
  reg &= ~(BM8563_AF | BM8563_TF);
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);

  // Decide to use timer or alarm based on given interval
  if (interval >= SCHEDULER_INTERVAL_10S && interval < SCHEDULER_INTERVAL_1M) { // use timer
    // enable timer interrupt and disable alarm interrupt
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
    reg &= ~(BM8563_AIE);
    reg |= (BM8563_TIE);
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);
    
    // set timer count register
    bm8563_ioctl(&bm8563, BM8563_TIMER_WRITE, &interval);
    
    // enable timer and set clock frequency to 1 Hz
    reg = 0 | (BM8563_TIMER_ENABLE | BM8563_TIMER_1HZ);
    bm8563_ioctl(&bm8563, BM8563_TIMER_CONTROL_WRITE, &reg);
  }
  else if (interval >= SCHEDULER_INTERVAL_1M && interval <= SCHEDULER_INTERVAL_12U) { // use alarm
    // enable alarm interrupt and disable timer interrupt
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
    reg |= (BM8563_AIE);
    reg &= ~(BM8563_TIE);
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);

    // add interval to currrent time
    unix_time = time(NULL);
    unix_time += interval;
    new_time = localtime(&unix_time);

    // set rtc alarm with this new time
    rtc_time.tm_wday = BM8563_ALARM_NONE;
    rtc_time.tm_mday = BM8563_ALARM_NONE;
    rtc_time.tm_min = new_time->tm_min;
    rtc_time.tm_hour = new_time->tm_hour;
    bm8563_ioctl(&bm8563, BM8563_ALARM_SET, &rtc_time);
  }
}
