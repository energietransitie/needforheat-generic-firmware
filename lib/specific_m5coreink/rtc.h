#ifndef RTC_H
#define RTC_H
#include <time.h>
#include <stdbool.h>
#include "bm8563.h"

extern bm8563_t bm8563;

void rtc_ainit();
void rtc_syncronize_rtc_time();
void rtc_syncronize_sys_time();
void rtc_print_time();


#endif