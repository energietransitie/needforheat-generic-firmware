#ifndef _GENERIC_ESP_32_H
#define _GENERIC_ESP_32_H

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
#include <driver/gpio.h>

#include <wifi_provisioning/manager.h>
#define VERSION "V1.7.2"
#define WIFI_RESET_BUTTON   GPIO_NUM_0
#define LED_ERROR   GPIO_NUM_19
#define MAX_RESPONSE_LENGTH 100

#define SSID_PREFIX "TWOMES-"
#define DEVICE_NAME_SIZE 14 /*SSID_PREFIX will be appended with six hexadecimal digits derived from the last 48 bits of the MAC address */ 

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_HTTP_RECV_BUFFER 512

#define LONG_BUTTON_PRESS_DURATION 10

#define TWOMES_TEST_SERVER_HOSTNAME "api.tst.energietransitiewindesheim.nl"
#define TWOMES_TEST_SERVER "https://api.tst.energietransitiewindesheim.nl"

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
char* get_types(char* stringf, int count);
int variable_sprintf_size(char* string, int count, ...);
void initialize();
void create_dat();
void prepare_device(const char *device_type_name);
void time_sync_notification_cb(struct timeval *tv);
void prov_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);
esp_err_t http_event_handler(esp_http_client_event_t *evt);
void wifi_init_sta(void);
void get_device_service_name(char *service_name, size_t max);
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data);
void initialize_sntp(void);
void obtain_time(void);
void initialize_time(char* timezone);
int post_https(const char *url, char *data, const char *cert, char *authenticationToken, char* response_buf, uint8_t resp_buf_size);
void upload_heartbeat(const char* variable_interval_upload_url, const char* root_cert, char* bearer);
char* get_bearer();
const char* get_root_ca();
void activate_device(const char *url, char *name,const char *cert);
void get_http(const char* url);

void initialize_nvs();

wifi_prov_mgr_config_t initialize_provisioning();
void start_provisioning(wifi_prov_mgr_config_t config, bool connect);
void disable_wifi();
void enable_wifi();
void disconnect_wifi();
void connect_wifi();
#endif