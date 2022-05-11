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

#define VERSION "V2.7.0"
#define BOOT   GPIO_NUM_0
#define RED_LED_ERROR   GPIO_NUM_19
#define LONG_BUTTON_PRESS_DURATION 19 // (10 s * 2 halfseconds - 1); this constant specifies the number of half seconds minus one to wait

#define MAX_RESPONSE_LENGTH 100

#define SSID_PREFIX "TWOMES-"
#define DEVICE_NAME_SIZE 14 /*SSID_PREFIX will be appended with six hexadecimal digits derived from the last 48 bits of the MAC address */ 

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_HTTP_RECV_BUFFER 512

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
#endif // CONFIG_TWOMES_STRESS_TEST

extern xSemaphoreHandle wireless_802_11_mutex;

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
#define OUTPUT_BITMASK ((1ULL<<RED_LED_ERROR))
#define INPUT_BITMASK ((1ULL << BOOT))
/**
 * Initialize the buttons and LEDs on the device, with interrupts on the buttons.
 */
void initGPIO();

/**
 * Check for input of buttons and the duration.
 * 
 * If the press duration was more than 10 seconds, erase the flash memory to restart provisioning
 * otherwise, blink the status LED (and possibly run another task (sensor provisioning?))
*/
void buttonPressHandlerGeneric(void *args);
#endif // CONFIG_TWOMES_CUSTOM_GPIO

/**
 * Blink LEDs to test GPIO.
 * 
 * @param args Pass two arguments in uint8_t array:
 * args[0] = amount of blinks.
 * args[1] = pin to blink on (LED_STATUS or RED_LED_ERROR).
 */
void blink(void *args);

/**
 * TODO: Not yet able to reverse engineer.
 * 
 * @param stringf String
 * @param count Amount of bytes.
 * 
 * @returns Heap-allocated memory.
 */
char *get_types(char *stringf, int count);

/**
 * Calculate size of formatted string.
 * 
 * @param string Template for printing.
 * @param count Amount of bytes.
 * 
 * @returns The total size of the formatted string in bytes.
 */
int variable_sprintf_size(char *string, int count, ...);

/**
 * Initialize the firmware.
 * 
 * - Default event loop is created. 
 * - WiFi event group is created.
 * - Custom GPIO is configured if CONFIG_TWOMES_CUSTOM_GPIO is defined.
 */
void initialize_generic_firmware();

/**
 * Open NVS twomes_storage and read 'dat' or create new.
 * 
 * 'dat' is logged to the console.
 */
void create_dat();

/**
 * Open NVS twomes_storage and read 'dat'.
 * 
 * @param buf Pointer to where 'dat' should be saved.
 */
void get_dat(uint32_t *buf);

/**
 * Log device and provisioning details to console.
 * 
 * @param device_type_name String that holds the name of the device type.
 */
void prepare_device(const char *device_type_name);

void time_sync_notification_cb(struct timeval *tv);

void prov_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

esp_err_t http_event_handler(esp_http_client_event_t *evt);

void wifi_init_sta(void);

/**
 * Get device service name based on SSID_PREFIX and MAC-address.
 * 
 * @param service_name pointer where the name should be saved.
 * @param max max size of the service_name.
 */
void get_device_service_name(char *service_name, size_t max);

/**
 * TODO: Document use of this function.
 * 
 * @param session_id
 * @param inbuf
 * @param inlen
 * @param outbuf
 * @param outlen
 * @param priv_data
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
    uint8_t **outbuf, ssize_t *outlen, void *priv_data);

/**
 * Initialize SNTP
 */
void initialize_sntp(void);

/**
 * This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
 * 
 * Read "Establishing Wi-Fi or Ethernet Connection" section in
 * examples/protocols/README.md for more information about this function.
 */
void obtain_time(void);

/**
 * Task to sync time.
 * 
 * This task can be started with the FreeRTOS Task API.
 */
void timesync_task(void *data);

#define ALREADY_CONNECTED true
#define NOT_ALREADY_CONNECTED false
/**
 * Sync time by connecting to Wi-Fi and getting time over NTP.
 */
void timesync(bool already_connected);

void initialize_timezone(char *timezone);

#define POST_WITH_BEARER true

#define POST_WITHOUT_BEARER false

/**
 * Wrapper function for post_https.
 * 
 * @param endpoint Endpoint to send the request to.
 * @param use_bearer Use bearer token or not.
 * @param data Data to send in POST-request.
 * @param response_buf Pointer to buffer where the response should be saved.
 * @param resp_buf_size Size of the response buffer.
 * 
 * @returns Status code of the response.
 */
int upload_data_to_server(char *endpoint, bool use_bearer, char *data, char *response_buf, uint8_t resp_buf_size);

/**
 * Send a HTTPS POST-request.
 * 
 * @param endpoint Endpoint to send the request to.
 * @param use_bearer Use bearer token or not.
 * @param already_connected Signal if WiFi is connected or not.
 * @param data Data to send in POST-request.
 * @param response_buf Pointer to buffer where the response should be saved.
 * @param resp_buf_size Size of the response buffer.
 * 
 * @returns Status code of the response.
 */
int post_https(char *endpoint, bool use_bearer, bool already_connected, char *data, char *response_buf, uint8_t resp_buf_size);

/**
 * Prepare JSON request and send it to the backend server.
 */
void upload_heartbeat(int hbcounter);

/**
 * Task to send a heartbeat every HEARTBEAT_MEASUREMENT_INTERVAL_MS.
 * 
 * This task can be started with the FreeRTOS Task API.
 */
void heartbeat_task(void *data);

/**
 * Store beared token used for authenticating with the backend server in NVS storage.
 * 
 * @param activation_response Bearer token to store.
 * 
 * @returns esp_err_t to check for errors.
 */
esp_err_t store_bearer(char *activation_response);

/**
 * Override bearer token in NVS storage with empty string.
 */
void delete_bearer();

/**
 * Get bearer token from NVS storage.
 * 
 * @returns Bearer as a string.
 */
char *get_bearer();

/**
 * Activate the device to the backend server.
 */
void activate_device();

void get_http(const char *url);

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
/**
 * Start presence detection using the FreeRTOS Task API.
 */
void start_presence_detection();
#endif // CONFIG_TWOMES_PRESENCE_DETECTION

/**
 * Initialize NVS partition.
 * 
 * If there are no free pages, NVS flash is erased before init.
 */
void initialize_nvs();

/**
 * Create configuration for Espressif unified provisioning.
 * 
 * @returns WiFi provision manager configuration.
 */
wifi_prov_mgr_config_t initialize_provisioning();

/**
 * Start provisioning if device is not yet provisioned.
 * 
 * @param config Provisioning configuration.
 * @param connect Should the device connect to WiFi if it is already provisioned?
 */
void start_provisioning(wifi_prov_mgr_config_t config, bool connect);

/**
 * Check/intitialize firmware, NVS partition, device preparation, provisioning, time synchronization and device activation.
 */
void twomes_device_provisioning(const char *device_type_name);

/**
 * Disable WiFi and release 802.11 mutex.
 * 
 * @param taskString Name of the task. Used for logging.
 * 
 * @returns True if WiFi was successfully disabled.
 */
bool disable_wifi(char *taskString);

/**
 * Disabled Wi-Fi without releasing 802.11 mutex.
 * 
 * @returns True if WiFi was successfully disabled.
 */
bool disable_wifi_keeping_802_11_mutex();

/**
 * Enable WiFi.
 * 
 * @param taskString Name of the task. Used for logging.
 * 
 * @returns True if WiFi was successfully enabled.
 */
bool enable_wifi(char *taskString);

/**
 * Disconnect WiFi.
 * 
 * @param taskString Name of the task. Used for logging.
 * 
 * @returns True if WiFi was successfully enabled.
 */
bool disconnect_wifi(char *taskString);

/**
 * Connect to WiFi.
 * 
 * @param taskString Name of the task. Used for logging.
 * 
 * @returns True if WiFi was successfully enabled.
 */
bool connect_wifi(char *taskString);

/**
 * TODO: This function has no definition.
 */
bool connect_wifi_having_802_11_mutex();
void wait_for_wifi(char *endpoint);

#endif // GENERIC_ESP_32_H