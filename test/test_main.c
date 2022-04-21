#include <unity.h>
#include <time.h>
#include <stdint.h>

#include <bm8563.h>
#include <i2c_hal.h>
#include <rtc.h>

// needed for vtaskdelay function
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// extern from rtc.h
// bm8563_t bm8563;

void init() {
    i2c_hal_init();
    bm8563.handle = NULL;
    bm8563.read = i2c_hal_read;
    bm8563.write = i2c_hal_write;
    bm8563_init(&bm8563);
}

void setup_rtc_alarm(struct tm *in, int min, int hour, int wday, int mday) {
    // define contents to write
    in->tm_wday = 4;
    in->tm_mday = 5;
    in->tm_min = 15;
    in->tm_hour = 3;
      
    // write rtc alarm
    bm8563_ioctl(&bm8563, BM8563_ALARM_SET, in);
}

// check if the library copy the alarm correctly
void test_rtc_write_alarm() {
 struct tm write_time={0}, read_time={0};
    // setup alarm
    setup_rtc_alarm(&write_time, 15, 3, 4, 5);

    // read rtc alarm
    bm8563_ioctl(&bm8563, BM8563_ALARM_READ, &read_time);

    // check if contens are the same
    TEST_ASSERT_EQUAL(write_time.tm_wday, read_time.tm_wday);
    TEST_ASSERT_EQUAL(write_time.tm_mday, read_time.tm_mday);
    TEST_ASSERT_EQUAL(write_time.tm_min, read_time.tm_min);
    TEST_ASSERT_EQUAL(write_time.tm_hour, read_time.tm_hour);
}

// check if the library copy the correct time to rtc
void test_rtc_write_time() {
 struct tm write_time={},read_time={};
    // setup time
    /* 2020-12-31 23:59:45 */
    write_time.tm_year = 2020 - 1900;
    write_time.tm_mon = 12 - 1;
    write_time.tm_mday = 31;
    write_time.tm_hour = 23;
    write_time.tm_min = 59;
    write_time.tm_sec = 45;

    // write time to rtc
    bm8563_write(&bm8563, &write_time);

    // read time from rtc
    bm8563_read(&bm8563, &read_time);

    // check if contens are the same
    TEST_ASSERT_EQUAL(write_time.tm_year, read_time.tm_year);
    TEST_ASSERT_EQUAL(write_time.tm_mon, read_time.tm_mon);
    TEST_ASSERT_EQUAL(write_time.tm_mday, read_time.tm_mday);
    TEST_ASSERT_EQUAL(write_time.tm_hour, read_time.tm_hour);
    TEST_ASSERT_EQUAL(write_time.tm_min, read_time.tm_min);
    TEST_ASSERT_EQUAL(write_time.tm_sec, read_time.tm_sec);
}

void test_rtc_minute_alarm() {
 struct tm write_time={},write_rtc;
 uint8_t reg;
    // setup time
    /* 2022-1-1 00:00:55 */
    write_time.tm_year = 2022 - 1900;
    write_time.tm_mon = 1 - 1;
    write_time.tm_mday = 1;
    write_time.tm_hour = 0;
    write_time.tm_min = 0;
    write_time.tm_sec = 55;

    // write time to rtc
    bm8563_write(&bm8563, &write_time);

    // clear alarm flag
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
    reg &= ~BM8563_AF;
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_WRITE, &reg);

    // setup rtc alarm
    setup_rtc_alarm(
        &write_rtc,
        1,
        0,
        BM8563_ALARM_NONE,
        BM8563_ALARM_NONE);   

    // wait 1 second
    vTaskDelay(pdMS_TO_TICKS(10000));

    // check alarm flag
    bm8563_ioctl(&bm8563, BM8563_CONTROL_STATUS2_READ, &reg);
    TEST_ASSERT_TRUE(reg & BM8563_AF);
}

void app_main() {
    UNITY_BEGIN();
    
    init();
    
    RUN_TEST(test_rtc_write_alarm);
    RUN_TEST(test_rtc_write_time);
    RUN_TEST(test_rtc_minute_alarm);

    UNITY_END();
}