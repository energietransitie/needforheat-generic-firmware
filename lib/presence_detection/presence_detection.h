#ifndef PRESENCE_DETECTION_H
#define PRESENCE_DETECTION_H

#define HTTPS_PRE_PRESENCE_UPLOAD_ADDITIONAL_WAIT_MS (10) // milliseconds

#ifdef CONFIG_TWOMES_STRESS_TEST
#define PRESENCE_MEASUREMENT_INTERVAL_S (1 * 60) // seconds ( 1 min * 60 s/min)
#define PRESENCE_INTERVAL_TXT "Wating 1 minute before next presence scan"
#else
#define PRESENCE_MEASUREMENT_INTERVAL_S (1 * 60 * 60) // seconds (1 hr 60 min/hr * 60 s/min)
#define PRESENCE_INTERVAL_TXT "Wating 1 hour before next presence scan"
#endif

#define RSSI_PRESENT 900
#define RSSI_ABSENT -899

#include <generic_esp_32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include <esp_gatt_common_api.h>

#include "esp_bt_defs.h"
#include "esp_gap_bt_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#define PRESENCE_DATA_SIZE 11
typedef struct
{
    esp_bd_addr_t addr;
    bool isHome;
    uint32_t timeMeasured;
} presence_data;

void initialize_bluetooth();
void send_name_request(esp_bd_addr_t mac_addr);
void store_measurement(bool isHome);
void initialize_presence_detection();
void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
void initialize_timer(timer_group_t group, timer_idx_t timer, bool auto_reload, int timer_interval);
void presence_detection_loop();
void reset_results();
void upload_presence_detection_data();
char *result_to_string(presence_data data);
char *results_to_rssi_list();
void presence_addr_to_string(presence_data data, char *buffer, int buffer_size);
void stop_requesting();
void start_requesting();

#endif