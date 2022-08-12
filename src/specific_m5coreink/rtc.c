#include <specific_m5coreink/rtc.h>
#include <time.h>
#include <sys/time.h>
#include <i2c_hal.h>
#include <esp_log.h>

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
void rtc_set_alarm(time_t alarm_time) {
  uint8_t reg;
  // clear alarm and timer flag
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
  reg &= ~(BM8563_AF | BM8563_TF);
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);

  // enable alarm interrupt and disable timer interrupt
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
  reg |= (BM8563_AIE);
  reg &= ~(BM8563_TIE);
  bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);

  struct tm rtc_time = *(localtime(&alarm_time));

  // set rtc alarm.
  bm8563_ioctl(&bm8563, BM8563_ALARM_SET, &rtc_time);
}

// get the time that the RTC alarm was set to
time_t rtc_get_alarm() {
  struct tm rtc_time;

  // read the rtc alarm
  bm8563_ioctl(&bm8563, BM8563_ALARM_READ, &rtc_time);

  return mktime(&rtc_time);
}
