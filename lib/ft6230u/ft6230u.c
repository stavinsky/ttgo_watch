#include "ft6230u.h"
#include "esp_log.h"
#include <stdlib.h>
#include "i2c.h"

static const char* TAG = "axp202";


static esp_err_t register_read(uint8_t reg, uint8_t *result) {
  esp_err_t ret = ESP_OK;
  ret = i2c_master_read_slave_reg(I2C_NUM_1, 0x38, reg, result, 1);
  return ret;
}



void get_touch(uint8_t* data){
    esp_err_t ret = ESP_OK;
    ret = i2c_master_read_slave_reg(I2C_NUM_1, 0x38, 0x01, data, 14);
    if (ret != ESP_OK){
      ESP_LOGE(TAG, "Can't read from device");
    }
}

enum gestures get_gesture(uint8_t* data){
    return data[0];
}

uint8_t num_points(uint8_t* data){
    return data[1] & 0x0F;
}
