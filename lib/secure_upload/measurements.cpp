#include "measurements.hpp"

constexpr const char *TAG = "Measurements";

namespace Measurements
{
	std::unordered_map<std::string, std::string> Measurement::m_formatStrings;

	void Measurement::AddFormatter(const std::string &propertyName, const std::string &formatString)
	{
		m_formatStrings[propertyName] = formatString;
	}

	cJSON *Measurement::GetJSON()
	{
		// Create the object that eventually gets added to "property_measurements".
		auto propertyMeasurementObject = cJSON_CreateObject();

		// Add the property name to the object.
		auto propertyName = cJSON_CreateString(m_propertyName.c_str());
		cJSON_AddItemToObject(propertyMeasurementObject, "property_name", propertyName);

		// Add a measurements array to the object.
		auto measurements = cJSON_CreateArray();
		cJSON_AddItemToObject(propertyMeasurementObject, "measurements", measurements);

		// Add a new measurement object to the measurements array.
		auto measurement = cJSON_CreateObject();
		cJSON_AddItemToArray(measurements, measurement);

		// Add the measurement timestamp to the measurement object.
		auto timestamp = cJSON_CreateNumber(m_timestamp);
		cJSON_AddItemToObject(measurement, "timestamp", timestamp);

		// Add the measurement value to the measurement object.
		auto value = cJSON_CreateString(m_value.c_str());
		cJSON_AddItemToObject(measurement, "value", value);

		return propertyMeasurementObject;
	}
} // namespace Measurements
