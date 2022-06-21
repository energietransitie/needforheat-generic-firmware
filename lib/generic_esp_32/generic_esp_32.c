#include <generic_esp_32.h>

static const char *TAG = "Twomes Generic Firmware Library ESP32";

bool activation = false;

static EventGroupHandle_t wifi_event_group;

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;

char *bearer = NULL;

xSemaphoreHandle wireless_802_11_mutex;

// Twomes servers at *.energietransitiewindesheim.nl use Let's Encrypt certificates
// based on https://letsencrypt.org/docs/dst-root-ca-x3-expiration-september-2021/
// we use the ISRG Root X1 certificate found at https://letsencrypt.org/certs/isrgrootx1.pem
// this certificate was translated to ESP32 code
// by adding " at the befinning each line and \n" at the end of each line in the code

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
void sntp_sync_time(struct timeval *tv) {
    settimeofday(tv, NULL);
    ESP_LOGD(TAG, "Time is synchronized from custom code");
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

#ifndef CONFIG_TWOMES_CUSTOM_GPIO
//Interrupt Queue Handler:
static xQueueHandle gpio_evt_queue = NULL;

//Gpio ISR handler:
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
} //gpio_isr_handler

void initGPIO() {
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

void buttonPressHandlerGeneric(void *args) {
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            vTaskDelay(100 / portTICK_PERIOD_MS); //Debounce delay: nothing happens if user presses button for less than 100 ms = 0.1s

            //INTERRUPT HANDLER BOOT
            if (io_num == BOOT) {
                uint8_t halfSeconds = 0;
                //Determine length of button press before taking action
                while (!gpio_get_level(BOOT)) {
                    //Button BOOT is (still) pressed
                    vTaskDelay(500 / portTICK_PERIOD_MS); //Wait for 0.5s
                    halfSeconds++;
                    ESP_LOGD(TAG, "Button BOOT has been pressed for %u half-seconds now", halfSeconds);
                    if (halfSeconds >= LONG_BUTTON_PRESS_DURATION) {
                        //Long press on BOOT is for clearing Wi-Fi provisioning memory:
                        ESP_LOGI("ISR", "Long-button press detected on button BOOT; resetting Wi-Fi provisioning and restarting device");
                        char blinkArgs[2] = { 5, RED_LED_ERROR };
                        xTaskCreatePinnedToCore(blink, "blink_5_times_red", 768, (void *)blinkArgs, 10, NULL, 1);
                        esp_wifi_restore();
                        // also remove bearer to force re-activation
						delete_bearer();
                        vTaskDelay(5 * (200 + 200) / portTICK_PERIOD_MS); //Wait for blink to finish
                        esp_restart();                         //software restart, to enable linking to new Wi-Fi network.
                        break;                                 //Exit loop (this should not be reached)
                    }                                          //if (halfSeconds == 9)
                    else if (gpio_get_level(BOOT)) {
                        //Button BOOT is released
                        //Short press on BOOT not used for anything currently
                        ESP_LOGI("ISR", "Short button press detected on button BOOT");
                    }
                } //while(!gpio_level)
            }
        }     //if(xQueueReceive)
    }  //while(1)
} // buttonPressHandlerGEneric
#endif

void blink(void *args) {
    uint8_t *arguments = (uint8_t *)args;
    uint8_t amount = arguments[0];
    uint8_t pin = arguments[1];
    uint8_t i;
    for (i = 0; i < amount; i++) {
        gpio_set_level(pin, 1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(pin, 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    } //for(i<amount)
    //Delete the blink task after completion:
    vTaskDelete(NULL);
} //void blink;

void initialize_generic_firmware() {
    ESP_LOGD(TAG, "Generic Firmware Version: %s", VERSION);

    wireless_802_11_mutex = xSemaphoreCreateMutex();

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();
#ifndef CONFIG_TWOMES_CUSTOM_GPIO
    initGPIO();
    //Attach interrupt handler to GPIO pins:
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreatePinnedToCore(buttonPressHandlerGeneric, "handle_button_pressed", 2048, NULL, 10, NULL, 1);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BOOT, gpio_isr_handler, (void *)BOOT);
#endif
}

void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGD(TAG, "Notification of a time synchronization event");
}

char *get_types(char *stringf, int count) {
    char *stringfBuffer = stringf;
    char c = *stringf;
    char *types = malloc(sizeof(char) * count);
    for (int i = 0; i < strlen(stringf); i++) {
        c = *stringfBuffer;
        if (c == '%') {
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

int variable_sprintf_size(char *string, int count, ...) {
    va_list list;
    va_start(list, count);
    int extraSize = 0;
    char *snBuf = "";
    char *types = get_types(string, count);
    for (int i = 0; i < count; i++) {
        char type = *types++;
        switch (type) {
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
            case 'f':
                extraSize += snprintf(snBuf, 0, "%f", va_arg(list, double));
                break;
        }
    }
    int totalSize = strlen(string) * sizeof(char) + sizeof(char) * extraSize;
    return totalSize;
}

/* Event handler for catching system events */
void prov_event_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data) {
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGD(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV:
            {
                ESP_LOGD(TAG, "Received Wi-Fi credentials");
                // wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                // ESP_LOGD(TAG, "Received Wi-Fi credentials"
                //     "\n\tSSID     : %s\n\tPassword : %s",
                //     (const char *)wifi_sta_cfg->ssid,
                //     (const char *)wifi_sta_cfg->password); // only reveal password on serial interface in very specific debug sessions!
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
                ESP_LOGD(TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_start();
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGD(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGD(TAG, "Disconnected.");
        /* Clear the WIFI_CONNECTED_EVENT bits when a disconnect is detected; ensuring that the bits properly signal connectivity */
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_EVENT);
        if (wifi_autoconnect) {
            ESP_LOGD(TAG, "Connecting to the AP again...");
            esp_wifi_connect();
        }
    }
}

esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id) {
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
                }
                else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client)); //DONE: checked malloc() balance by free()
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
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                    output_len = 0;
                }
                ESP_LOGD(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGD(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        default:
            ESP_LOGD(TAG, "GOT AN ERROR BUT DONT KNOW WHAT!");
            break;
    }
    return ESP_OK;
}

void wifi_init_sta(void) {
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void create_dat() {
    esp_err_t err;
    uint32_t dat;
    nvs_handle_t dat_handle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &dat_handle);
    if (err) {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGD(TAG, "Succesfully opened NVS twomes_storage!");
        err = nvs_get_u32(dat_handle, "dat", &dat);
        switch (err) {
            case ESP_OK:
                ESP_LOGD(TAG, "The dat was initialized already!\n");
                ESP_LOGD(TAG, "The dat is: %u\n", dat);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGD(TAG, "The dat is not initialized yet!");
                ESP_LOGD(TAG, "Creating dat");
                dat = esp_random();
                ESP_LOGD(TAG, "Attempting to store dat: %d", dat);
                err = nvs_set_u32(dat_handle, "dat", dat);
                if (!err) {
                    ESP_LOGD(TAG, "Succesfully wrote dat: %u to NVS twomes_storage", dat);
                }
                else {
                    ESP_LOGE(TAG, "Failed to write dat to NVS twomes_storage: %s", esp_err_to_name(err));
                }
                break;
            default:
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        nvs_close(dat_handle);
    }
    ESP_LOGD(TAG, "dat: %u", dat);
}

void get_dat(uint32_t *buf) {
    esp_err_t err;
    nvs_handle_t dat_handle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &dat_handle);
    if (err) {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGD(TAG, "Succesfully opened NVS twomes_storage!");
        err = nvs_get_u32(dat_handle, "dat", buf);
        switch (err) {
            case ESP_OK:
                ESP_LOGD(TAG, "The dat has succesfully been read!\n");
                break;
            default:
                ESP_LOGE(TAG, "%s", esp_err_to_name(err));
                break;
        }
    }
}

void prepare_device(const char *device_type_name) {
    uint32_t dat;
    if (wifi_initialized) {
        ESP_LOGD(TAG, "Wi-Fi has been enabled for true random dat generation!");
        create_dat();
        get_dat(&dat);
    }
    else {
        ESP_LOGD(TAG, "Wi-Fi has not been enabled for true random dat generation, enabling Wi-Fi!");
        if (connect_wifi("prepare_device")) {
            while (!wifi_initialized) {
                ESP_LOGD(TAG, "Waiting for Wi-Fi enable to finish.");
                vTaskDelay(100 / portTICK_PERIOD_MS);
            };
            get_dat(&dat); //device access token  created with true random generation
            ESP_LOGD(TAG, "Disabling Wi-Fi again as to not disturb provisioning.");
            disconnect_wifi("prepare_device");
        }
        else {
            get_dat(&dat); //device access token  created with pseudo random generation
            disconnect_wifi("prepare_device");
        }
    }
    char *device_name = malloc(DEVICE_NAME_SIZE); //DONE checked malloc() balanced by free()
    get_device_service_name(device_name, DEVICE_NAME_SIZE);
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_BLE
    char *qr_code_payload_template = "\n\n{\n\"ver\":\"v1\",\n\"name\":\"%s\",\n\"pop\":\"%u\",\n\"transport\":\"ble\"\n}\n\n";
    int qr_code_payload_size = variable_sprintf_size(qr_code_payload_template, 2, device_name, dat);
    char *qr_code_payload = malloc(qr_code_payload_size); //DONE checked malloc() balanced by free()
    snprintf(qr_code_payload, qr_code_payload_size, qr_code_payload_template, device_name, dat);
#endif
#ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
    char *qr_code_payload_template = "\n\n{\n\"ver\":\"v1\",\n\"name\":\"%s\",\n\"pop\":\"%u\",\n\"transport\":\"ble\",\n\"security\":\"1\",\n\"password\":\"%s\"\n}\n\n";
    int qr_code_payload_size = variable_sprintf_size(qr_code_payload_template, 2, device_name, dat, dat);
    char *qr_code_payload = malloc(qr_code_payload_size); //DONE checked malloc() balanced by free()
    snprintf(qr_code_payload, qr_code_payload_size, qr_code_payload_template, device_name, dat, dat);
#endif

    ESP_LOGI(TAG, "QR Code Payload: ");
    ESP_LOGI(TAG, "%s", qr_code_payload);
    free(qr_code_payload);

    char *post_device_payload_template = "\n\n{\n\"name\":\"%s\",\n\"device_type\":\"%s\",\n\"activation_token\":\"%u\"\n}\n\n";
    int post_device_payload_size = variable_sprintf_size(post_device_payload_template, 3, device_name, device_type_name, dat);
    char *post_device_payload = malloc(post_device_payload_size); //DONE checked malloc() balanced by free()
    snprintf(post_device_payload, post_device_payload_size, post_device_payload_template, device_name, device_type_name, dat);

    ESP_LOGI(TAG, "POST /device payload: ");
    ESP_LOGI(TAG, "%s", post_device_payload);
    free(post_device_payload);
    free(device_name);
}

void get_device_service_name(char *service_name, size_t max) {
    uint8_t eth_mac[6];
    const char *ssid_prefix = SSID_PREFIX;
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
        ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
    uint8_t **outbuf, ssize_t *outlen, void *priv_data) {
    if (inbuf) {
        ESP_LOGD(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void initialize_sntp(void) {
    ESP_LOGD(TAG, "SNTP: setting operating mode");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

void obtain_time(void) {
    ESP_LOGD(TAG, "Initializing SNTP");

    if (sntp_enabled()) {
        ESP_LOGD(TAG, "SNTP already initialized; no need to initialize again");
    }
    else {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        initialize_sntp();
        ESP_LOGD(TAG, "SNTP Initialized");
    }

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < NTP_RETRIES) {
        ESP_LOGD(TAG, "Waiting for system time to be set... (%d/%d)", retry, NTP_RETRIES);
        vTaskDelay(HTTPS_RETRY_WAIT_MS / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void timesync_task(void *data) {
    ESP_LOGD("Main", "Timesync task started");
    while (1) {
        //Wait for next measurement
        ESP_LOGD("Main", TIMESYNC_INTERVAL_TXT);
        vTaskDelay((TIMESYNC_INTERVAL_MS - HTTPS_PRE_WAIT_MS - HTTPS_POST_WAIT_MS) / portTICK_PERIOD_MS);
        timesync(ALREADY_CONNECTED);
    }
}

void initialize_timezone(char *timezone) {
    // Set timezone
    setenv("TZ", timezone, 0);
    tzset();
}

void timesync(bool already_connected) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGD(TAG, "Time sync: connecting to Wi-Fi and getting time over NTP.");
    if (!already_connected) {
        if (connect_wifi("timesync")) {

            ESP_LOGD(TAG, "Waiting for IP connection...");
            xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
            //Wait to make extra sure Wi-Fi is connected and stable
            vTaskDelay(HTTPS_PRE_WAIT_MS / portTICK_PERIOD_MS);
            ESP_LOGD(TAG, "Waiting for IP connection... done; initiating timesync call");
        }
        else {
            ESP_LOGE(TAG, "Failed to connect to network for timesync");
        }
    }

    //obtain time via NTP
    obtain_time();

    // update 'now' variable with current time
    time(&now);

    // log time as Unix time.
    uint32_t unix_time = time(NULL);
    ESP_LOGD(TAG, "Unix Time is: %d", unix_time);

    // log time as UTC time.
    char strftime_buf[64];
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGD(TAG, "UTC/date/time is: %s", strftime_buf);

    if (!already_connected) {
        //Disconnect WiFi
        disconnect_wifi("timesync");
    }
}

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
void start_presence_detection() {
    xTaskCreatePinnedToCore(&presence_detection_loop, "presence_detection_task", 4096, NULL, 1, NULL, 1);
}
#endif

void upload_heartbeat(int hbcounter) {
    char *measurementType = "\"heartbeat\"";
    //Updates Epoch Time
    time_t now = time(NULL);
    //Plain JSON request where values will be inserted.
    char *msg_plain = "{\"upload_time\":\"%d\",\"property_measurements\":[{"
        "\"property_name\": %s,"
        "\"measurements\":["
        "{\"timestamp\":\"%d\","
        "\"value\":\"%d\"}"
        "]}]}";
    //Get size of the message after inputting variables.
    int msgSize = variable_sprintf_size(msg_plain, 4, now, measurementType, now, hbcounter);
    //Allocating enough memory so inputting the variables into the string doesn't overflow
    char *msg = malloc(msgSize); //DONE: malloc() is balanced by free
    //Inputting variables into the plain json string from above(msgPlain).
    snprintf(msg, msgSize, msg_plain, now, measurementType, now, hbcounter);
    upload_data_to_server(VARIABLE_UPLOAD_ENDPOINT, POST_WITH_BEARER, msg, NULL, 0);
    free(msg);
}

void heartbeat_task(void *data) {
    ESP_LOGD("Main", "Heartbeat task started");
    for (int heartbeatcounter = 1; true; heartbeatcounter++) {
        upload_heartbeat(heartbeatcounter);
        //Wait for next measurement
        ESP_LOGD(TAG, HEARTBEAT_MEASUREMENT_INTERVAL_TXT);
        vTaskDelay((HEARTBEAT_MEASUREMENT_INTERVAL_MS - HTTPS_PRE_WAIT_MS - HTTPS_POST_WAIT_MS) / portTICK_PERIOD_MS);
    }
}

esp_err_t store_bearer(char *activation_response) {
    esp_err_t err;
    nvs_handle_t bearer_handle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &bearer_handle);
    if (err) {
        ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
    }
    else {
        ESP_LOGD(TAG, "Succesfully opened NVS twomes_storage!");
        err = nvs_set_str(bearer_handle, "bearer", activation_response);
        switch (err) {
            case ESP_OK:
                ESP_LOGD(TAG, "The bearer was written!\n");
                ESP_LOGD(TAG, "The bearer is %i characters: %s\n", strlen(activation_response), activation_response);
                break;
            default:
                ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
        }
        err = nvs_commit(bearer_handle);
        nvs_close(bearer_handle);
    }
    return err;
}

void delete_bearer() {
    esp_err_t err;
    bearer = "";
    err = store_bearer(""); 
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Bearer reset to force re-activation");
    }
    else {
        ESP_LOGE(TAG, "Error (%s) storing empty bearer value into nvs!", esp_err_to_name(err));
    }
}

char *get_bearer() {
    if (bearer != NULL) {
        ESP_LOGD(TAG, "Existing bearer read from memory: %s", bearer);
    }
    else {
        esp_err_t err;
        nvs_handle_t bearer_handle;
        err = nvs_open("twomes_storage", NVS_READONLY, &bearer_handle);
        if (err) {
            ESP_LOGE(TAG, "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
        }
        else {
            ESP_LOGD(TAG, "Succesfully opened NVS twomes_storage!");
            size_t bearer_size;
            nvs_get_str(bearer_handle, "bearer", NULL, &bearer_size);
            bearer = malloc(bearer_size); //TODO: don't free(bearer); malloc() is used only once!
            err = nvs_get_str(bearer_handle, "bearer", bearer, &bearer_size);
            switch (err) {
                case ESP_OK:
                    ESP_LOGD(TAG, "The bearer was read!\n");
                    ESP_LOGD(TAG, "The bearer is: %s\n", bearer);
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGD(TAG, "The bearer was not yet initialized!");
                    bearer = "";
                    break;
                default:
                    bearer = NULL;
                    ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
            }
            nvs_close(bearer_handle);
        }
    }
    return bearer;
}

void activate_device() {
    esp_err_t err = 0;
    uint32_t dat;
    get_dat(&dat);
    activation = true;
    int https_response_status = 0;
    char *device_activation_plain = "{\"activation_token\":\"%u\"}";
    int activation_data_size = variable_sprintf_size(device_activation_plain, 1, dat);
    char *device_activation_data = malloc(activation_data_size); //DONE: check whether malloc() is balanced by free()
    snprintf(device_activation_data, activation_data_size, device_activation_plain, dat);

    ESP_LOGD(TAG, "%s", device_activation_data);
    char *activation_response = malloc(sizeof(char) * MAX_RESPONSE_LENGTH); //DONE: check whether malloc() is balanced by free()
    https_response_status = post_https(DEVICE_ACTIVATION_ENDPOINT, POST_WITHOUT_BEARER, ALREADY_CONNECTED, device_activation_data, activation_response, MAX_RESPONSE_LENGTH);
    free(device_activation_data);
    ESP_LOGD(TAG, "Return from device activation endpoint!");

    // this section of code first retrieves the bearer from the JSON-formatted response.
    // TODO: replace with a call to a proper json parser.
    if (https_response_status != HTTPSTATUS_OK) { //change everywhere in code to HttpStatus_Ok when including esp_http_client.h library of 19 Nov 2020 or later
        ESP_LOGE(TAG, "Failed to activate device: response is not %i but %i", HTTPSTATUS_OK, https_response_status);
    }
    else {
        if (!activation_response) {
            ESP_LOGE(TAG, "Failed to activate device: no activation response!");
        }
        else {
            ESP_LOGD(TAG, "Bearer returned in response: %i characters: %s", strlen(activation_response), activation_response);
            char *bearer_trimmed = strchr(activation_response, ':'); // set begin of string at first colon
            ESP_LOGD(TAG, "Initial bearer trimmed: %i characters: %s", strlen(bearer_trimmed), bearer_trimmed);
            bearer_trimmed = strchr(bearer_trimmed, '\"'); // set begin of string at first quote after that
            bearer_trimmed++; // move forward one position to first real character
            char *end_of_bearer_string = strchr(bearer_trimmed, '\"'); // search for position of final quote
            *end_of_bearer_string = '\0'; // set end-of-string character in that position in memory
            ESP_LOGD(TAG, "Final bearer_trimmed %i characters: %s", strlen(bearer_trimmed), bearer_trimmed);
            if (store_bearer(bearer_trimmed) == ESP_OK) {
                bearer = strdup(bearer_trimmed); //copy the trimmed bearer into the global bearer variable.
                ESP_LOGD(TAG, "Final bearer stored: %i characters: %s", strlen(bearer), bearer);
            }
            else {
                ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
            }
            free(activation_response);
        }
    }
}

int post_https(char *endpoint, bool use_bearer, bool already_connected, char *data, char *response_buf, uint8_t resp_buf_size) {
    int connect_retry_counter = 0;
    int upload_retry_counter = 0;
    int content_length = 0;
    bool connect_success = false;
    int status_code = 0;
    char *response = NULL;
    char *urlString = NULL;

    if (endpoint) {
        char *urlPlain = TWOMES_SERVER "%s";
        int strCount = snprintf(urlString, 0, urlPlain, endpoint) + 1;
        urlString = malloc(sizeof(char) * strCount); //DONE: checked this malloc() is balanced by free()
        snprintf(urlString, strCount * sizeof(char), urlPlain, endpoint);
    }

    esp_http_client_config_t config = {
        .url = urlString,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = isrgrootx1,
        .event_handler = http_event_handler,
        .is_async = false };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Host", TWOMES_SERVER_HOSTNAME);
    esp_http_client_set_header(client, "Content-Type", "text/plain");

    char *authenticationToken;
    char *authenticationTokenString = "";
    if (use_bearer) {
        authenticationToken = get_bearer();
        if (strlen(authenticationToken) > 1) {
            ESP_LOGD(TAG, "Bearer read %i characters: %s", strlen(authenticationToken), authenticationToken);
        }
        else if (strcmp(authenticationToken, "") == 0) {
            ESP_LOGD(TAG, "Bearer not found, activate device first!");
        }
        else if (!authenticationToken) {
            ESP_LOGE(TAG, "Something went wrong reading the bearer!");
        }

        if (authenticationToken) {
            char *authenticationTokenStringPlain = "Bearer %s";
            int strCount = snprintf(authenticationTokenString, 0, authenticationTokenStringPlain, authenticationToken) + 1;
            authenticationTokenString = malloc(sizeof(char) * strCount);
            snprintf(authenticationTokenString, strCount * sizeof(char), authenticationTokenStringPlain, authenticationToken);
            esp_http_client_set_header(client, "Authorization", authenticationTokenString);
        }
    }

    esp_http_client_set_post_field(client, data, strlen(data));
    ESP_LOGD(TAG, "Free heap: %d bytes", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    if (!already_connected) {
        connect_success = connect_wifi(endpoint);
        while (!connect_success && ++connect_retry_counter < WIFI_CONNECT_RETRIES) {
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi (%d/%d) at %s", connect_retry_counter, WIFI_CONNECT_RETRIES, esp_log_system_timestamp());
            connect_success = connect_wifi(endpoint);
        }

        if (!connect_success) {
            //if still not managed to upload data after many retries; then a reset may be needed to avoid worse...
            ESP_LOGD(TAG, "Minimum free heap size: %d bytes", esp_get_minimum_free_heap_size());
            for (int i = 10; i >= 0; i--) {
                ESP_LOGE(TAG, "Could still not get connection; restarting in %d seconds...", i);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            ESP_LOGE(TAG, "Restarting now.");
            fflush(stdout);
            esp_restart();
        }

        ESP_LOGD(TAG, "Waiting for IP connection...");
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
        ESP_LOGD(TAG, "Waiting for IP connection... done");

        //Wait to make extra sure Wi-Fi is connected and stable
        vTaskDelay(HTTPS_PRE_WAIT_MS / portTICK_PERIOD_MS);
    }

    ESP_LOGD(TAG, "Initiating call to endpoint %s with payload: %s", endpoint, data);

    esp_err_t err = esp_http_client_open(client, strlen(data));
    while (err != ESP_OK && ++upload_retry_counter < HTTPS_UPLOAD_RETRIES) {
        ESP_LOGE(TAG, "Failed to open HTTPS connection %s (%d/%d) at %s", esp_err_to_name(err), upload_retry_counter, HTTPS_UPLOAD_RETRIES, esp_log_system_timestamp());
        vTaskDelay(HTTPS_RETRY_WAIT_MS / portTICK_PERIOD_MS);
        ESP_LOGD(TAG, "Free heap: %d bytes", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        err = esp_http_client_open(client, strlen(data));
    }

    if (err == ESP_OK) {
        esp_err_t err = esp_http_client_perform(client);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send HTTPS POST request: %s at at %s", esp_err_to_name(err), esp_log_system_timestamp());
        }
        else {
            status_code = esp_http_client_get_status_code(client);
            content_length = esp_http_client_get_content_length(client);
            if (content_length > 0) {
                response = malloc(sizeof(char) * content_length); //DONE: checked that malloc() is balanced by free()
                esp_http_client_read(client, response, content_length);
                if (status_code != HTTPSTATUS_OK) {
                    ESP_LOGE(TAG, "Status Code: %d Response Length: %d", status_code,
                        content_length);
                    ESP_LOGE(TAG, "Response: %s", response);
                }
                else {
                    ESP_LOGD(TAG, "Status Code: %d Response Length: %d", status_code,
                        content_length);
                    ESP_LOGD(TAG, "Response: %s", response);
                }
                esp_http_client_close(client);
            }
            else {
                ESP_LOGE(TAG, "No proper response, response length: %d status_code: %d", content_length, status_code);
            }
        }
    }
    else { //if still not managed to upload data after many retries; then a reset may be needed to avoid worse...
        ESP_LOGD(TAG, "Minimum free heap size: %d bytes", esp_get_minimum_free_heap_size());
        for (int i = 10; i >= 0; i--) {
            ESP_LOGE(TAG, "Could still not upload; restarting in %d seconds...", i);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGE(TAG, "Restarting now.");
        fflush(stdout);
        esp_restart();
    }
    if (use_bearer) {
        if (authenticationToken) {
            free(authenticationTokenString);
        }
    }
    if (endpoint) {
        free(urlString);
    }
    esp_http_client_cleanup(client);
    if (response && resp_buf_size) {
        int missed = snprintf(response_buf, resp_buf_size, "%s", response);
        if (missed > resp_buf_size) {
            ESP_LOGE(TAG, "Buffer was too small, full string was not written. Missed %d amount of character space!", missed - resp_buf_size);
        }
        if (content_length > 0) {
            free(response);
        }

    }
    //always disconnect after & release mutex at the end of this function
    disconnect_wifi(endpoint);
    return status_code;
}

int upload_data_to_server(char *endpoint, bool use_bearer, char *data, char *response_buf, uint8_t resp_buf_size) {
    return post_https(endpoint, use_bearer, NOT_ALREADY_CONNECTED, data, response_buf, resp_buf_size);
}

wifi_prov_mgr_config_t initialize_provisioning() {
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
                .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
        #endif /* CONFIG_TWOMES_PROV_TRANSPORT_BLE */
        #ifdef CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP
                                            .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
        #endif /* CONFIG_TWOMES_PROV_TRANSPORT_SOFTAP */
    };
    return config;
}

void start_provisioning(wifi_prov_mgr_config_t config, bool connect) {
    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        ESP_LOGD(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
        char *service_name = malloc(DEVICE_NAME_SIZE); //TODO: check whether malloc() is balanced by free()
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
        char *dat_str = malloc(msgSize); //TODO: check whether malloc() is balanced by free()
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
    else {
        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();
        if (connect) {
            ESP_LOGD(TAG, "Already provisioned, starting Wi-Fi STA");
            /* Start Wi-Fi station */
            wifi_init_sta();
        }
        else {
            ESP_LOGD(TAG, "Already provisioned, not starting Wi-Fi because connecting is disabled");
        }
    }
    /* Wait for Wi-Fi connection */
    if (connect) {
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
    }

    // free(service_name); //TODO: perhaps free needed here
    // free(dat_str); // TODO: perhaps free needed here
}

bool disable_wifi(char *taskString) {
    if (esp_wifi_stop() == ESP_OK) {
        ESP_LOGD(TAG, "Disabled Wi-Fi");
        wifi_initialized = false;
        ESP_LOGD(TAG, "%s will release the 802_11 resource at %s", taskString, esp_log_system_timestamp());
        //Wait to make sure everyting is finished.
        vTaskDelay(HTTPS_POST_WAIT_MS / portTICK_PERIOD_MS);
        xSemaphoreGiveRecursive(wireless_802_11_mutex);
        return true;
    }
    else {
        ESP_LOGE(TAG, "Failed to disable Wi-Fi");
        return false;
    }
}

bool disable_wifi_keeping_802_11_mutex() {
    if (esp_wifi_stop() == ESP_OK) {
        ESP_LOGD(TAG, "Disabled Wi-Fi without releasing 802.11 mutex");
        wifi_initialized = false;
        //Wait to make sure everyting is finished.
        vTaskDelay(HTTPS_POST_WAIT_MS / portTICK_PERIOD_MS);
        return true;
    }
    else {
        ESP_LOGE(TAG, "Failed to disable Wi-Fi without releasing 802.11 mutex");
        return false;
    }
}


bool enable_wifi(char *taskString) {
    if (xSemaphoreTakeRecursive(wireless_802_11_mutex, MAX_WAIT_802_11_MS / portTICK_PERIOD_MS)) {
        ESP_LOGD(TAG, "%s got access to 802_11 resource at %s", taskString, esp_log_system_timestamp());
        if (esp_wifi_start() == ESP_OK) {
            ESP_LOGD(TAG, "Enabled Wi-Fi");
            wifi_initialized = true;
            return true;
        }
        else {
            ESP_LOGE(TAG, "Failed to enable Wi-Fi");
            return false;
        }
    }
    else {
        ESP_LOGE(TAG, "%s failed to get access to 802_11 resource witin %s", taskString, MAX_WAIT_802_11_TXT);
        return false;
    }
}

bool disconnect_wifi(char *taskString) {
    wifi_autoconnect = false;
    esp_err_t err = esp_wifi_disconnect();
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Disconnected Wi-Fi");
        ESP_LOGD(TAG, "%s will release the 802_11 resource at %s", taskString, esp_log_system_timestamp());
        //Wait to make extra sure Wi-Fi is connected and stable
        vTaskDelay(HTTPS_POST_WAIT_MS / portTICK_PERIOD_MS);
        xSemaphoreGive(wireless_802_11_mutex);
        return true;
    }
    else {
        ESP_LOGE(TAG, "Failed to disconnect Wi-Fi: %s", esp_err_to_name(err));
        return false;
    }
}

bool connect_wifi(char *taskString) {
    if (xSemaphoreTakeRecursive(wireless_802_11_mutex, MAX_WAIT_802_11_MS / portTICK_PERIOD_MS)) {
        ESP_LOGD(TAG, "%s got access to 802_11 resource at %s", taskString, esp_log_system_timestamp());
        esp_err_t err;
        if (wifi_initialized) {
            err = esp_wifi_connect();
        }
        else {
            err = esp_wifi_start();
        }
        wifi_autoconnect = true;
        if (err == ESP_OK) {
            wifi_initialized = true;
            ESP_LOGD(TAG, "Succesfully connected Wi-Fi");
            return true;
        }
        else {
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi: %s", esp_err_to_name(err));
            ESP_LOGD(TAG, "%s will release the 802_11 resource at %s", "connect_wifi_having_802_11_mutex", esp_log_system_timestamp());
            xSemaphoreGiveRecursive(wireless_802_11_mutex);
            return false;
        }
    }
    else {
        ESP_LOGE(TAG, "%s failed to get access to 802_11 resource witin %s", taskString, MAX_WAIT_802_11_TXT);
        return false;
    }
}

void initialize_nvs() {
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

void twomes_device_provisioning(const char *device_type_name) {
    initialize_nvs();
    initialize_generic_firmware();
    /* initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_prov_mgr_config_t config = initialize_provisioning();

    // make sure to have this here otherwise the device names won't match because
    // of config changes made by the above function call.
    prepare_device(device_type_name);

    // starts provisioning if not provisioned, otherwise skips provisioning.
    // if set to false it will not autoconnect after provisioning.
    // if set to true it will autonnect.
    start_provisioning(config, true);

    // initialize time with timezone UTC; building timezone is stored in central database
    initialize_timezone("UTC");
    timesync(ALREADY_CONNECTED);

    // get bearer token
    bearer = get_bearer();

    if (bearer != NULL && strlen(bearer) > 1) {
        ESP_LOGD(TAG, "Bearer read %i characters: %s", strlen(bearer), bearer);
    }

    else if (strcmp(bearer, "") == 0) {
        ESP_LOGD(TAG, "Bearer not found, activating device!");
        activate_device();
        ESP_LOGD(TAG, "Bearer after activation is %i characters: %s", strlen(bearer), bearer);
    }

    else if (!bearer) {
        ESP_LOGE(TAG, "Something went wrong while reading the bearer!");
    }
    disconnect_wifi("after provisioning");
}