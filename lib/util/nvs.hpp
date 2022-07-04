#ifndef NVS_HPP
#define NVS_HPP

#include <cstdint>
#include <string>

#include <esp_err.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

namespace NVS
{
    /**
     * Set string value for given key in NVS.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     * @param val Value to set.
     *
     * @returns ESP error.
     */
    esp_err_t Set(const char *ns, const char *key, const std::string &val);

    /**
     * Set int32_t (int) value for given key in NVS.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     * @param val Value to set.
     *
     * @returns ESP error.
     */
    esp_err_t Set(const char *ns, const char *key, int32_t val);

    /**
     * Get string value for given key from NVS.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     * @param outVal Read value.
     *
     * @returns ESP error.
     */
    esp_err_t Get(const char *ns, const char *key, std::string &outVal);

    /**
     * Get int32_t (int) value for given key from NVS.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     * @param outVal Read value.
     *
     * @returns ESP error.
     */
    esp_err_t Get(const char *ns, const char *key, int32_t &outVal);

    /**
     * Erase key value pair of given key.
     *
     * @param ns NVS namespace.
     * @param key Key to erase.
     *
     * @returns ESP error.
     */
    esp_err_t Erase(const char *ns, const char *key);

    /**
     * Get value for given key from NVS. If the key is not found, it is created and initialized to the type's zero-value.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     * @param outVal Read value.
     *
     * @returns ESP error.
     */
    template <typename T>
    esp_err_t GetOrInit(const char *ns, const char *key, T &outVal)
    {
        T value;
        auto err = Get(ns, key, value);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            value = T();

            ESP_LOGD("NVSUtil", "Key \"%s\" was not found. Initializing it to its zero-value.", key);

            outVal = value;
            return Set(ns, key, value);
        }

        return err;
    }

    /**
     * Increment value at the given key.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     *
     * @returns ESP error.
     */
    template <typename T>
    esp_err_t Increment(const char *ns, const char *key)
    {
        T value;
        auto err = Get(ns, key, value);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGD("NVSUtil", "Key \"%s\" was not found for incrementing. Initializing it to 0 and incrementing it.", key);
            value = T(0);
        }
        else if (err != ESP_OK)
        {
            return err;
        }

        value++;

        return Set(ns, key, value);
    }

    /**
     * Decrement value at the given key.
     *
     * @param ns NVS namespace.
     * @param key Key to set.
     *
     * @returns ESP error.
     */
    template <typename T>
    esp_err_t Decrement(const char *ns, const char *key)
    {
        T value;
        auto err = Get(ns, key, value);
        if (err != ESP_OK)
            return err;

        value--;

        return Set(ns, key, value);
    }
}

#endif // NVS_HPP
