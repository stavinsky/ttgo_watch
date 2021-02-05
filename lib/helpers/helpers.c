#include "helpers.h"
#include "tt_config.h"
#include "esp_err.h"


esp_err_t i2c_master_read_slave_reg(
        i2c_port_t i2c_num,
        uint8_t i2c_addr,
        uint8_t i2c_reg,
        uint8_t* data_rd,
        size_t size
) {
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( i2c_addr << 1 ), true);
    i2c_master_write_byte(cmd, i2c_reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( i2c_addr << 1 ) | I2C_MASTER_READ, true);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}



esp_err_t i2c_master_write_slave_reg(
        i2c_port_t i2c_num,
        uint8_t i2c_addr,
        uint8_t i2c_reg,
        uint8_t* data_wr,
        size_t size
) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( i2c_addr << 1 ) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, i2c_reg, true);
    i2c_master_write(cmd, data_wr, size, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
