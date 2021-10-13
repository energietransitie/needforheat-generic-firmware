/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "presence_detection.h"
// #include "hcicmds.h"
// #include "hid_dev.h"

//Start the addr list with room for 10 adresses
#define PRESENCE_ADDR_LIST_START_SIZE 10
#define PRESENCE_NAME_REQ_TIMEOUT 5
#define BLUETOOTH_PRESENCE_TASK_TXT "bluetooth_presence_detection"
#define TIMER_DIVIDER 80
#define ADDR_LEN 17 // 6 * 2 hex digits + 5 colons

esp_gatt_if_t interface;

esp_bd_addr_t phone = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F}; //example address
esp_bd_addr_t phone2 = {0x7A, 0x8B, 0x9C, 0xAD, 0xAE, 0xBA}; //example address
esp_bd_addr_t phone3 = {0xCA, 0xDB, 0xEC, 0xFD, 0xA1, 0xB2}; //example address

esp_bd_addr_t presence_addr_list[3] = {};
int presence_addr_list_count = 3;
presence_data result_list[3];
const char *TAG = "Twomes Presence Detection";
int requesting_number = -1;
bool requesting = false;
int timeout_count = 0;
int measuring_interval_count = 0;
uint32_t time_measuring_start_timestamp;
bool stopped = false;
bool found_after_stopped = false;

//Our timer interrupt so that we increment the appropriate counters every interval
//Timeout is only used when we are sending requests. It is started after a request is sent and will count until a request returns
//Or it reaches it's maximum value(checked in the presence detection loop) in which case we note that the device was not detected
bool IRAM_ATTR timer_group_isr_callback(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;
    if (requesting || stopped)
    {
        timeout_count++;
    }
    else
    {
        measuring_interval_count++;
    }
    return high_task_awoken == pdTRUE; // return whether we need to yield at the end of ISR
}

//Function that is used to start the necessary timer to track our counters.
void initialize_timer(timer_group_t group, timer_idx_t timer, bool auto_reload, int timer_interval)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = (timer_autoreload_t)auto_reload,
        .divider = TIMER_DIVIDER}; // default clock source is APB

    timer_init(group, timer, &config);

    timer_set_alarm_value(group, timer, timer_interval);
    timer_enable_intr(group, timer);

    timer_isr_callback_add(group, timer, timer_group_isr_callback, NULL, 0);

    timer_start(group, timer);
}

//Callback mostly just for our name requests, it starts the next name request if there is another device that needs to be detected
//It also inserts its result into the result_list The 1 second delay is needed so that the measurements go well. If you do not
//do this then it will not always detect the device properly and thus make a measuring error.
void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_READ_REMOTE_NAME_EVT:
        timeout_count = 0;
        ESP_LOGD(TAG, "Read Remote Name!");
        ESP_LOGD(TAG, "Name: %s", param->read_rmt_name.rmt_name);
        if (strlen((const char *)param->read_rmt_name.rmt_name) > 0)
        {
            ESP_LOGD(TAG, "CB found device, name not empty: %s!", param->read_rmt_name.rmt_name);
            store_measurement(true);
            if (stopped)
            {
                found_after_stopped = true;
            }
        }
        if (requesting && requesting_number <= (presence_addr_list_count - 1))
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            send_name_request(presence_addr_list[++requesting_number]);
        }
        break;
    default:
        ESP_LOGD(TAG, "Got Other GAP Event: %d!", event);
        break;
    }
}

//Initializes presence_detection
void initialize_presence_detection()
{
    initialize_bluetooth();
    initialize_timer(TIMER_GROUP_0, TIMER_0, true, 1000000);
    memcpy(presence_addr_list[0], phone, sizeof(esp_bd_addr_t));
    memcpy(presence_addr_list[1], phone2, sizeof(esp_bd_addr_t));
    memcpy(presence_addr_list[2], phone3, sizeof(esp_bd_addr_t));
}

//Send name request to private MAC adress given in mac_addr variable.
//Note, this function does not return if the request has found it's target.
//That happens later in the callback.
void send_name_request(esp_bd_addr_t mac_addr)
{
    esp_err_t err = esp_bt_gap_read_remote_name(mac_addr);
    if (err == ESP_OK)
    {
        ESP_LOGD(TAG, "Sent name request!");
    }
    else
    {
        ESP_LOGD(TAG, "Failed to send name request, error: %s!", esp_err_to_name(err));
    }
}

//Initializes bluetooth
void initialize_bluetooth()
{
    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret)
    {
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    esp_bt_gap_register_callback(gap_callback);
}

//This function starts the requesting by setting the right variables.
void start_requesting()
{
    if (xSemaphoreTake(wireless_802_11_mutex, MAX_WAIT_802_11_MS / portTICK_PERIOD_MS)) {
        ESP_LOGD(TAG, "%s got access to 802_11 resource at %s", BLUETOOTH_PRESENCE_TASK_TXT, esp_log_system_timestamp());
        if (disable_wifi_keeping_802_11_mutex()) {
            time_measuring_start_timestamp = time(NULL);
            measuring_interval_count = 0;
            requesting = true;
            requesting_number = 0;
            send_name_request(presence_addr_list[requesting_number]);
        } 
        else {
            ESP_LOGE(TAG, "%s failed to disable wifi without releasing 802.11 mutex", BLUETOOTH_PRESENCE_TASK_TXT);
        }
    }
    else {
        ESP_LOGE(TAG, "%s failed to get access to 802_11 resource witin %s", BLUETOOTH_PRESENCE_TASK_TXT, MAX_WAIT_802_11_TXT);
    }
}

void reset_results(){
     memset(result_list, 0, sizeof(result_list));
}

//Stops our requesting of names to the devices. This is called when we have sent out requests to all devices.
void stop_requesting()
{
    requesting = false;
}

//This function takes the bluetooth address of a presence_data result and turns it into a string
//Which goes into the given buffer and does not write more than the buffer_size which in general is ADDR_LEN + 1.
//The +1 is for the null-terminator which snprintf inserts automatically if there is space for it.
void presence_addr_to_string(presence_data data, char *buffer, int buffer_size)
{
    esp_bd_addr_t addr;
    memcpy(addr, data.addr, sizeof(esp_bd_addr_t));
    ESP_LOGD(TAG, "Converting address %X:%X:%X:%X:%X:%X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    snprintf(buffer, buffer_size, "%X:%X:%X:%X:%X:%X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    ESP_LOGD(TAG, "Converted addr: %s", buffer);
}

//This function turns one detection result(presence_data) into a string which can be inserted into another string
//In the function upload_presence_detection_data() to make the data string for the POST request.
/*
char *result_to_string(presence_data data)
{
    char *property_string_plain_addr = "{\"property_name\":\"%s\","
                                       "\"measurements\": ["
                                       "{ \"timestamp\":\"%d\","
                                       "\"value\":\"%s\"}]},";

    char *property_string_plain_isHome = "{\"property_name\":\"%s\","
                                         "\"measurements\": ["
                                         "{ \"timestamp\":\"%d\","
                                         "\"value\":\"%d\"}]}";
    int max_property_string_length = (variable_sprintf_size("%s", 1, "bleAddress") + sizeof(uint32_t) +
                                      ADDR_LEN + strlen(property_string_plain_addr));
    int presence_string_properties_length = 2 * max_property_string_length;
    char *presence_string_properties = malloc(presence_string_properties_length);
    int addr_size = ADDR_LEN + 1;
    char *addr_buf = malloc(addr_size);
    presence_addr_to_string(data, addr_buf, addr_size);
    char *property_buf = malloc(max_property_string_length);
    snprintf(property_buf, max_property_string_length, property_string_plain_addr, "bleAddress", data.timeMeasured, addr_buf);
    strcpy(presence_string_properties, property_buf);
    snprintf(property_buf, max_property_string_length, property_string_plain_isHome, "isHome", data.timeMeasured, data.isHome);
    strcat(presence_string_properties, property_buf);
    return presence_string_properties;
}
*/

char *results_to_rssi_list()
{
    char *property_string_plain = "{\"property_name\":\"%s\","
                                  "\"measurements\":["
                                  "{ \"timestamp\":\"%d\","
                                  "\"value\":\"%s\"}";

    //Max size of RSSI(4 characters) + size of ',' and '\0'(2 characters) * addr list count gives us our maximum string size.
    int rssi_string_size = variable_sprintf_size("%d", 1, RSSI_ABSENT) + sizeof(char) * 2;
    char *rssi_string = malloc(rssi_string_size * presence_addr_list_count); //DONE check whether this malloc() is balanced by free()
    ESP_LOGD(TAG, "RSSI String Size: %d", rssi_string_size);
    char *rssi_partial = malloc(rssi_string_size); //DONE check whether this malloc() is balanced by free()
    if (result_list[0].isHome)
    {
        snprintf(rssi_partial, rssi_string_size, "%d,", RSSI_PRESENT);
    }
    else if (result_list[0].isHome == 0)
    {
        snprintf(rssi_partial, rssi_string_size, "%d,", RSSI_ABSENT);
    }
    strcpy(rssi_string, rssi_partial);
    for (int i = 1; i < presence_addr_list_count; i++)
    {
        if (result_list[i].isHome)
        {
            snprintf(rssi_partial, rssi_string_size, "%d", RSSI_PRESENT);
        }
        else if (result_list[i].isHome == 0)
        {
            snprintf(rssi_partial, rssi_string_size, "%d", RSSI_ABSENT);
        }
        if (i != presence_addr_list_count - 1)
        {
            strcat(rssi_partial, ",");
        }
        strcat(rssi_string, rssi_partial);
    }
    ESP_LOGD(TAG, "RSSI Result: %s", rssi_string);
    int property_string_size = variable_sprintf_size(property_string_plain, 3, "listRSSI", time_measuring_start_timestamp,
                                                     rssi_string);
    char *property_string = malloc(property_string_size); //DONE checked that this malloc() is balanced by free()
    snprintf(property_string, property_string_size, property_string_plain, "listRSSI", time_measuring_start_timestamp, rssi_string);
    free(rssi_string);
    free(rssi_partial);
    ESP_LOGD(TAG, "Property String Result: %s", property_string);
    return property_string;
}
//Uploads our data via the upload_data_to_server() of the generic firmware
void upload_presence_detection_data()
{
    char *msg_multiple_string_plain = "{\"upload_time\":\"%d\",\"property_measurements\":["
                                    "%s"
                                    "]}]}";

    int msg_multiple_string_size;
    char *msg_multiple_string;
    char *rssi_property_string = results_to_rssi_list(); //DONE: checked malloc() is balanced by free()
    msg_multiple_string_size = variable_sprintf_size(msg_multiple_string_plain, 2, time(NULL), rssi_property_string);
    msg_multiple_string = malloc(msg_multiple_string_size); //DONE: checked: malloc() is balanced by free()
    snprintf(msg_multiple_string, msg_multiple_string_size, msg_multiple_string_plain, time(NULL), rssi_property_string);
    upload_data_to_server(VARIABLE_UPLOAD_ENDPOINT, POST_WITH_BEARER, msg_multiple_string, NULL, 0);
    free(rssi_property_string);
    free(msg_multiple_string);
    reset_results();
}

void store_measurement(bool isHome)
{
    presence_data res;
    memcpy(res.addr, presence_addr_list[requesting_number], sizeof(esp_bd_addr_t));
    ESP_LOGD(TAG, "Storing measurement address: %X:%X:%X:%X:%X:%X with isHome: %d", res.addr[0], res.addr[1], res.addr[2], res.addr[3],
             res.addr[4], res.addr[5], isHome);
    res.isHome = isHome;
    res.timeMeasured = time_measuring_start_timestamp;
    result_list[requesting_number] = res;
}

//Our presence detection loop
void presence_detection_loop(void)
{
    initialize_presence_detection();
    // for (int i = 0; i < 10; i++)
    // {
    //     add_addr_to_target_list(phone);
    //     ESP_LOGD(TAG, "Addr Count: %d", presence_addr_list_count);
    // }
    // print_presence_addr_list();
    // add_addr_to_target_list(phone);
    // extend_presence_addr_list();
    start_requesting();
    while (1)
    {
        //We have stopped scanning and were waiting for the last response which now timedout
        if (stopped && timeout_count >= PRESENCE_NAME_REQ_TIMEOUT)
        {
            ESP_LOGD(TAG, "Timedout final %d", requesting_number);
            if (found_after_stopped)
            {
                store_measurement(true);
            }
            else
            {
                store_measurement(false);
            }
            stopped = false;
            timeout_count = 0;
            requesting_number = 0;
            found_after_stopped = false;
            ESP_LOGD(TAG, "Finally Finishing up!");
            ESP_LOGD(TAG, "%s keeps the 802_11 resource a little longer for uploading", BLUETOOTH_PRESENCE_TASK_TXT);
            upload_presence_detection_data();
            ESP_LOGD(TAG, PRESENCE_INTERVAL_TXT);
        }
        //Starting a measurement when we reach the specified timer interval
        if (measuring_interval_count >= PRESENCE_MEASUREMENT_INTERVAL_S)
        {
            start_requesting();
        }
        //Timedout so device isn't home
        if (requesting && timeout_count >= PRESENCE_NAME_REQ_TIMEOUT)
        {
            //We still have another device to scan
            if (requesting_number < (presence_addr_list_count - 1))
            {
                ESP_LOGD(TAG, "Timedout_not_end %d", requesting_number);
                store_measurement(false);
                requesting_number++;
                ESP_LOGD(TAG, "Could not find number: %d", requesting_number - 1);
            }
            //No more devices to scan
            else
            {
                ESP_LOGD(TAG, "Timedout_end %d", requesting_number);
                stop_requesting();
                store_measurement(false);
                ESP_LOGD(TAG, "%s keeps the 802_11 resource a little longer for uploading", BLUETOOTH_PRESENCE_TASK_TXT);
                upload_presence_detection_data();
                ESP_LOGD(TAG, "Stopping Requesting Early!");
                ESP_LOGD(TAG, PRESENCE_INTERVAL_TXT);
            }
            timeout_count = 0;
        }
        //Stopping requesting because we have scanned all devices, giving the last one time
        //to respond which is why we set stopped, not uploading until last result is in.
        if (requesting && requesting_number >= presence_addr_list_count - 1)
        {
            stop_requesting();
            stopped = true;
            ESP_LOGD(TAG, "Stopping Requesting!");
        }
        //Only a very short delay needed here, can be longer if it interferes with other tasks ofcourse.
        vTaskDelay(10 / portTICK_PERIOD_MS);
        /* Blink off (output low) */
    }
}
