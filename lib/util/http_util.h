#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <string>
#include <map>

#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_tls.h>
#include <esp_event.h>
#include <esp_ota_ops.h>
#include <esp_https_ota.h>

#include "certs.h"
#include "error.h"

namespace HTTPUtil
{
    using buffer_t = std::string;
    using headers_t = std::map<std::string, std::string>;

    /**
     * Performs an HTTP request, sends and receives data and headers.
     *
     * @param config The HTTP client config.
     * @param headersReceive Map to save received headers in.
     * @param dataReceive Buffer to save received data in.
     * @param headersSend Map that contains headers to send.
     * @param dataSend Buffer that contains the data to send.
     *
     * @returns HTTP status code.
     */
    int HTTPRequest(esp_http_client_config_t config, headers_t &headersReceive, buffer_t &dataReceive, headers_t headersSend, buffer_t dataSend);

    /**
     * Performs an HTTP request and receives data and headers.
     *
     * @param config The HTTP client config.
     * @param headersReceive Map to save received headers in.
     * @param dataReceive Buffer to save received data in.
     * @param headersSend Map that contains headers to send.
     *
     * @returns HTTP status code.
     */
    int HTTPRequest(esp_http_client_config_t config, headers_t &headersReceive, buffer_t &dataReceive, headers_t headersSend);

    /**
     * Performs an HTTP request and receives data and headers.
     *
     * @param config The HTTP client config.
     * @param headersReceive Map to save received headers in.
     * @param dataReceive Buffer to save received data in.
     *
     * @returns HTTP status code.
     */
    int HTTPRequest(esp_http_client_config_t config, headers_t &headersReceive, buffer_t &dataReceive);
}

#endif // HTTP_UTIL_H
