#include <secure_upload.hpp>

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cJSON.h>

#include <generic_esp_32.hpp>

constexpr const char *TAG = "Secure upload";

constexpr int UPLOAD_QUEUE_MAX = 50;

namespace SecureUpload
{
	Queue &Queue::GetInstance()
	{
		static Queue q;
		return q;
	}

	Queue::Queue()
		: m_measurementQueue(xQueueCreate(UPLOAD_QUEUE_MAX, sizeof(Measurements::Measurement *)))
	{
		ESP_LOGD(TAG, "Queue created.");
	}

	void Queue::AddMeasurement(const Measurements::Measurement &measurement)
	{
		auto measurementPtr = new Measurements::Measurement(std::move(measurement));

		auto err = xQueueSend(m_measurementQueue, &measurementPtr, 0);
		if (err != pdTRUE)
			ESP_LOGE(TAG, "An error occured when adding an item to the queue: no space.");
	}

	void Queue::Upload()
	{
		auto uploadObject = cJSON_CreateObject();

		// Add device upload time.
		auto uploadTime = cJSON_CreateNumber(time(nullptr));
		cJSON_AddItemToObject(uploadObject, "device_time", uploadTime);

		// Ddd measurements array
		auto measurements = cJSON_CreateArray();
		cJSON_AddItemToObject(uploadObject, "measurements", measurements);

		int measurementItems = 0;

		while (true)
		{
			auto item = new Measurements::Measurement();
			if (xQueueReceive(m_measurementQueue, &item, 0) != pdTRUE)
				break;

			cJSON_AddItemToArray(measurements, item->GetJSON());
			delete item;

			measurementItems++;
		}

		if (measurementItems == 0)
		{
			ESP_LOGD(TAG, "There were no measurements in the upload queue.");
			cJSON_Delete(uploadObject);
			return;
		}

		HTTPUtil::buffer_t dataSend = cJSON_Print(uploadObject);
		HTTPUtil::headers_t headersSend;
		HTTPUtil::buffer_t dataReceive;
		HTTPUtil::headers_t headersReceive;

		auto statusCode = GenericESP32Firmware::PostHTTPSToBackend(ENDPOINT_VARIABLE_UPLOAD, dataSend, headersSend, dataReceive, headersReceive, true);
		if (statusCode != 200)
		{
			ESP_LOGE(TAG, "Posting to server returned statuscode %d.", statusCode);
		}

		ESP_LOGD(TAG, "Backend server response body: \n%s", dataReceive.c_str());

		cJSON_Delete(uploadObject);
	}
} // namespace SecureUpload
