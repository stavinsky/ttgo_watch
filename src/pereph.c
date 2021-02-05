#include "pereph.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "axp202.h"

#include "driver/gpio.h"
#include "helpers.h"
#include "tt_config.h"
#include "driver/ledc.h"


#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

/* Init i2c port for AXP202 power module */
static esp_err_t i2c_master_init_port_0(){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = SCL_PIN,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t  err=ESP_OK;
    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    return err;
}

/* Init i2c port for touch module */
static esp_err_t i2c_master_init_port_1(void) {
  int i2c_master_port = I2C_NUM_1;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = CONFIG_LV_TOUCH_I2C_SDA;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = CONFIG_LV_TOUCH_I2C_SCL;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  return i2c_driver_install(i2c_master_port, conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
}

void i2c_init();
void power_init();

void pereph_init(){
    gpio_pad_select_gpio(35); // axp202
    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_intr_type(35, GPIO_INTR_NEGEDGE);

    gpio_pad_select_gpio(38); // ft6236
    gpio_set_direction(38, GPIO_MODE_INPUT);
    gpio_set_intr_type(38, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);
    ESP_ERROR_CHECK(i2c_master_init_port_0());
    ESP_ERROR_CHECK(i2c_master_init_port_1());
    axp_power_on();
    enable_pek_irq();
    clear_irq();
     /*Display backlight*/
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT, // resolution of PWM duty
        .freq_hz = 200,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 200,
        .gpio_num   = 12,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_1,
    };
    ledc_channel_config(&ledc_channel);
}

