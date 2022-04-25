#ifndef ERROR_H
#define ERROR_H

#include <esp_err.h>
#include <esp_log.h>

namespace Error
{
    /**
     * Check esp_err_t and log if not ESP_OK.
     *
     * @param err ESP error.
     * @param TAG TAG to use for logging.
     *
     * @returns True if error occured.
     */
    bool Check(esp_err_t err, const char *TAG);

    /**
     * Check esp_err_t and log if not ESP_OK.
     *
     * Append ": <error name>" to message.
     *
     * @param err ESP error.
     * @param TAG TAG to use for logging.
     * @param message Message to display on error.
     *
     * @returns True is error occured.
     */
    bool CheckAppendName(esp_err_t err, const char *TAG, const char *message);
}

#endif // ERROR_H
