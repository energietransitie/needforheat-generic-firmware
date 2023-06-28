#include <measurements.hpp>

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
		// Create the object that eventually gets added to "measurements".
		auto measurementObject = cJSON_CreateObject();

		// Add the property object.
		auto propertyObject = cJSON_CreateObject();
		cJSON_AddItemToObject(measurementObject, "property", propertyObject);

		// Add the property name to the property object.
		auto propertyName = cJSON_CreateString(m_propertyName.c_str());
		cJSON_AddItemToObject(propertyObject, "name", propertyName);

		// Add the measurement timestamp to the measurement object.
		auto timestamp = cJSON_CreateNumber(m_timestamp);
		cJSON_AddItemToObject(measurementObject, "time", timestamp);

		// Add the measurement value to the measurement object.
		auto value = cJSON_CreateString(m_value.c_str());
		cJSON_AddItemToObject(measurementObject, "value", value);

		return measurementObject;
	}
} // namespace Measurements
