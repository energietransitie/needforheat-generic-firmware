#include "measurements.hpp"

namespace Measurements
{
    MeasurementValue::MeasurementValue(time_t timestamp, std::string value)
        : timestamp(timestamp), value(value) {}

    MeasurementValue::MeasurementValue(std::string value)
        : timestamp(time(nullptr)), value(value) {}

    std::string CreateRequestBodyVariable(const std::string propertyName, MeasurementValue measurementValue)
    {
        std::ostringstream filledRequestBody;

        time_t now = time(nullptr);

        filledRequestBody << "{\"upload_time\": \"" << now << "\","
                          << "\"property_measurements\": [{\"property_name\": \"" << propertyName << "\","
                          << "\"measurements\": ["
                          << "{\"timestamp\": \"" << measurementValue.timestamp << "\", \"value\": \"" << measurementValue.value << "\"}"
                          << "]}]}";

        return filledRequestBody.str();
    }
}
