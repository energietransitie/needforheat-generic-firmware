#include "nvs.hpp"

#include "error.hpp"

namespace NVS
{
    namespace
    {
        inline esp_err_t OpenNVS(const char *ns, nvs_open_mode_t om, nvs_handle_t &handle)
        {
            return nvs_open(ns, om, &handle);
        }

        inline esp_err_t CloseNVS(nvs_handle_t &handle)
        {
            auto err = nvs_commit(handle);
            nvs_close(handle);
            return err;
        }
    } // namespace

    esp_err_t Set(const char *ns, const char *key, const std::string &val)
    {
        nvs_handle_t handle;
        auto err = OpenNVS(ns, NVS_READWRITE, handle);
        if (err != ESP_OK)
            return err;

        err = nvs_set_str(handle, key, val.c_str());
        if (err != ESP_OK)
            return err;

        return CloseNVS(handle);
    }
    
    esp_err_t Set(const char *ns, const char *key, int32_t val)
    {
        nvs_handle_t handle;
        auto err = OpenNVS(ns, NVS_READWRITE, handle);
        if (err != ESP_OK)
            return err;

        err = nvs_set_i32(handle, key, val);
        if (err != ESP_OK)
            return err;

        return CloseNVS(handle);
    }

    esp_err_t Get(const char *ns, const char *key, std::string &outVal)
    {
        nvs_handle_t handle;
        auto err = OpenNVS(ns, NVS_READONLY, handle);
        if (err != ESP_OK)
            return err;

        size_t length;
        err = nvs_get_str(handle, key, nullptr, &length);
        if (err != ESP_OK)
            return err;

        // Allocate size for the string.
        outVal.resize(length);
        
        return nvs_get_str(handle, key, &outVal[0], &length);
    }

    esp_err_t Get(const char *ns, const char *key, int32_t &outVal)
    {
        nvs_handle_t handle;
        auto err = OpenNVS(ns, NVS_READONLY, handle);
        if (err != ESP_OK)
            return err;

        return nvs_get_i32(handle, key, &outVal);
    }

    esp_err_t Erase(const char *ns, const char *key)
    {
        nvs_handle_t handle;
        auto err = OpenNVS(ns, NVS_READWRITE, handle);
        if (err != ESP_OK)
            return err;

        err = nvs_erase_key(handle, key);
        if (err != ESP_OK)
            return err;
        
        return CloseNVS(handle);
    }
} // namespace NVS
