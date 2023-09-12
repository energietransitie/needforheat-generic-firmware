#include <mac_address.hpp>

#include <cstdlib>
#include <vector>
#include <set>
#include <regex>
#include <string>

#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <scheduler.hpp>
#include <measurements.hpp>
#include <secure_upload.hpp>
#include <util/error.hpp>
#include <util/delay.hpp>
#include <util/format.hpp>
#include <util/nvs.hpp>
#include <util/strings.hpp>

#define MACAddressLength 18

constexpr const char *NVS_NAMESPACE = "twomes_storage";
constexpr const char *TAG = "MAC Address";

namespace MACAddres
{

    static char *bda2str(esp_bd_addr_t bda, char *str, size_t size)
    {
        if (bda == NULL || str == NULL || size < 18) {
            return NULL;
        }

        uint8_t *p = bda;
        sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
                p[0], p[1], p[2], p[3], p[4], p[5]);
        return str;
    }

    void new_paired_device(esp_bt_gap_cb_param_t *param)
    {
        char bda_str[MACAddressLength];
        ESP_LOGI(TAG, "Device found: %s", bda2str(param->disc_res.bda, bda_str, 18));
        //PresenceDetection::AddMacAddress(param->disc_res.bda);

        //macaddress nvs

        addMacAddress(bda2str(param->disc_res.bda, bda_str, 18));
    }

    esp_err_t addMacAddress(std::string newAddress)
    {
        bool doubleMACAddress = false, empty = false;
        std::string macAddressList;
        auto err = NVS::Get(NVS_NAMESPACE, "mac_addresses", macAddressList);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            empty = true;
        }

        if(!empty)
        {
            auto macAddressStrings = Strings::Split(macAddressList, ';');

            // loop true all current MAC adresses
            ESP_LOGI(TAG, "Stored adresses:");
            for (const auto &address : macAddressStrings)
            {
                ESP_LOGI(TAG, "%s", address.c_str());
                if(address.compare(newAddress) == 0)
                {
                    ESP_LOGI(TAG, "MAC address is already stored.");
                    doubleMACAddress = true;
                }
            }
        }
        

        //if the new MAC address is not found or there are no MAC adresses add the new address to nvs
        if (!doubleMACAddress || empty)
        {   
            auto err = NVS::Set(NVS_NAMESPACE, "mac_addresses", macAddressList + ";" + newAddress);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "An error occured when adding MAC-addresses to NVS.");
                return err;
            }
        }

        // Addresses will be added after startup when presencedetection will do the initialisation again.
        // PresenceDetection::InitializeMacAddresses();

        return ESP_OK;
    }
}
