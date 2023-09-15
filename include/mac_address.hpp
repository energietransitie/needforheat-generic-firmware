#pragma once

#include <string>

#include <esp_bt_defs.h>
#include <presence_detection.hpp>
#include <esp_gap_bt_api.h>

namespace MACAddres
{
    std::string converter(uint8_t *str);
    void new_paired_device(esp_bt_gap_cb_param_t *param);
    esp_err_t addMacAddress(std::string newAddress, std::string newSmartphone);
    esp_err_t removeMacAddress(uint8_t id);
}