#include <time.h>
#include <sys/time.h>
#include "rtc.h"
#include "bm8563.h"
#include "i2c_hal.h"
#include "esp_log.h"

// private global variables
bm8563_t bm8563;

// initialize rtc library
void rtc_initialize()
{
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
void rtc_syncronize_sys_time()
{
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
void rtc_print_time()
{
  char buffer[128];
  struct tm rtc;
  bm8563_read(&bm8563, &rtc);
  strftime(buffer, 128, "%c (day %j)", &rtc);
  ESP_LOGD("print", "RTC: %s\n", buffer);
}