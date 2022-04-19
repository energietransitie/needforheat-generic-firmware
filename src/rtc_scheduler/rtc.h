#ifndef RTC_H
#define RTC_H
#include <time.h>
#include "bm8563.h"

extern bm8563_t bm8563;

void rtc_scheduler_init();
time_t rtc_get_time();
void rtc_set_alarm(time_t *alarm);


#endif