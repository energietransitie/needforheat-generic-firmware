#include <mac_address.hpp>
#include <presence_detection.hpp>

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

    std::string converter(uint8_t *str){
    return std::string((char *)str);
    }

    void addOnboardedSmartphone(esp_bt_gap_cb_param_t *param)
    {
        char bda_str[MACAddressLength];
        ESP_LOGI(TAG, "Paired with MAC address: %s", bda2str(param->disc_res.bda, bda_str, 18));
        
        ESP_LOGI(TAG, "Paired with Bluetooth name: %s", converter(param->auth_cmpl.device_name).c_str());
        addOnboardedSmartphoneToNVS(bda2str(param->disc_res.bda, bda_str, 18), converter(param->auth_cmpl.device_name));
    }

    esp_err_t addOnboardedSmartphoneToNVS(std::string newMacAddress, std::string newBluetoothName)
    {
        bool doubleMacAddress = false, empty = false;
        std::string macAddressList, smartphoneList;
        auto err = NVS::Get(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY, smartphoneList);
        err = NVS::Get(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY, macAddressList);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            empty = true;
        }

        if(!empty)
        {
            auto macAddressStrings = Strings::Split(macAddressList, ';');

            // loop true all current MAC adresses
            for (const auto &existingMacAddress : macAddressStrings)
            {
                if(existingMacAddress.compare(newMacAddress) == 0)
                {
                    ESP_LOGW(TAG, "MAC address is already stored.");
                    doubleMacAddress = true;
                }
            }
        }
        

        //if the new MAC address is not found or there are no MAC adresses add the new address to nvs
        if (!doubleMacAddress || empty)
        {   
            if (empty)
            {
                err = NVS::Set(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY, newMacAddress);
                err = NVS::Set(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY, newBluetoothName);
            }
            else
            {
                err = NVS::Set(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY, macAddressList + ";" + newMacAddress);
                err = NVS::Set(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY, smartphoneList + ";" + newBluetoothName);
            }

            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "An error occured when adding MAC-addresses to NVS.");
                return err;
            }
        }

        return ESP_OK;
    }

    esp_err_t deleteOnboardedSmartphoneFromNVS(uint8_t id)
    {
        bool doubleMacAddress = false, empty = false;
        std::string macAddressList, smartphoneList, newMacAddressList, newSmartphoneList;
        uint8_t count = 0;
        auto err = NVS::Get(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY, smartphoneList);
        err = NVS::Get(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY, macAddressList); 
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            empty = true;
        }

        if(!empty)
        {
            auto macAddressStrings = Strings::Split(macAddressList, ';');

            // loop trough all current MAC adresses to create a new list without the selected address
            for (const auto &address : macAddressStrings)
            {
               
                
                if((count != id && count <= 0) || (id == 0 && count == 1))
                {
                    newMacAddressList += address;
                }
                else if(count != id && count > 0)
                {
                    newMacAddressList += (";" + address);
                }
                
                count++;
            }

            count = 0;
            auto smartphoneStrings = Strings::Split(smartphoneList, ';');

            //loop trough all smartphones to create a new list without the selected phone
            for (const auto &smartphone : smartphoneStrings)
            {
                if((count != id && count <= 0) || (id == 0 && count == 1))
                {
                    newSmartphoneList += smartphone;
                }
                else if (count != id && count > 0)
                {
                    newSmartphoneList += (";" + smartphone);
                }
               
                
                count++;
            }
        
        NVS::Erase(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY);
        NVS::Erase(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY); 

        //When theres no phones left dont's set nvs, otherwise if we set it without value it will still think that it's not empty
        if(newSmartphoneList.compare("") != 0)
        {
        err = NVS::Set(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY, newSmartphoneList);
        err = NVS::Set(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY, newMacAddressList);
        }
    
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "An error occured when adding MAC-addresses to NVS.");
            return err;
        }

        }

        return ESP_OK;
    }
}
