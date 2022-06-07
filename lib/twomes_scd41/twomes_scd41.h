/**
 * Library for reading the SCD41 CO2 sensor on the Twomes room sensor node
 *
 * Author: Sjors
 * Date: July 2021
 */

#ifndef _TWOMES_CO2SENSOR_H
#define _TWOMES_CO2SENSOR_H

#include "driver/i2c.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <i2c_hal.h>

#define LOG_LOCAL_LEVEL ESP_LOG_WARN

#define SCD41_INIT_DELAY_MS 1000        // milliseconds
#define SCD41_WAIT_MS 2                 // milliseconds
#define SCD41_SINGLE_SHOT_DELAY_MS 5000 // ms

#define SCD41_ADDR 0x62



void co2_init(uint8_t address);

/**
 * CRC8 code taken from Sensirion SCD41 Datasheet: https://nl.mouser.com/datasheet/2/682/Sensirion_CO2_Sensors_SCD4x_Datasheet-2321195.pdf
 * @brief calculate the CRC for an SCD41 I2C message
 *
 * @param data pointer to the received i2c data
 * @param count size of the buffer
 *
 * @return calculated CRC8
 */
uint8_t scd41_crc8(const uint8_t *data, uint16_t count);

/**
 * @brief read the serial number of the CO2 sensor
 *
 * @param address i2c address of the device
 *
 * @return the serial number
 */
uint64_t co2_get_serial(uint8_t address);

/**
 * @brief disable the SCD41 ASC (Automatic Self Calibration)
 * Device performs a read immeadiately after to check for disable success
 * @param address i2c address of the device
 *
 * @return value of ASC after write (0 == success)
 */
uint8_t co2_disable_asc(uint8_t address);

/**
 * send singleshot command, sleep for the conversion time and read the output \n
 * SLEEPS FOR 5 SECONDS IN LIGHT SLEEP MODE \n
 *
 * @param address i2c address
 * @param buffer buffer to hold the read data in (uint16_t[3])
 */
void co2_read(uint8_t address, uint16_t *buffer);

/**
 * @brief convert raw temp value to degrees Celsius
 *
 * @param raw the raw temperature value
 *
 * @return temperature in degrees Celsius
 */
float scd41_temp_raw_to_celsius(uint16_t);

/**
 * @brief convert raw RH value to humidity percentage
 *
 * @param raw the raw RH value
 *
 * @return Humidity in percent
 */
float scd41_rh_raw_to_percent(uint16_t raw);

#endif //_TWOMES_CO2SENSOR_H