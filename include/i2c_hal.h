#ifndef I2C_HAL_H
#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

#define I2C_SEND_STOP    true
#define I2C_SEND_NO_STOP false

void i2c_hal_init();
int32_t i2c_hal_read(void *handle, uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t size);
int32_t i2c_hal_write(void *handle, uint8_t address, uint8_t reg, const uint8_t *buffer, uint16_t size);

// function from needforheat source for compatibility
esp_err_t NFH_i2c_write(uint8_t address, uint8_t *buffer, uint8_t len, bool sendStop);
esp_err_t NFH_i2c_read(uint8_t address, uint8_t *buffer, uint8_t len);


#endif