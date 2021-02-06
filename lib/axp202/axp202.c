#include "axp202.h"
#include "esp_log.h"
#include <stdlib.h>
#include "esp_err.h"
#include "helpers.h"

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

void read_irq(AXP_IRQ* irq ){
    //char buffer[8];
    ESP_ERROR_CHECK(i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, REG_IRQ_1, (void*)irq, sizeof(AXP_IRQ)));
}

/*esp_err_t read_irq(uint8_t* result){*/
/*    uint8_t tmp = 0;*/
/*    //char buffer[8];*/
/*    esp_err_t err = ESP_OK;*/
/*    for(uint8_t i=0; i<5; i++) {*/
/*        err = register_read(REG_IRQ_1 + i, &tmp);*/
/*       if (err != ESP_OK) {*/
/*           ESP_LOGE(TAG, "can't read irq");*/
/*       }*/
/*       result[i] = tmp;*/
/*    }*/
/*    return err;*/
/*}*/
void clear_irq(){
    AXP_IRQ irq = {0};
    ESP_ERROR_CHECK(i2c_master_write_slave_reg(AXP_I2C_PORT, AXP202_ADDR, REG_IRQ_1, (void*)&irq, sizeof(AXP_IRQ)));
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
    return ((high << 5) | (low & 0x1f)) * 0.5;

}
void axp_power_on(){
    PowerOutputControl poc;

    ESP_ERROR_CHECK(i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x12, (void*)&poc, sizeof(poc)));
    poc.dc3 = 1;
    poc.ldo2 = 1;
    ESP_ERROR_CHECK(i2c_master_write_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x12, (void*)&poc, sizeof(poc)));
}
void axp_sleep(){

    PowerOutputControl poc;
    poc.exten = 0;
    poc.dc3 = 1;
    poc.ldo2 = 0;
    poc.ldo4 = 0;
    poc.dc2 = 0;
    poc.ldo3 = 0;

    ESP_ERROR_CHECK(i2c_master_write_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x12, (void*)&poc, sizeof(poc)));
}
bool axp_charging(){
    PowerWorkingMode pwm;
    ESP_ERROR_CHECK(i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x01, (void*)&pwm, sizeof(pwm)));
    return (bool) pwm.charging;
}
uint8_t get_fuel_guage(){
    uint8_t value=0;
    register_read(0xB9, &value);
    return value & 0x7f;

}
