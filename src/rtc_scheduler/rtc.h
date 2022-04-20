#ifndef RTC_H
#define RTC_H
#include <time.h>
#include <stdbool.h>
#include "bm8563.h"

extern bm8563_t bm8563;

typedef enum {
    INTERVAL_10S=10,
    INTERVAL_15S=15,
    INTERVAL_30S=30,
    INTERVAL_1M=60,
    INTERVAL_5M=300,
    INTERVAL_10M=600,
    INTERVAL_15M=900,
    INTERVAL_30M=1800,
    INTERVAL_1U=3600,
    INTERVAL_2U=7200,
    INTERVAL_3U=10800,
    INTERVAL_4U=14400,
    INTERVAL_6U=21600,
    INTERVAL_12U=43200,
    INTERVAL_1D=86400,
    INTERVAL_2D=172800
} interval_t;

void rtc_scheduler_init();
time_t rtc_get_time();
void rtc_set_alarm(interval_t alarm);

void rtc_print_time();
bool rtc_check_TF();
bool rtc_check_AF();

#endif