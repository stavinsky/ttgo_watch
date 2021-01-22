#pragma once

#include <stdio.h>
#include "i2c.h"
#define AXP202_ADDR 0x35
#define AXP_I2C_PORT I2C_NUM_0


#define REG_POWER_OUTPUT 0x12

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
