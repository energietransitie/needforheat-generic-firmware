#include <generic_esp_32.h>

static const char *TAG = "Twomes Example Application ESP32";

void app_main(void)
{
    initialize_nvs();
    initialize();
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_prov_mgr_config_t config = initialize_provisioning();
    //Starts provisioning if not provisioned, otherwise skips provisioning. If set to false it will not autoconnect after provisioning. If set to true it will autonnect.
    start_provisioning(config, false);
    //Initialize time with timezone Europe and city Amsterdam
    initialize_time("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
    //URL Do not forget to use https:// when using the https() function.
    char* url = "https://192.168.178.75:4444/set/house/opentherm";

    //Gets time as epoch time.
    int now = time(NULL);

    //Creates data string replacing %d with the time integer.
    char* dataPlain = "{\"deviceMac\":\"8C:AA:B5:85:A2:3D\",\"measurements\": [{\"property\":\"testy\",\"value\":\"hello_world\"}],\"time\":%d}";
    char data [strlen(dataPlain)];
    sprintf(data, dataPlain, now);

    /* Start main application now */
    while (1)
    {
        //Logs hello world and tries to post with https every 10 seconds. Replace post_https(url, data, cert) with post_http(url,data) to use plain http over TCP
        ESP_LOGI(TAG, "Hello World!");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        post_https(url, data, NULL);
        //Reconnect to provisioning by supplying the conifg and setting connect to true.
        start_provisioning(config, true);
    }
}