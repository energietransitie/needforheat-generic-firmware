#ifndef GENERIC_ESP_32_H
#define GENERIC_ESP_32_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_attr.h>
#include <esp_sleep.h>
#include <esp_sntp.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <esp_tls.h>
#include <esp_http_client.h>
#define HTTPSTATUS_OK 200 //change everywhere in code to HttpStatus_Ok when including esp_http_client.h library of 19 Nov 2020 or later
#include <driver/gpio.h>

#include <wifi_provisioning/manager.h>

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
#include "presence_detection.h"
#endif

#define VERSION "V2.5.0"
#define WIFI_RESET_BUTTON   GPIO_NUM_0
#define LED_ERROR   GPIO_NUM_19
#define MAX_RESPONSE_LENGTH 100

#define SSID_PREFIX "TWOMES-"
#define DEVICE_NAME_SIZE 14 /*SSID_PREFIX will be appended with six hexadecimal digits derived from the last 48 bits of the MAC address */ 

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_HTTP_RECV_BUFFER 512

#define LONG_BUTTON_PRESS_DURATION 10 // seconds

#define HTTPS_PRE_WAIT_MS (100) // milliseconds
#define HTTPS_RETRY_WAIT_MS (2 * 1000) // milliseconds ( 2 s * 1000 ms/s)  
#define HTTPS_POST_WAIT_MS (100) // milliseconds
#define HTTPS_UPLOAD_RETRIES 10 // number of retries including initial try
#define WIFI_CONNECT_RETRIES 10 // number of retries including initial try
#define NTP_RETRIES 10 // // number of retries for timesync including initial try

#ifdef CONFIG_TWOMES_STRESS_TEST
#define HEARTBEAT_UPLOAD_INTERVAL_MS (1 * 60  * 1000) // milliseconds ( 1 min * 60 s/min * 1000 ms/s) // stress test value
#define HEARTBEAT_MEASUREMENT_INTERVAL_MS HEARTBEAT_UPLOAD_INTERVAL_MS
#define HEARTBEAT_MEASUREMENT_INTERVAL_TXT "Wating 1 minute for next heartbeat"
#define TIMESYNC_INTERVAL_MS (4 * 60 * 1000) // milliseconds (4 min * 60 s/min * 1000 ms/s)  // stress test value 
#define TIMESYNC_INTERVAL_TXT "Wating 4 minutes before next NTP timesync"
#define MAX_WAIT_802_11_MS (30 * 1000) // milliseconds ( 30 s * 1000 ms/s)
#define MAX_WAIT_802_11_TXT "30 seconds"
#else
#define HEARTBEAT_MEASUREMENT_INTERVAL_MS (10 * 60 * 1000) // milliseconds ( 10 min * 60 s/min * 1000 ms/s)
#define HEARTBEAT_UPLOAD_INTERVAL_MS (10 * 60 * 1000) // milliseconds ( 10 min * 60 s/min * 1000 ms/s)
#define HEARTBEAT_MEASUREMENT_INTERVAL_TXT "Wating 10 minutes for next heartbeat"
#define TIMESYNC_INTERVAL_MS (6 *60 * 60 * 1000) // milliseconds (6 hr * 60 min/hr * 60 s/min * 1000 ms/s)   
#define TIMESYNC_INTERVAL_TXT "Wating 6 hours before next NTP timesync"
#define MAX_WAIT_802_11_MS (30 * 1000) // milliseconds ( 30 s * 1000 ms/s)
#define MAX_WAIT_802_11_TXT "30 seconds"
#endif

xSemaphoreHandle wireless_802_11_mutex;

#ifdef CONFIG_TWOMES_TEST_SERVER
#define TWOMES_SERVER_HOSTNAME "api.tst.energietransitiewindesheim.nl"
#define TWOMES_SERVER "https://api.tst.energietransitiewindesheim.nl"
#endif

#ifdef CONFIG_TWOMES_PRODUCTION_SERVER
#define TWOMES_SERVER_HOSTNAME "api.energietransitiewindesheim.nl"
#define TWOMES_SERVER "https://api.energietransitiewindesheim.nl"
#endif

#define VARIABLE_UPLOAD_ENDPOINT "/device/measurements/variable-interval"
#define FIXED_INTERVAL_UPLOAD_ENDPOINT "/device/measurements/fixed-interval"
#define DEVICE_ACTIVATION_ENDPOINT "/device/activate"

#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
#include <wifi_provisioning/scheme_ble.h>
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */

#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
#include <wifi_provisioning/scheme_softap.h>
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv);
#endif


#ifndef CONFIG_TWOMES_CUSTOM_GPIO
#define OUTPUT_BITMASK ((1ULL<<LED_ERROR))
#define INPUT_BITMASK ((1ULL << WIFI_RESET_BUTTON))

void initGPIO();
void buttonPressDuration(void *args);
#endif
void blink(void *args);
char *get_types(char *stringf, int count);
int variable_sprintf_size(char *string, int count, ...);
void initialize_generic_firmware();
void create_dat();
void prepare_device(const char *device_type_name);
void time_sync_notification_cb(struct timeval *tv);
void prov_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
esp_err_t http_event_handler(esp_http_client_event_t *evt);
void wifi_init_sta(void);
void get_device_service_name(char *service_name, size_t max);
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
    uint8_t **outbuf, ssize_t *outlen, void *priv_data);
void initialize_sntp(void);
void obtain_time(void);
void timesync_task(void *data);
#define ALREADY_CONNECTED true
#define NOT_ALREADY_CONNECTED false
void timesync(bool already_connected);
void initialize_timezone(char *timezone);
#define POST_WITH_BEARER true
#define POST_WITHOUT_BEARER false
int upload_data_to_server(char *endpoint, bool use_bearer, char *data, char *response_buf, uint8_t resp_buf_size);
int post_https(char *endpoint, bool use_bearer, bool already_connected, char *data, char *response_buf, uint8_t resp_buf_size);
void upload_heartbeat(int hbcounter);
void heartbeat_task(void *data);
char *get_bearer();
void activate_device();
void get_http(const char *url);
#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
void start_presence_detection();
#endif

void initialize_nvs();

wifi_prov_mgr_config_t initialize_provisioning();
void start_provisioning(wifi_prov_mgr_config_t config, bool connect);
void twomes_device_provisioning(const char *device_type_name);
bool disable_wifi(char *taskString);
bool disable_wifi_keeping_802_11_mutex();
bool enable_wifi(char *taskString);
bool disconnect_wifi(char *taskString);
bool connect_wifi(char *taskString);
bool connect_wifi_having_802_11_mutex();

#endif