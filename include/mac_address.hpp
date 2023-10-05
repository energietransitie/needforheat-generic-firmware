#pragma once

#include <string>

#include <esp_bt_defs.h>
#include <presence_detection.hpp>
#include <esp_gap_bt_api.h>

namespace MACAddres
{
    std::string converter(uint8_t *str);
    void addOnboardedSmartphone(esp_bt_gap_cb_param_t *param);
    esp_err_t addOnboardedSmartphoneToNVS(std::string newMacAddress, std::string newBluetoothName);
    esp_err_t deleteOnboardedSmartphoneFromNVS(uint8_t id);
}