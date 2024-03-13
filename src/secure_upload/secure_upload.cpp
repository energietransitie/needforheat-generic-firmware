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

	void Queue::AddMeasurement(const Measurements::Measurement &measurement)
	{
		m_measurements.Add(measurement);
	}

	void Queue::Upload()
	{
		auto items = m_measurements.GetItems();

		if (items.size() == 0)
		{
			ESP_LOGD(TAG, "There were no measurements in the upload queue.");
			return;
		}

		auto uploadObject = cJSON_CreateObject();

		// Add device upload time.
		auto uploadTime = cJSON_CreateNumber(time(nullptr));
		cJSON_AddItemToObject(uploadObject, "device_time", uploadTime);

		// add property_measurements array
		auto propertyMeasurements = cJSON_CreateArray();
		cJSON_AddItemToObject(uploadObject, "property_measurements", propertyMeasurements);
		
		for (const auto &item: items)
		{
			cJSON_AddItemToArray(propertyMeasurements, item->GetJSON());
			delete item;
		}

		HTTPUtil::buffer_t dataSend = cJSON_Print(uploadObject);
		HTTPUtil::headers_t headersSend;
		HTTPUtil::buffer_t dataReceive;
		HTTPUtil::headers_t headersReceive;

		auto statusCode = GenericESP32Firmware::PostHTTPSToBackend(ENDPOINT_VARIABLE_UPLOAD, dataSend, headersSend, dataReceive, headersReceive, true);

		cJSON_Delete(uploadObject);

		ESP_LOGD(TAG, "Backend server response body: \n%s", dataReceive.c_str());

		if (statusCode != 200)
		{
			ESP_LOGE(TAG, "Posting to backend returned statuscode %d.", statusCode);
			return;
		}

		// Clear SPIFFS to avoid resending items in the future.
		m_measurements.EraseItems();
	}
} // namespace SecureUpload
