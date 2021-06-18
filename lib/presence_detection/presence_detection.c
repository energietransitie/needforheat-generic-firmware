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
#define TIMER_DIVIDER 80
#define MEASURING_INTERVAL 300
#define ADDR_LEN 12

const char *rootCA;
char *bearer;

const char *variable_interval_upload_url = TWOMES_TEST_SERVER "/device/measurements/variable-interval";

esp_err_t gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);

esp_gatt_if_t interface;

esp_bd_addr_t phone = {0xf0, 0x67, 0x28, 0xbd, 0xa1, 0xa2};
esp_bd_addr_t phone2 = {0xac, 0x5f, 0xea, 0x6b, 0x3a, 0x6b};

esp_bd_addr_t presence_addr_list[2] = {};
int presence_addr_list_count = 2;
presence_data result_list[2];
const char *TAG = "twomes_presence_detection";
int requesting_number = -1;
bool requesting = false;
int timeout_count = 0;
int measuring_interval_count = 0;
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
esp_err_t gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_READ_REMOTE_NAME_EVT:
        ESP_LOGI(TAG, "Read Remote Name!");
        ESP_LOGI(TAG, "Name: %s", param->read_rmt_name.rmt_name);
        presence_data result;
        memcpy(result.addr, presence_addr_list[requesting_number], sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "Got cb address %x%x%x%x%x%x", result.addr[0], result.addr[1], result.addr[2], result.addr[3],
                 result.addr[4], result.addr[5]);
        if (strlen((const char *)param->read_rmt_name.rmt_name) == 0)
        {
            ESP_LOGI(TAG, "CB Did not find device, name empty!");
            result.isHome = false;
        }
        else
        {
            ESP_LOGI(TAG, "CB found device, name not empty: %s!", param->read_rmt_name.rmt_name);
            result.isHome = true;
        }
        result.timeMeasured = time(NULL);
        result_list[requesting_number] = result;
        if (stopped)
        {
            found_after_stopped = true;
        }
        if (requesting && requesting_number <= (presence_addr_list_count - 1))
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            send_name_request(presence_addr_list[++requesting_number]);
        }
        timeout_count = 0;
        break;
    default:
        ESP_LOGI(TAG, "Got Other GAP Event: %d!", event);
        break;
    }
    return ESP_OK;
}

//Initializes presence_detection
void initialize_presence_detection()
{
    initialize_bluetooth();
    initialize_timer(TIMER_GROUP_0, TIMER_0, true, 1000000);
    memcpy(presence_addr_list[0], phone, sizeof(esp_bd_addr_t));
    memcpy(presence_addr_list[1], phone2, sizeof(esp_bd_addr_t));
}

//Send name request to private MAC adress given in mac_addr variable.
//Note, this function does not return if the request has found it's target.
//That happens later in the callback.
void send_name_request(esp_bd_addr_t mac_addr)
{
    esp_err_t err = esp_bt_gap_read_remote_name(mac_addr);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Sent name request!");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to send name request, error: %s!", esp_err_to_name(err));
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
    measuring_interval_count = 0;
    requesting = true;
    requesting_number = 0;
    send_name_request(presence_addr_list[requesting_number]);
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
    ESP_LOGI(TAG, "Converting address %x%x%x%x%x%x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    snprintf(buffer, buffer_size, "%x%x%x%x%x%x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    ESP_LOGI(TAG, "Converted addr: %s", buffer);
}

//This function turns one detection result(presence_data) into a string which can be inserted into another string
//In the function upload_presence_detection_data() to make the data string for the POST request.
char *result_to_string(presence_data data)
{
    char *property_string_plain_addr = "{\"property_name\": \"%s\","
                                       "\"measurements\": ["
                                       "{ \"timestamp\":\"%d\","
                                       "\"value\":\"%s\"}]},";

    char *property_string_plain_isHome = "{\"property_name\": \"%s\","
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
//Uploads our data via the post_https of the generic firmware
void upload_presence_detection_data()
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    enable_wifi();
    char *msg_multiple_string_plain = "{\"upload_time\": \"%d\",\"property_measurements\":[ "
                                      "%s"
                                      "]}]}";

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    for (int i = 0; i < presence_addr_list_count; i++)
    {
        char *result_str = result_to_string(result_list[i]);
        int result_len = strlen(result_str) + 1;
        int multiple_property_measurements_size = result_len;
        char *multiple_property_measurements = malloc(multiple_property_measurements_size);
        strcpy(multiple_property_measurements, result_str);

        int msg_multiple_string_size = strlen(msg_multiple_string_plain) + sizeof(uint32_t) + multiple_property_measurements_size;
        char *msg_multiple_string = malloc(msg_multiple_string_size);
        snprintf(msg_multiple_string, msg_multiple_string_size, msg_multiple_string_plain, time(NULL), multiple_property_measurements);
        ESP_LOGI(TAG, "Payload: %s", msg_multiple_string);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        post_https(variable_interval_upload_url, msg_multiple_string, rootCA, bearer, NULL, 0);
        ESP_LOGI(TAG, "Waiting for next upload");
        free(multiple_property_measurements);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    disable_wifi();
}

//Our presence detection loop
void presence_detection_loop(void)
{
    rootCA = get_root_ca();
    initialize_presence_detection();
    bearer = get_bearer();
    // for (int i = 0; i < 10; i++)
    // {
    //     add_addr_to_target_list(phone);
    //     ESP_LOGI(TAG, "Addr Count: %d", presence_addr_list_count);
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
            presence_data res;
            memcpy(res.addr, presence_addr_list[requesting_number], sizeof(esp_bd_addr_t));
            ESP_LOGI(TAG, "Got final address %x%x%x%x%x%x", res.addr[0], res.addr[1], res.addr[2], res.addr[3], res.addr[4], res.addr[5]);
            if (found_after_stopped)
            {
                res.isHome = true;
            }
            else
            {
                res.isHome = false;
            }
            res.timeMeasured = time(NULL);
            result_list[requesting_number] = res;
            stopped = false;
            timeout_count = 0;
            requesting_number = 0;
            found_after_stopped = false;
            ESP_LOGI(TAG, "Finally Finishing up!");
            upload_presence_detection_data();
        }
        //Starting a measurement when we reach the specified timer interval
        if (measuring_interval_count >= MEASURING_INTERVAL)
        {
            start_requesting();
        }
        //Timedout so device isn't home
        if (requesting && timeout_count >= PRESENCE_NAME_REQ_TIMEOUT)
        {
            //We still have another device to scan
            if (requesting_number < (presence_addr_list_count - 1))
            {
                presence_data res;
                memcpy(res.addr, presence_addr_list[requesting_number], sizeof(esp_bd_addr_t));
                ESP_LOGI(TAG, "Got another address %x%x%x%x%x%x", res.addr[0], res.addr[1], res.addr[2], res.addr[3], res.addr[4], res.addr[5]);
                res.isHome = false;
                res.timeMeasured = time(NULL);
                result_list[requesting_number++] = res;
                ESP_LOGI(TAG, "Could not find number: %d", requesting_number - 1);
            }
            //No more devices to scan
            else
            {
                stop_requesting();
                presence_data res;
                memcpy(res.addr, presence_addr_list[requesting_number], sizeof(esp_bd_addr_t));
                ESP_LOGI(TAG, "Got no address %x%x%x%x%x%x", res.addr[0], res.addr[1], res.addr[2], res.addr[3], res.addr[4], res.addr[5]);
                res.isHome = false;
                res.timeMeasured = time(NULL);
                result_list[requesting_number] = res;
                upload_presence_detection_data();
                ESP_LOGI(TAG, "Stopping Requesting Early!");
            }
            timeout_count = 0;
        }
        //Stopping requesting because we have scanned all devices, giving the last one time
        //to respond which is why we set stopped, not uploading until last result is in.
        if (requesting && requesting_number >= presence_addr_list_count - 1)
        {
            stop_requesting();
            stopped = true;
            ESP_LOGI(TAG, "Stopping Requesting!");
        }
        //Only a very short delay needed here, can be longer if it interferes with other tasks ofcourse.
        vTaskDelay(10 / portTICK_PERIOD_MS);
        /* Blink off (output low) */
    }
}
