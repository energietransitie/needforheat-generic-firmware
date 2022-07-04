#include "error.hpp"

namespace Error
{
    bool Check(esp_err_t err, const char *TAG)
    {
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "An err occured: %s", esp_err_to_name(err));
            return true;
        }

        return false;
    }

    bool CheckAppendName(esp_err_t err, const char *TAG, const char *message)
    {
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "%s: %s", message, esp_err_to_name(err));
            return true;
        }

        return false;
    }
}
