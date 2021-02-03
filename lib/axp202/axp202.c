#include "axp202.h"
#include "esp_log.h"
#include <stdlib.h>
#include "esp_err.h"

static const char* TAG = "axp202";

static esp_err_t register_read(uint8_t reg, uint8_t *result) {
  esp_err_t ret = ESP_OK;
  ret = i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, reg, result, 1);


  return ret;
}
static esp_err_t register_write(uint8_t reg, uint8_t value) {
  esp_err_t ret = ESP_OK;
  ret = i2c_master_write_slave_reg(AXP_I2C_PORT, AXP202_ADDR, reg, &value, 1);
  return ret;
}

static esp_err_t register_bit_set(uint8_t reg, uint8_t value) {
  uint8_t old_value = 0;
  uint8_t err = ESP_OK;
  err = register_read(reg, &old_value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "unable to read register");
    return err;
  }

  err = register_write(reg, old_value | value);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "unable to write register");
    return err;
  }
  return err;
}

static esp_err_t register_bit_clear(uint8_t reg, uint8_t value) {
  uint8_t old_value = 0;
  uint8_t new_value = 0;
  uint8_t err = ESP_OK;
  err = register_read(reg, &old_value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "unable to read register");
    return err;
  }

  new_value = old_value& (~(value));

  err = register_write(REG_POWER_OUTPUT, new_value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "unable to write register");
    return err;
  }
  return err;
}


void power_enable(enum power_source source){
    register_bit_set(REG_POWER_OUTPUT, source);
}

void power_disable(enum power_source source){
    register_bit_clear(REG_POWER_OUTPUT, source);
}
bool power_enabled(enum power_source source){
    return true;
}
bool power_disabled(enum power_source source){
    return true;
}

void enable_pek_irq(){
    register_bit_set(0x42,  1<<1 | 1);
    register_bit_set(0x44,  1<<5 | 1<<6);
    register_bit_set(0x31,  1<<3 );

    register_bit_set(0x82,  1<<6 ); // TODO: move to init somehow
}

esp_err_t read_irq(uint8_t* result){
    uint8_t tmp = 0;
    //char buffer[8];
    esp_err_t err = ESP_OK;
    for(uint8_t i=0; i<5; i++) {
        err = register_read(REG_IRQ_1 + i, &tmp);
       if (err != ESP_OK) {
           ESP_LOGE(TAG, "can't read irq");
       }
       // itoa(tmp, buffer, 2);
       // printf("register %02X: %s\n", REG_IRQ_1 + i, buffer);
       result[i] = tmp;
    }
    return err;
}
esp_err_t clear_irq(){
    esp_err_t err = ESP_OK;
    for(uint8_t i=0; i<5; i++) {
       err = register_write(REG_IRQ_1 + i, 0xFF);
       if (err != ESP_OK) {
           ESP_LOGE(TAG, "can't write irq");

       }
    }
    return err;

}
bool axp_is_pek_short_press(uint8_t* irq){
    return (bool)(irq[2] & (1 << 1));
}
bool axp_is_pek_long_press(uint8_t* irq){
    return (bool)(irq[2] & (1 << 0));
}
float axp_battery_discharge_current(){
    uint8_t low = 0;
    uint8_t high = 0;
    register_read(0x7c, &high);
    register_read(0x7d, &low);
    printf("current %2X, %2x", high, low);
    return ((high << 5) | (low & 0x1f)) * 0.5;

}

void axp_power_on(){
    power_enable(AXP_EXTEN);
    power_enable(AXP_LDO4);
    power_enable(AXP_DC2);
    power_enable(AXP_LDO3);
    power_enable(AXP_LDO2);
    power_enable(AXP_DC3); // esp32
}
void axp_sleep(){
    power_disable(AXP_EXTEN);
    power_disable(AXP_LDO4);
    power_disable(AXP_DC2);
    power_disable(AXP_LDO3);
    power_disable(AXP_LDO2);
    //power_disable(AXP_DC3);
}
bool axp_charging(){
    uint8_t value=0;
    register_read(0x01, &value);
    return (bool)(value & 1<<6);
}
uint8_t get_fuel_guage(){
    uint8_t value=0;
    register_read(0xB9, &value);
    return value & 0x7f;

}
