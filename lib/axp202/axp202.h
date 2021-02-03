#pragma once

#include <stdio.h>
#include "i2c.h"
#define AXP202_ADDR 0x35
#define AXP_I2C_PORT I2C_NUM_0


#define REG_POWER_OUTPUT 0x12
#define REG_IRQ_1 0x48

enum power_source {
    AXP_EXTEN = 1<<0,
    AXP_DC3   = 1<<1,
    AXP_LDO2  = 1<<2,
    AXP_LDO4  = 1<<3,
    AXP_DC2   = 1<<4,
    AXP_LDO3  = 1<<6,
};
void power_enable(enum power_source source);
void power_disable(enum power_source source);
bool power_enabled(enum power_source source);
bool power_disabled(enum power_source source);
void enable_pek_irq();
void axp_power_on();
void axp_sleep();

esp_err_t read_irq(uint8_t* result);
esp_err_t clear_irq();


bool axp_is_pek_short_press(uint8_t* irq);
bool axp_is_pek_long_press(uint8_t* irq);
float axp_battery_discharge_current();

bool axp_charging();
uint8_t get_fuel_guage();

