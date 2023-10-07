#include <specific_m5coreink/battery_voltage.hpp>

#include <driver/adc.h>
#include <esp_adc_cal.h>

#include <secure_upload.hpp>
#include <measurements.hpp>

constexpr const char *BATTERY_MEASUREMENT_PROPERTY_NAME = "battery_voltage__V";

namespace M5CoreInkSpecific
{
    namespace
    {
        static bool s_initialized = false;
        static esp_adc_cal_characteristics_t s_adcChars;

        auto secureUploadQueue = SecureUpload::Queue::GetInstance();
    } // namespace

    void BatteryVoltageTask(void *taskInfo)
    {
        if (!s_initialized)
        {
            // Setup ADC1 channel 7 (gpio35).
            adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_11db);

            // Calculate characteristic of adc1.
            esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3600, &s_adcChars);

            // Add a formatter for the batteryVoltage property.
            Measurements::Measurement::AddFormatter(BATTERY_MEASUREMENT_PROPERTY_NAME, "%.2f");

            s_initialized = true;
        }

        // Measure battery voltage.
        auto rawMeasurement = adc1_get_raw(ADC1_CHANNEL_7);
        auto rawVoltage_mV = esp_adc_cal_raw_to_voltage(rawMeasurement, &s_adcChars);
        auto batteryVoltage = ((float)rawVoltage_mV) * 25.1 / 5.1 / 1000;

        // Send data to queue.
        Measurements::Measurement batteryVoltageMeasurement(BATTERY_MEASUREMENT_PROPERTY_NAME, batteryVoltage);
        secureUploadQueue.AddMeasurement(batteryVoltageMeasurement);
    }
} // namespace M5CoreInkSpecific
