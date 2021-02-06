#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "esp_err.h"



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



bool axp_is_pek_short_press(uint8_t* irq);
bool axp_is_pek_long_press(uint8_t* irq);
float axp_battery_discharge_current();

bool axp_charging();
uint8_t get_fuel_guage();
#pragma pack(push, 1)
typedef struct {
    uint8_t exten  :1;
    uint8_t dc3    :1;
    uint8_t ldo2   :1;
    uint8_t ldo4   :1;
    uint8_t dc2    :1;
    uint8_t        :1;
    uint8_t ldo3   :1;
    uint8_t        :1;

}PowerOutputControl;
typedef struct {
    uint8_t                    :2;
    uint8_t low_charge_current :1;
    uint8_t activate_model     :1;
    uint8_t                    :1;
    uint8_t battery_exists     :1;
    uint8_t charging           :1;
    uint8_t over_temperature   :1;
}PowerWorkingMode;

typedef struct {
    //reg 0x48
    uint8_t                    :1;
    uint8_t acin_over_volt     :1;
    uint8_t acin_plugin        :1;
    uint8_t acin_removal       :1;
    uint8_t vbus_plugin        :1;
    uint8_t vbus_removal       :1;
    uint8_t vbus_low_volt      :1;
    uint8_t over_temperature   :1;

    //reg 0x49
    uint8_t bat_temp_low       :1;
    uint8_t bat_temp_high      :1;
    uint8_t charge_done        :1;
    uint8_t charging           :1;
    uint8_t exit_bat_act_mode  :1;
    uint8_t bat_act_mode       :1;
    uint8_t bat_removal        :1;
    uint8_t bat_plugin         :1;

    //reg 0x4A
    uint8_t pek_long_press     :1;
    uint8_t pek_short_press    :1;
    uint8_t                    :1;
    uint8_t dc3_volt_long      :1;
    uint8_t dc2_volt_long      :1;
    uint8_t dc1_volt_long      :1;
    uint8_t charge_insuff      :1;
    uint8_t die_temp_high      :1;

    //reg 0x4B
    uint8_t low_pow_lev_2      :1;
    uint8_t low_pow_lev_1      :1;
    uint8_t vbus_sess_end      :1;
    uint8_t vbus_sess_valid    :1;
    uint8_t vbus_invalid       :1;
    uint8_t vbus_valid         :1;
    uint8_t n_oe_pow_off       :1;
    uint8_t n_oe_pow_on        :1;

    //reg 0x4C
    uint8_t gpio_0_trig        :1;
    uint8_t gpio_1_trig        :1;
    uint8_t gpio_2_trig        :1;
    uint8_t gpio_3_trig        :1;
    uint8_t                    :1;
    uint8_t pek_falling_edge   :1;
    uint8_t pek_rising_edge    :1;
    uint8_t timer_int          :1;




}AXP_IRQ;

#pragma pack(pop)

void read_irq(AXP_IRQ* irq);
void clear_irq();
