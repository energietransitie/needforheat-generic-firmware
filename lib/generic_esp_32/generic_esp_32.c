#include "generic_esp_32.h"

static const char *TAG = "Twomes Generic Firmware Library ESP32";
bool activation = false;

static EventGroupHandle_t wifi_event_group;

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;

const char *isrgrootx1 = "-----BEGIN CERTIFICATE-----\n"
                        "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
                        "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
                        "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
                        "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
                        "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
                        "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
                        "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
                        "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
                        "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
                        "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
                        "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
                        "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
                        "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
                        "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
                        "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
                        "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
                        "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
                        "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
                        "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
                        "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
                        "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
                        "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
                        "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
                        "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
                        "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
                        "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
                        "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
                        "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
                        "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
                        "-----END CERTIFICATE-----\n";

bool wifi_initialized = false;
bool wifi_autoconnect = true;
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
    settimeofday(tv, NULL);
    ESP_LOGI(TAG, "Time is synchronized from custom code");
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

#ifndef CONFIG_TWOMES_CUSTOM_GPIO
//Interrupt Queue Handler:
static xQueueHandle gpio_evt_queue = NULL;

//Gpio ISR handler:
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
} //gpio_isr_handler

//Function to initialise the buttons and LEDs on the gateway, with interrupts on the buttons
void initGPIO()
{
    gpio_config_t io_conf;
    //CONFIGURE OUTPUTS:
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = OUTPUT_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    //CONFIGURE INPUTS:
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = INPUT_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

/**
 * Check for input of buttons and the duration
 * if the press duration was more than 5 seconds, erase the flash memory to restart provisioning
 * otherwise, blink the status LED (and possibly run another task (sensor provisioning?))
*/
void buttonPressDuration(void *args)
{
    uint32_t io_num;
    ESP_LOGI(TAG, "Button Press Duration is Here!");
    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            uint8_t seconds = 0;
            while (!gpio_get_level(WIFI_RESET_BUTTON))
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                seconds++;
                if (seconds == 9)
                {
                    ESP_LOGI("ISR", "Button held for over 10 seconds\n");
                    char blinkArgs[2] = {5, LED_ERROR};
                    xTaskCreatePinnedToCore(blink, "blink longpress", 768, (void *)blinkArgs, 10, NULL, 1);
                    //Long press on WIFI_RESET_BUTTON(BOOT on the esp32) is for clearing Wi-Fi provisioning memory:
                    ESP_LOGI("ISR", "Resetting Provisioning and Restarting Device!");
                    esp_wifi_restore();
                    vTaskDelay(1000 / portTICK_PERIOD_MS); //Wait for blink to finish
                    esp_restart();                         //software restart, to get new provisioning. Sensors do NOT need to be paired again when gateway is reset (MAC address does not change)
                    break;                                 //Exit loop
                }
            }
        }
    }
}
#endif

/**Blink LEDs to test GPIO:
 * Pass two arguments in uint8_t array:
 * argument[0] = amount of blinks
 * argument[1] = pin to blink on (LED_STATUS or LED_ERROR)
 */
void blink(void *args)
{
    uint8_t *arguments = (uint8_t *)args;
    uint8_t amount = arguments[0];
    uint8_t pin = arguments[1];
    uint8_t i;
    for (i = 0; i < amount; i++)
    {
        gpio_set_level(pin, 1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(pin, 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    } //for(i<amount)
    //Delete the blink task after completion:
    vTaskDelete(NULL);
} //void blink;

void initialize()
{
    ESP_LOGI(TAG, "Generic Firmware Version: %s", VERSION);

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();
#ifndef CONFIG_TWOMES_CUSTOM_GPIO
    initGPIO();
    //Attach interrupt handler to GPIO pins:
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreatePinnedToCore(buttonPressDuration, "buttonPressDuration", 2048, NULL, 10, NULL, 1);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(WIFI_RESET_BUTTON, gpio_isr_handler, (void *)WIFI_RESET_BUTTON);
#endif
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

char *get_types(char *stringf, int count)
{
    char *stringfBuffer = stringf;
    char c = *stringf;
    char *types = malloc(sizeof(char) * count);
    for (int i = 0; i < strlen(stringf); i++)
    {
        c = *stringfBuffer;
        if (c == '%')
        {
            char *tmp = ++stringfBuffer;
            memcpy(types, tmp, sizeof(char));
            types += sizeof(char);
        }
        else
            stringfBuffer++;
    }
    types -= count * sizeof(char);
    return types;
}

int variable_sprintf_size(char *string, int count, ...)
{
    va_list list;
    va_start(list, count);
    int extraSize = 0;
    char *snBuf = "";
    char *types = get_types(string, count);
    for (int i = 0; i < count; i++)
    {
        char type = *types++;
        switch (type)
        {
        case 'd':
            // int tmp = va_arg(list, int);
            extraSize += snprintf(snBuf, 0, "%d", va_arg(list, int));
            break;
        case 'u':
            extraSize += snprintf(snBuf, 0, "%u", va_arg(list, u32_t));
            break;
        case 's':
            extraSize += snprintf(snBuf, 0, "%s", va_arg(list, char *));
            break;
        }
    }
    int totalSize = strlen(string) * sizeof(char) + sizeof(char) * extraSize;
    return totalSize;
}

/* Event handler for catching system events */
void prov_event_handler(void *arg, esp_event_base_t event_base,
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
        ESP_LOGI(TAG, "Disconnected.");
        if (wifi_autoconnect)
        {
            ESP_LOGI(TAG, " Connecting to the AP again...");
            esp_wifi_connect();
        }
    }
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
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
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            if (evt->user_data)
            {
                memcpy(evt->user_data + output_len, evt->data, evt->data_len);
            }
            else
            {
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
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
        if (output_buffer != NULL)
        {
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
        if (err != 0)
        {
            if (output_buffer != NULL)
            {
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

void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void create_dat()
{
    esp_err_t err;
    uint32_t dat;
    nvs_handle_t dat_handle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &dat_handle);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGE(TAG, "Succesfully opened NVS twomes_storage!");
        err = nvs_get_u32(dat_handle, "dat", &dat);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "The dat was initialized already!\n");
            ESP_LOGI(TAG, "The dat is: %u\n", dat);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "The dat is not initialized yet!");
            ESP_LOGI(TAG, "Creating dat");
            dat = esp_random();
            ESP_LOGI(TAG, "Attempting to store dat: %d", dat);
            err = nvs_set_u32(dat_handle, "dat", dat);
            if (!err)
            {
                ESP_LOGI(TAG, "Succesfully wrote dat: %u to NVS twomes_storage", dat);
            }
            else
            {
                ESP_LOGE(TAG, "Failed to write dat to NVS twomes_storage: %s", esp_err_to_name(err));
            }
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        nvs_close(dat_handle);
    }
    ESP_LOGI(TAG, "dat: %u", dat);
}

void get_dat(uint32_t *buf)
{
    esp_err_t err;
    nvs_handle_t dat_handle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &dat_handle);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGE(TAG, "Succesfully opened NVS twomes_storage!");
        err = nvs_get_u32(dat_handle, "dat", buf);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "The dat has succesfully been read!\n");
            break;
        default:
            ESP_LOGE(TAG, "%s", esp_err_to_name(err));
            break;
        }
    }
}

void prepare_device(const char *device_type_name)
{
    if (wifi_initialized)
    {
        ESP_LOGI(TAG, "Wi-Fi has been enabled for true random dat generation!");
        create_dat();
    }
    else
    {
        ESP_LOGI(TAG, "Wi-Fi has not been enabled for true random dat generation, enabling Wi-Fi!");
        enable_wifi();
        while (!wifi_initialized)
        {
            ESP_LOGI(TAG, "Waiting for Wi-Fi enable to finish.");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        };
        ESP_LOGI(TAG, "Disabling Wi-Fi again as to not disturb provisioning.");
        disable_wifi();
    }
    uint32_t dat;
    get_dat(&dat);
    char *device_name = malloc(DEVICE_NAME_SIZE);
    get_device_service_name(device_name, DEVICE_NAME_SIZE);
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
    char *qr_code_payload_template = "\n\n{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%u\",\"transport\":\"ble\"}\n\n";
    int qr_code_payload_size = variable_sprintf_size(qr_code_payload_template, 2, device_name, dat);
    char *qr_code_payload = malloc(qr_code_payload_size);
    snprintf(qr_code_payload, qr_code_payload_size, qr_code_payload_template, device_name, dat);
#endif
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
    char *qr_code_payload_template = "\n\n{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%u\",\"transport\":\"ble\",\"security\":\"1\",\"password\":\"%s\"}\n\n";
    int qr_code_payload_size = variable_sprintf_size(qr_code_payload_template, 2, device_name, dat, dat);
    char *qr_code_payload = malloc(qr_code_payload_size);
    snprintf(qr_code_payload, qr_code_payload_size, qr_code_payload_template, device_name, dat, dat);
#endif

    ESP_LOGI(TAG, "QR Code Payload: ");
    ESP_LOGI(TAG, "%s", qr_code_payload);
    free(qr_code_payload);

    char *post_device_payload_template = "\n\n{\"name\":\"%s\",\"device_type\":\"%s\",\"activation_token\":\"%u\"}\n\n";
    int post_device_payload_size = variable_sprintf_size(post_device_payload_template, 3, device_name, device_type_name, dat);
    char *post_device_payload = malloc(post_device_payload_size);
    snprintf(post_device_payload, post_device_payload_size, post_device_payload_template, device_name, device_type_name, dat);

    ESP_LOGI(TAG, "POST /device payload: ");
    ESP_LOGI(TAG, "%s", post_device_payload);
    free(post_device_payload);
    free(device_name);
}

void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = SSID_PREFIX;
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

void initialize_sntp(void)
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

void obtain_time(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */

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
}

void initialize_time(char *timezone)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to Wi-Fi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    char strftime_buf[64];
    // Set timezone
    setenv("TZ", timezone, 0);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current UTC/date/time is: %s", strftime_buf);
}

void upload_heartbeat(const char *variable_interval_upload_url, const char *root_cert, char *bearer)
{
    char *measurementType = "\"heartbeat\"";
    //Updates Epoch Time
    time_t now = time(NULL);
    //Plain JSON request where values will be inserted.
    char *msg_plain = "{\"upload_time\": \"%d\",\"property_measurements\":[    {"
                      "\"property_name\": %s,"
                      "\"measurements\": ["
                      "{ \"timestamp\":\"%d\","
                      "\"value\":\"1\"}"
                      "]}]}";
    //Get size of the message after inputting variables.
    int msgSize = variable_sprintf_size(msg_plain, 3, now, measurementType, now);
    //Allocating enough memory so inputting the variables into the string doesn't overflow
    char *msg = malloc(msgSize);
    //Inputting variables into the plain json string from above(msgPlain).
    snprintf(msg, msgSize, msg_plain, now, measurementType, now);
    //Posting data over HTTPS, using url, msg and bearer token.
    ESP_LOGI(TAG, "Data: %s", msg);
    post_https(variable_interval_upload_url, msg, root_cert, bearer, NULL, 0);
}

esp_err_t store_bearer(char *bearer)
{
    esp_err_t err;
    nvs_handle_t bearer_handle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &bearer_handle);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGE(TAG, "Succesfully opened NVS twomes_storage!");
        err = nvs_set_str(bearer_handle, "bearer", bearer);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "The bearer was written!\n");
            ESP_LOGI(TAG, "The bearer is: %s\n", bearer);
            break;
        default:
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
        }
        err = nvs_commit(bearer_handle);
        nvs_close(bearer_handle);
    }
    return err;
}

char *get_bearer()
{
    esp_err_t err;
    char *bearer = NULL;
    nvs_handle_t bearer_handle;
    err = nvs_open("twomes_storage", NVS_READONLY, &bearer_handle);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGE(TAG, "Succesfully opened NVS twomes_storage!");
        size_t bearer_size;
        nvs_get_str(bearer_handle, "bearer", NULL, &bearer_size);
        bearer = malloc(bearer_size);
        err = nvs_get_str(bearer_handle, "bearer", bearer, &bearer_size);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "The bearer was read!\n");
            ESP_LOGI(TAG, "The bearer is: %s\n", bearer);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "The bearer has not been initialized yet!");
            bearer = "";
            break;
        default:
            bearer = NULL;
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
        }
        nvs_close(bearer_handle);
    }
    return bearer;
}

const char *get_root_ca()
{
    // Twomes servers at *.energietransitiewindesheim.nl use Let's Encrypt certificates
    // based on https://letsencrypt.org/docs/dst-root-ca-x3-expiration-september-2021/
    // we use the ISRG Root X1 certificate found at https://letsencrypt.org/certs/isrgrootx1.pem
    // this certificate was translated to ESP32 code 
    // by adding " at the befinning each line and \n" at the end of each line in the code
    return isrgrootx1;
}

void activate_device(const char *url, char *name, const char *cert)
{
    esp_err_t err;
    uint32_t dat;
    get_dat(&dat);
    activation = true;
    char *device_activation_plain = "{ \"activation_token\":\"%u\"}";
    int activation_data_size = variable_sprintf_size(device_activation_plain, 1, dat);
    char *device_activation_data = malloc(activation_data_size);
    snprintf(device_activation_data, activation_data_size, device_activation_plain, dat);

    ESP_LOGI(TAG, "%s", device_activation_data);
    char *bearer = malloc(sizeof(char)*MAX_RESPONSE_LENGTH);
    ESP_LOGI(TAG, "GOT HERE!"); 
    post_https(url, device_activation_data, cert, NULL, bearer, MAX_RESPONSE_LENGTH);
    ESP_LOGI(TAG, "Got Here!");
    if (!bearer)
    {
        ESP_LOGE(TAG, "Failed to activate device!");
    }
    else
    {
        ESP_LOGE(TAG, "Bearer after post is: %s", bearer);
        int size = strlen(bearer) * sizeof(char);
        char *bearer_trimmed = malloc(size);
        char c = *bearer;
        int count = 0;
        int length = 0;
        bool done = false;
        while (!done)
        {
            switch (c)
            {
            case '"':
                count++;
                if (count == 4)
                {
                    *bearer_trimmed = '\0';
                    done = true;
                }
                break;
            default:
                if (count == 3)
                {
                    *bearer_trimmed++ = c;
                    length++;
                }
            }
            ESP_LOGI(TAG, "Loopsies! %c", c);
            c = *++bearer;
        }
        bearer_trimmed -= length;
        ESP_LOGI(TAG, "Bearer Trimmed: %s", bearer_trimmed);
        err = store_bearer(bearer_trimmed);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
        }
    }
}


int post_https(const char *url, char *data, const char *cert, char *authenticationToken, char* response_buf, uint8_t resp_buf_size)
{
    int content_length;
    int status_code = 0;
    char *response = NULL;
    esp_http_client_config_t config = {
        .url = url,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = cert,
        .event_handler = http_event_handler};
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Host", TWOMES_TEST_SERVER_HOSTNAME);
    esp_http_client_set_header(client, "Content-Type", "text/plain");
    char *authenticationTokenString = "";
    if (authenticationToken)
    {
        char *authenticationTokenStringPlain = "Bearer %s";
        int strCount = snprintf(authenticationTokenString, 0, authenticationTokenStringPlain, authenticationToken) + 1;
        authenticationTokenString = malloc(sizeof(char) * strCount);
        snprintf(authenticationTokenString, strCount * sizeof(char), authenticationTokenStringPlain, authenticationToken);
        esp_http_client_set_header(client, "Authorization", authenticationTokenString);
    }
    ESP_LOGI(TAG, "Got Here 1");
    esp_http_client_set_post_field(client, data, strlen(data));
    esp_err_t err = esp_http_client_perform(client);
    ESP_LOGI(TAG, "Got Here 2");
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    else
    {
        status_code = esp_http_client_get_status_code(client);
        content_length = esp_http_client_get_content_length(client);
        if (content_length > 0)
        {
            ESP_LOGE(TAG, "Status Code: %d Response Length: %d", status_code,
                     content_length);
            response = malloc(sizeof(char) * content_length);
            esp_http_client_read(client, response, content_length);
            ESP_LOGE(TAG, "Response: %s", response);
        }
        else
        {
            ESP_LOGE(TAG, "No proper response, response length: %d status_code: %d", content_length, status_code);
        }
    }
    if (authenticationToken)
    {
        free(authenticationTokenString);
    }
    free(data);
    esp_http_client_cleanup(client);
    if (response&&resp_buf_size)
    {
        int missed = snprintf(response_buf, resp_buf_size, "%s", response);
        if(missed > resp_buf_size){
            ESP_LOGE(TAG, "Buffer was too small, full string was not written. Missed %d amount of character space!", missed-resp_buf_size);
        }
    }
    return status_code;
}

wifi_prov_mgr_config_t initialize_provisioning()
{
    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &prov_event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    wifi_initialized = true;
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
    esp_netif_create_default_wifi_ap();
#endif /* CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
    /* What is the Provisioning Scheme that we want ?
         * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
        .scheme = wifi_prov_scheme_ble,
#endif /* CONFIG_TWOMES_PROV_TRANSPORT_BLE */
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
        .scheme = wifi_prov_scheme_softap,
#endif /* CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP */

    /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#endif /* CONFIG_TWOMES_PROV_TRANSPORT_BLE */
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
                                    .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
#endif /* CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP */
    };
    return config;
}

void start_provisioning(wifi_prov_mgr_config_t config, bool connect)
{
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
        char *service_name = malloc(DEVICE_NAME_SIZE);
        get_device_service_name(service_name, DEVICE_NAME_SIZE);
        /* What is the security level that we want (0 or 1):
         *      - WIFI_PROV_SECURITY_0 is simply plain text communication.
         *      - WIFI_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and device.activaton_token (dat) and AES-CTR
         *          for encryption/decryption of messages.
         */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        /* Do we want a device.activaton_token (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        uint32_t dat;
        get_dat(&dat);
        int msgSize = variable_sprintf_size("%u", 1, dat);
        char *dat_str = malloc(msgSize);
        //Inputting variables into the plain json string from above(msgPlain).
        snprintf(dat_str, msgSize, "%u", dat);

        /* What is the service key (could be NULL)
         * This translates to :
         *     - Wi-Fi password when scheme is wifi_prov_scheme_softap
         *     - simply ignored when scheme is wifi_prov_scheme_ble
         */
        char *service_key = dat_str;

#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
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
#endif /* CONFIG_TWOMES_PROV_TRANSPORT_BLE */

        /* An optional endpoint that applications can create if they expect to
         * get some additional custom data during provisioning workflow.
         * The endpoint name can be anything of your choice.
         * This call must be made before starting the provisioning.
         */
        // wifi_prov_mgr_endpoint_create("custom-data");
        /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, dat_str, service_name, service_key));

        /* The handler for the optional endpoint created above.
         * This call must be made after starting the provisioning, and only if the endpoint
         * has already been created above.
         */
        // wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);

        /* Uncomment the following to wait for the provisioning to finish and then release
         * the resources of the manager. Since in this case de-initialization is triggered
         * by the default event loop handler, we don't need to call the following */
        // wifi_prov_mgr_wait();
        // wifi_prov_mgr_deinit();
    }
    else
    {
        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();
        if (connect)
        {
            ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
            /* Start Wi-Fi station */
            wifi_init_sta();
        }
        else
        {
            ESP_LOGI(TAG, "Already provisioned, not starting Wi-Fi because connecting is disabled");
        }
    }
    /* Wait for Wi-Fi connection */
    if (connect)
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
}

void disable_wifi()
{
    if (esp_wifi_stop() == ESP_OK)
    {
        ESP_LOGI(TAG, "Disabled Wi-Fi");
        wifi_initialized = false;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to disable Wi-Fi");
    }
}

void enable_wifi()
{
    if (esp_wifi_start() == ESP_OK)
    {
        ESP_LOGI(TAG, "Enabled Wi-Fi");
        wifi_initialized = true;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to enable Wi-Fi");
    }
}

void disconnect_wifi()
{
    wifi_autoconnect = false;
    esp_err_t err = esp_wifi_disconnect();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to disconnect Wi-Fi: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Disconnected Wi-Fi");
    }
}

void connect_wifi()
{
    esp_err_t err = esp_wifi_connect();
    wifi_autoconnect = true;
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Succesfully connected Wi-Fi");
    }
}

void initialize_nvs()
{
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
