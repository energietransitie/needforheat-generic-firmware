#include <i2c_hal.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>
#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#include <esp_log.h>

void i2c_hal_init() {
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

int32_t i2c_hal_read(void *handle, uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t size) {
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

int32_t i2c_hal_write(void *handle, uint8_t address, uint8_t reg, const uint8_t *buffer, uint16_t size) {
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

// i2c write function copied from twomes-room-monitor-firmware firmware to be compatible with scd41 library
esp_err_t NFH_i2c_write(uint8_t address, uint8_t *buffer, uint8_t len, bool sendStop) {
    //Setup i2c communication:
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(i2c_cmd);
    ESP_LOGD("I2C", "Master start returned %s", esp_err_to_name(err));
    //Write the address and command to the i2c output buffer:
    err = i2c_master_write_byte(i2c_cmd, (address << 1) | I2C_MASTER_WRITE, true);
    ESP_LOGD("I2C", "Master write byte returned %s", esp_err_to_name(err));
    for (uint8_t i = 0; i < len; i++) {
        i2c_master_write_byte(i2c_cmd, buffer[i], 1);
    }

    //End transmission (if stop is enabled):
    if (sendStop) i2c_master_stop(i2c_cmd);

    //Begin the command and return the esp_err_t code:
    err = i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd, 650);
    ESP_LOGD("I2C", "Master cmd begin returned %s", esp_err_to_name(err));
    //Clear i2c resources:
    i2c_cmd_link_delete(i2c_cmd);

    return err;
}

// i2c read function copied from twomes-room-monitor-firmware firmware to be compatible with scd41 library
esp_err_t NFH_i2c_read(uint8_t address, uint8_t *buffer, uint8_t len) {
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (address << 1) | I2C_MASTER_READ, true);
    for (uint8_t i = 0; i < (len - 1); i++) {  //i<(buffer-1) to send NACK on last read
        i2c_master_read_byte(i2c_cmd, &buffer[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(i2c_cmd, &buffer[len - 1], I2C_MASTER_NACK);
    i2c_master_stop(i2c_cmd);
    return i2c_master_cmd_begin(I2C_NUM_0, i2c_cmd, 650);
}