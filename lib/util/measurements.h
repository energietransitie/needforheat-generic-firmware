#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <string>
#include <sstream>
#include <vector>

namespace Measurements
{
    /**
     * Holds a single measurementValue.
     */
    struct MeasurementValue
    {
        time_t timestamp;
        std::string value;

        MeasurementValue() {};

        /**
         * Constructor to set timestamp and value.
         */
        MeasurementValue(time_t timestamp, std::string value);

        /**
         * Constructor to set value. Timestamp is automatically set to time of creation.
         */
        MeasurementValue(std::string value);
    };

    /**
     * Create a body to use in request when sending measurement data.
     *
     * @param propertyName Name of the property.
     * @param measurementvalue A single measurementvalue.
     *
     * @returns A filled request body.
     */
    std::string CreateRequestBodyVariable(const std::string propertyName, MeasurementValue measurementValue);
}

#endif // MEASUREMENTS_H
