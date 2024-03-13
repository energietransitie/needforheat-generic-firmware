#include <measurements.hpp>

#include <cstring>

constexpr const char *TAG = "Measurements";

namespace Measurements
{
	namespace
	{
		/**
		 * Serialize a variable to a file.
		 *
		 * @param input variable to serialize.
		 * @param file File to serialize to.
		 */
		template <typename T>
		esp_err_t Serialize(const T &input, std::FILE *file)
		{
			auto written = std::fwrite(&input, sizeof(T), 1, file);
			if (written != 1)
				return ESP_ERR_INVALID_SIZE;

			return ESP_OK;
		}

		/**
		 * Serialize a variable to a file.
		 * This is a specialization for std::string.
		 */
		template <>
		esp_err_t Serialize<std::string>(const std::string &input, std::FILE *file)
		{
			// Serialize the size of the string.
			auto strSize = input.size();
			auto err = Serialize(strSize, file);
			if (err != ESP_OK)
				return err;

			if (strSize <= 0)
				return ESP_OK;

			// Serialize the string itself.
			auto written = std::fwrite(&input[0], sizeof(char), strSize, file);
			if (written != strSize)
				return ESP_ERR_INVALID_SIZE;

			return ESP_OK;
		}

		/**
		 * Deserialize a variable from a file.
		 *
		 * @param input variable to deserialize to.
		 * @param file File to deserialize from.
		 */
		template <typename T>
		esp_err_t Deserialize(T &input, std::FILE *file)
		{
			auto read = std::fread(&input, sizeof(T), 1, file);
			if (read != 1)
				return ESP_ERR_INVALID_SIZE;

			return ESP_OK;
		}

		/**
		 * Deserialize a variable from a file.
		 * This is a specialization for std::string.
		 */
		template <>
		esp_err_t Deserialize<std::string>(std::string &input, std::FILE *file)
		{
			// Deserialize the size of the string.
			std::size_t strSize = 0;
			auto err = Deserialize(strSize, file);
			if (err != ESP_OK)
				return err;

			input.clear();

			if (strSize <= 0)
				return ESP_OK;

			input.resize(strSize);
			
			// Deserialize the string itself.
			auto read = std::fread(&input[0], sizeof(char), strSize, file);
			if (read != strSize)
				return ESP_ERR_INVALID_SIZE;

			return ESP_OK;
		}
	} // namespace

	std::unordered_map<std::string, std::string> Measurement::m_formatStrings;

	void Measurement::AddFormatter(const std::string &propertyName, const std::string &formatString)
	{
		m_formatStrings[propertyName] = formatString;
	}

	Measurement::Measurement(Measurement &&other)
		: m_propertyName(std::move(other.m_propertyName)),
		  m_timestamp(std::move(other.m_timestamp)),
		  m_value(std::move(other.m_value)) {}

	cJSON *Measurement::GetJSON() const
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

	esp_err_t SerializeMeasurement(const Measurement &measurement, FILE *file)
	{
		auto err = Serialize(measurement.m_propertyName, file);
		if (err != ESP_OK)
			return err;

		err = Serialize(measurement.m_timestamp, file);
		if (err != ESP_OK)
			return err;

		err = Serialize(measurement.m_value, file);
		if (err != ESP_OK)
			return err;

		return ESP_OK;
	}

	std::pair<Measurement, esp_err_t> DeserializeMeasurement(FILE *file)
	{
		Measurement measurement;

		auto err = Deserialize(measurement.m_propertyName, file);
		if (err != ESP_OK)
			return std::make_pair(Measurement(), ESP_ERR_INVALID_SIZE);

		err = Deserialize(measurement.m_timestamp, file);
		if (err != ESP_OK)
			return std::make_pair(Measurement(), ESP_ERR_INVALID_SIZE);

		std::string value;
		err = Deserialize(measurement.m_value, file);
		if (err != ESP_OK)
			return std::make_pair(Measurement(), ESP_ERR_INVALID_SIZE);

		return std::make_pair(measurement, ESP_OK);
	}
} // namespace Measurements
