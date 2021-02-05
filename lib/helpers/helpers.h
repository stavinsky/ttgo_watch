#pragma once
#include <driver/i2c.h>


#define BCD_4_HIGH(val) ((val & 0xF0)>>4)
#define BCD_4_LOW(val) (val & 0x0F)
#define BCD_INT(val) ((BCD_4_HIGH(val) * 10) + BCD_4_LOW(val))

esp_err_t i2c_master_read_slave_reg(
        i2c_port_t i2c_num,
        uint8_t i2c_addr,
        uint8_t i2c_reg,
        uint8_t* data_rd,
        size_t size
);

esp_err_t i2c_master_write_slave_reg(
        i2c_port_t i2c_num,
        uint8_t i2c_addr,
        uint8_t i2c_reg,
        uint8_t* data_wr,
        size_t size
);

