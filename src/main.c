/* Wi-Fi Provisioning Manager Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "main.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_HTTP_RECV_BUFFER 512

static const char *TAG = "Twomes Generic Firmware";

const char* rootCAR3 = "-----BEGIN CERTIFICATE-----\n" \
                       "MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
                       "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
                       "DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
                       "MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
                       "AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
                       "jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
                       "Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
                       "U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
                       "gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
                       "/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
                       "oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
                       "BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
                       "ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
                       "p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
                       "AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
                       "Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
                       "LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
                       "r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
                       "AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
                       "ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
                       "S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
                       "qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
                       "O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
                       "UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
                       "-----END CERTIFICATE-----\n";

const char* rootCert = "-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

const char* rootKey = "-----BEGIN ENCRYPTED PRIVATE KEY-----\n" \
"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\n" \
"HwtaqlZ4AiuvIQBcFzyVmA==\n" \
"-----END ENCRYPTED PRIVATE KEY-----\n";

// extern const char certificate_start[] asm("_binary_certificate_pem_start");
// extern const char certificate_end[]   asm("_binary_certificate_pem_end");

char *https_url = "192.168.178.75:4444/set/house/opentherm";

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
    settimeofday(tv, NULL);
    ESP_LOGI(TAG, "Time is synchronized from custom code");
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

/* Event handler for catching system events */
static void prov_event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    }
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                    output_len = 0;
                }
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        default:
            ESP_LOGI(TAG, "GOT AN ERROR BUT DONT KNOW WHAT!");
            break;
    }
    return ESP_OK;
}

static void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf)
    {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

static void obtain_time(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // ESP_ERROR_CHECK(example_connect());

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    // ESP_ERROR_CHECK( example_disconnect() );
}

static void https(char *data)
{
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request
    int content_length = 0;
    esp_http_client_config_t config = {
        .url = "https://192.168.178.75:4444/set/house/opentherm",
        // .host = "192.168.178.75",
        // .port = 4444,
        // .path = "/set/house/opentherm",
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        // .client_cert_pem = rootCert,
        // .client_key_pem = rootKey,
        .event_handler = http_event_handler
        };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // esp_err_t err = esp_http_client_connect(client);
    // ESP_LOGI(TAG, "First perform succesful!");
    // if (err == ESP_OK) {
    //     ESP_LOGI(TAG, "Finished performing for the first time?");
    //     ESP_LOGI(TAG, "Status = %d, content_length = %d",
    // }
    // esp_http_client_get_status_code(client),
    // esp_http_client_get_content_length(client));

    // second request
    esp_http_client_set_url(client, "/set/house/opentherm");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Host", "energietransitiewindesheim.nl:4444");
    esp_http_client_set_header(client, "Content-Type", "text/plain");
    ESP_LOGI(TAG, "Made it past headers!!");
    esp_http_client_set_post_field(client, data, strlen(data));
    ESP_LOGI(TAG, "DATA BE LIKE: %s %d", data, strlen(data));
    esp_err_t err = esp_http_client_perform(client);
    ESP_LOGI(TAG, "Made it past open!!");
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Made it past else!!");
        int wlen = esp_http_client_write(client, data, strlen(data));
        ESP_LOGI(TAG, "Made it past else write!!");
        if (wlen < 0)
        {
            ESP_LOGE(TAG, "Write failed");
        }
        // int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
        // ESP_LOGI(TAG, "Made it past else read!!");
        // if (data_read >= 0)
        // {
        //     ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
        //              esp_http_client_get_status_code(client),
        //              esp_http_client_get_content_length(client));
        //     ESP_LOG_BUFFER_HEX(TAG, output_buffer, strlen(output_buffer));
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "Failed to read response");
        // }
    }
    esp_http_client_cleanup(client);
}

wifi_prov_mgr_config_t initialize_provisioning(){
       /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &prov_event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
    esp_netif_create_default_wifi_ap();
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
    /* What is the Provisioning Scheme that we want ?
         * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
        .scheme = wifi_prov_scheme_ble,
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
        .scheme = wifi_prov_scheme_softap,
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */

    /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */
#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP
                                    .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP */
    };
    return config;
}

void start_provisioning(wifi_prov_mgr_config_t config){
/* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    /* If device is not yet provisioned start provisioning service */
    if (!provisioned)
    {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        /* What is the security level that we want (0 or 1):
         *      - WIFI_PROV_SECURITY_0 is simply plain text communication.
         *      - WIFI_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_0;

        /* Do we want a proof-of-possession (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        const char *pop = "abcd1234";

        /* What is the service key (could be NULL)
         * This translates to :
         *     - Wi-Fi password when scheme is wifi_prov_scheme_softap
         *     - simply ignored when scheme is wifi_prov_scheme_ble
         */
        const char *service_key = NULL;

#ifdef CONFIG_EXAMPLE_PROV_TRANSPORT_BLE
        /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
         * set a custom 128 bit UUID which will be included in the BLE advertisement
         * and will correspond to the primary GATT service that provides provisioning
         * endpoints as GATT characteristics. Each GATT characteristic will be
         * formed using the primary service UUID as base, with different auto assigned
         * 12th and 13th bytes (assume counting starts from 0th byte). The client side
         * applications must identify the endpoints by reading the User Characteristic
         * Description descriptor (0x2901) for each characteristic, which contains the
         * endpoint name of the characteristic */
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4,
            0xdf,
            0x5a,
            0x1c,
            0x3f,
            0x6b,
            0xf4,
            0xbf,
            0xea,
            0x4a,
            0x82,
            0x03,
            0x04,
            0x90,
            0x1a,
            0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
#endif /* CONFIG_EXAMPLE_PROV_TRANSPORT_BLE */

        /* An optional endpoint that applications can create if they expect to
         * get some additional custom data during provisioning workflow.
         * The endpoint name can be anything of your choice.
         * This call must be made before starting the provisioning.
         */
        wifi_prov_mgr_endpoint_create("custom-data");
        /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));

        /* The handler for the optional endpoint created above.
         * This call must be made after starting the provisioning, and only if the endpoint
         * has already been created above.
         */
        wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);

        /* Uncomment the following to wait for the provisioning to finish and then release
         * the resources of the manager. Since in this case de-initialization is triggered
         * by the default event loop handler, we don't need to call the following */
        // wifi_prov_mgr_wait();
        // wifi_prov_mgr_deinit();
    }
    else
    {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();

        /* Start Wi-Fi station */
        wifi_init_sta();
    }

}

void initialize_nvs(){
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

void app_main(void)
{
    initialize_nvs();

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    wifi_prov_mgr_config_t config = initialize_provisioning();
    start_provisioning(config);

    
    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
    
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    char strftime_buf[64];
    // Set timezone to Eastern Standard Time and print local time
    setenv("TZ", "Europe/Amsterdam,M3.2.0/2,M11.1.0", 0);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Amsterdam is: %s", strftime_buf);

    /* Start main application now */
    while (1)
    {
        ESP_LOGI(TAG, "Hello World!");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        https("{\"deviceMac\":\"8C:AA:B5:85:A2:3D\",\"measurements\": [{\"property\":\"testy\",\"value\":\"hello_world\"}],\"time\":7}");
    }
}
