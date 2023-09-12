#pragma once

#include <string>

#include <esp_bt_defs.h>
#include <presence_detection.hpp>
#include <esp_gap_bt_api.h>

namespace MACAddres
{
    void new_paired_device(esp_bt_gap_cb_param_t *param);
    esp_err_t addMacAddress(std::string newAddress);
    void get_bonded_devices(void);
}