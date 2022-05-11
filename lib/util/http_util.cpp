#include "http_util.h"

extern "C" {
    #include <generic_esp_32.h>
}

namespace HTTPUtil
{
    namespace
    {
        const char *TAG = "HTTPUtil";

        headers_t bufferedHeaders;

        esp_err_t HTTPEventHandler(esp_http_client_event_t *evt)
        {
            switch (evt->event_id)
            {
            case HTTP_EVENT_ERROR:
                ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
                break;
            case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
                break;
            case HTTP_EVENT_HEADER_SENT:
                ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
                break;
            case HTTP_EVENT_ON_HEADER:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
                bufferedHeaders[evt->header_key] = evt->header_value;
                break;
            case HTTP_EVENT_ON_DATA:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
                break;
            case HTTP_EVENT_ON_FINISH:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
                break;
            case HTTP_EVENT_DISCONNECTED:
                ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
                break;
            }
            return ESP_OK;
        }
    }

    int HTTPRequest(esp_http_client_config_t config, headers_t &headersReceive, buffer_t &dataReceive, headers_t headersSend, buffer_t dataSend)
    {
        esp_err_t err = ESP_OK;

        char *endpoint = const_cast<char *>("HTTPUtil::HTTPRequest");
        
        wait_for_wifi(endpoint);

        // Make sure there are no headers buffered.
        bufferedHeaders.clear();

        // Attach HTTPEventHandler. Only used to capture received headers.
        config.event_handler = HTTPEventHandler;

        auto client = esp_http_client_init(&config);

        for (auto const &header : headersSend)
        {
            err = esp_http_client_set_header(client, header.first.c_str(), header.second.c_str());
            Error::CheckAppendName(err, TAG, "An error occured when setting header");
        }

        err = esp_http_client_open(client, dataSend.size());
        if (Error::CheckAppendName(err, TAG, "An error occured when opening the HTTP connection"))
        {
            esp_http_client_cleanup(client);
            disconnect_wifi(endpoint);
            return 400;
        }

        if (dataSend.size() > 0)
        {
            auto bytesSent = esp_http_client_write(client, dataSend.data(), dataSend.size());
            if (bytesSent < dataSend.size())
            {
                ESP_LOGE(TAG, "Not all data was written.");
                esp_http_client_cleanup(client);
                disconnect_wifi(endpoint);
                return 400;
            }
        }

        auto contentLength = esp_http_client_fetch_headers(client);
        if (contentLength < 0)
        {
            ESP_LOGE(TAG, "An error occured when fetching headers: %s", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            disconnect_wifi(endpoint);
            return 400;
        }

        // Reserve memory for response + null-termination char.
        dataReceive.resize(contentLength + 1);

        auto bytesReceived = esp_http_client_read_response(client, &dataReceive[0], contentLength);

        // Append null-termination char.
        // dataReceive.emplace_back('\0');

        if (bytesReceived != contentLength)
        {
            ESP_LOGE(TAG, "An error occured when reading response. Expected %d but received %d", contentLength, bytesReceived);
            esp_http_client_cleanup(client);
            disconnect_wifi(endpoint);
            return 400;
        }

        // copy buffered headers from HTTPEventHandler.
        headersReceive = bufferedHeaders;

        auto statusCode = esp_http_client_get_status_code(client);

        err = esp_http_client_cleanup(client);
        Error::CheckAppendName(err, TAG, "An error occured when cleaning up HTTP client");

        disconnect_wifi(endpoint);

        return statusCode;
    }

    int HTTPRequest(esp_http_client_config_t config, headers_t &headersReceive, buffer_t &dataReceive, headers_t headersSend)
    {
        buffer_t data;

        return HTTPRequest(config, headersReceive, dataReceive, headersSend, data);
    }

    int HTTPRequest(esp_http_client_config_t config, headers_t &headersReceive, buffer_t &dataReceive)
    {
        buffer_t data;
        headers_t headers;

        return HTTPRequest(config, headersReceive, dataReceive, headers, data);
    }
}
