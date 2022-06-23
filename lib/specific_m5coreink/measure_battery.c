#include "measure_battery.h"
#include "scheduler.h"
#include "upload.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <stdbool.h>
#include <esp_log.h>

#define TAG "measure battery"

esp_adc_cal_characteristics_t *adc_chars;

// initilaize measure battery task
void measure_battery_initialize() {
    // setup ADC1 channel 7 (gpio35)
    adc1_config_channel_atten(ADC_CHANNEL_7,ADC_ATTEN_11db);

    // calculate characteristic of adc1
    adc_chars = (esp_adc_cal_characteristics_t *) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3600, adc_chars);
}

// scheduler compatible task that measure battery voltage of M5CoreINK
void measure_battery_task(void *arg) {
  static bool initialized = false;
  uint32_t raw_voltage_mV;
  union value_t value;
  int raw_adc;
    ESP_LOGD(TAG, "I go to work.");

    // initialize when this task is executed for first time
    if(initialized == false) {
        measure_battery_initialize();
        initialized = true;
    }

    // measure battery voltage
    raw_adc = adc1_get_raw(ADC1_CHANNEL_7);
    raw_voltage_mV = esp_adc_cal_raw_to_voltage(raw_adc,adc_chars);
    value._float = ((float) raw_voltage_mV) * 25.1 / 5.1 / 1000;
    
    // put measurment on upload queue
    upload_measurement(PROPERTY_BATTERY_VOLTAGE, value);
    
    // tell that is stopped
    ESP_LOGD(TAG, "I have done my job.");
    xEventGroupSetBits(scheduler_taskevents, GET_TASK_BIT_FROM_ARG(arg));
    vTaskDelete(NULL);
}