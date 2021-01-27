#include "pereph.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "axp202.h"
#include "lvgl_touch/ft6x36.h"
#include "lvgl_touch/tp_i2c.h"

#include "driver/gpio.h"

void i2c_init();
void power_init();

void pereph_init(){
    gpio_pad_select_gpio(35); // axp202
    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_intr_type(35, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);

    i2c_master_init();
    lvgl_i2c_master_init();
    //i2c_2_master_init();
    axp_power_on();
    enable_pek_irq();
    clear_irq();

}

