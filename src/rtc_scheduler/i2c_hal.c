#include "i2c_hal.h"
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>

void i2c_hal_init()
{
    // config
    i2c_config_t config =
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = GPIO_NUM_22,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 40000L
    };

    i2c_param_config(I2C_NUM_0,&config);

    i2c_driver_install(I2C_NUM_0,I2C_MODE_MASTER, 0, 0,ESP_INTR_FLAG_LEVEL3);
}

int32_t i2c_hal_read(void *handle, uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t size)
{
  i2c_cmd_handle_t cmd; 
    cmd = i2c_cmd_link_create();
    
    // --- create i2c command
    // select register address
    i2c_master_start(cmd); 
    i2c_master_write_byte(cmd, (address<<1)|I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    // read answer of device
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address<<1)|I2C_MASTER_READ, true);
    i2c_master_read(cmd, buffer, size, false);
    i2c_master_stop(cmd);

    // send i2c command
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 650);

    i2c_cmd_link_delete(cmd);
 return 0;
}    

int32_t i2c_hal_write(void *handle, uint8_t address, uint8_t reg, const uint8_t *buffer, uint16_t size)
{
  i2c_cmd_handle_t cmd; 
    cmd = i2c_cmd_link_create();
    
    // create i2c command
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address<<1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write(cmd,buffer,size,true);
    i2c_master_stop(cmd);

    // send i2c command
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 650);

    i2c_cmd_link_delete(cmd);
 return 0;
}