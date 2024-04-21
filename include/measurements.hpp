#pragma once

#include <string>
#include <unordered_map>

#include <esp_log.h>

#include <cJSON.h>

#include <scheduler.hpp>
#include <util/format.hpp>

namespace Measurements
{
	class Measurement
	{
	public:
		/**
		 * Define a formatting string for a specific property name.
		 *
		 * @param propertyName The name of the property.
		 * @param formatString The format string used to format the measurement value.
		 */
		static void AddFormatter(const std::string &propertyName, const std::string &formatString);

		/**
		 * Construct a new measurement.
		 *
		 * Use Measurement(propertyName, value) to create a new measurement with data.
		 */
		Measurement() = default;

		/**
		 * Construct a new measurement.
		 *
		 * A custom time will be used as a custom timestamp.
		 *
		 * @param property The property type of the measurement.
		 * @param value The value of the measurement.
		 * @param timestamp The timestamp to set on the measurement.
		 */
		template <typename T>
		Measurement(const std::string &propertyName, T value, time_t timestamp)
			: m_propertyName(propertyName)
		{
			auto formatString = m_formatStrings.find(propertyName);
			if (formatString == m_formatStrings.end())
			{
				ESP_LOGE("Measurements",
						 "An error occured when creating a new measurement: "
						 "a measurement with property \"%s\" was created, "
						 "but no format string was found.",
						 propertyName.c_str());
				return;
			}

			m_value = Format::String(formatString->second, value);

			// If 0 is passed into the timestamp, use currentTaskTime.
			if (timestamp == 0)
			{
				m_timestamp = Scheduler::GetCurrentTaskTime();
			}
			else
			{
				m_timestamp = timestamp;
			}
		}

		/**
		 * Construct a new measurement.
		 *
		 * The currentTaskTime will be used as a timestamp.
		 *
		 * @param property The property type of the measurement.
		 * @param value The value of the measurement.
		 */
		template <typename T>
		Measurement(const std::string &propertyName, T value)
			: Measurement(propertyName, value, 0) {}

		/**
		 * Create a JSON object that can be used to send to the server API.
		 *
		 * @returns JSON object of the property.
		 */
		cJSON *GetJSON();

	private:
		/**
		 * This map holds all the format strings for specific property types.
		 *
		 * When creating a new measurement, the propertyName is found in this map
		 * and the format string is used to format the value.
		 */
		static std::unordered_map<std::string, std::string> m_formatStrings;

		/**
		 * The property name of this measurement.
		 */
		std::string m_propertyName;

		/**
		 * Timestamp of the measurement.
		 */
		time_t m_timestamp;

		/**
		 * Value of the measurement, already formatted.
		 */
		std::string m_value;
	};
} // namespace Measurements
