#include "axp202.h"
#include "esp_log.h"

static const char* TAG = "axp202";

static esp_err_t read_register(uint8_t reg, uint8_t *result){
    esp_err_t ret = ESP_OK;
    ret = i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, reg, result, 1);
    return ret;
}
static esp_err_t write_register(uint8_t reg, uint8_t value){
    esp_err_t ret = ESP_OK;
    ret = i2c_master_write_slave_reg(AXP_I2C_PORT, AXP202_ADDR, reg, &value, 1);
    return ret;
}

void power_enable(enum power_source source){
    uint8_t old_value=0;
    uint8_t err = ESP_OK;
    err = read_register(REG_POWER_OUTPUT, &old_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to read register");
        return;
    }
    err = write_register(REG_POWER_OUTPUT, old_value | source);
}

void power_disable(enum power_source source){
    uint8_t old_value=0;
    uint8_t new_value=0;
    uint8_t err = ESP_OK;
    err = read_register(REG_POWER_OUTPUT, &old_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to read register");
        return;
    }
    new_value &= ~(source);
    err = write_register(REG_POWER_OUTPUT, new_value);

}
bool power_enabled(enum power_source source){
    return true;
}
bool power_disabled(enum power_source source){
    return true;
}

void enable_pek_irq(){

    uint8_t old_value=0;
    uint8_t err = ESP_OK;
    err = read_register(0x42, &old_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to read register");
        return;
    }
    err = write_register(0x42, old_value | 1<<1 | 1);

    old_value=0;
    err = ESP_OK;
    err = read_register(0x44, &old_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to read register");
        return;
    }
    err = write_register(0x44, old_value | 1<<5 | 1<<6);

    old_value=0;
    err = ESP_OK;
    err = read_register(0x31, &old_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to read register");
        return;
    }
    err = write_register(0x31, old_value | 1<<3 );

}
