#include "lv_conf.h"
#include "lv_core/lv_style.h"
#include "tt_config.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "esp_sleep.h"

#include "lvgl.h"
#include "lvgl_tft/st7789.h"
#include "lvgl_touch/tp_i2c.h"
#include "lvgl_touch/ft6x36.h"
#include "driver/gpio.h"
#include <driver/i2c.h>
#include "i2c.h"
#include "axp202.h"
#include "esp_intr_alloc.h"
#include "scanner.h"
//#include "ft6230u.h"
#define LV_TICK_PERIOD_MS 1
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_demo_application(void);
SemaphoreHandle_t xGuiSemaphore;
TaskHandle_t ISR = NULL;
//TaskHandle_t TouchISR = NULL;
void pereph_init(){
    i2c_master_init();
    lvgl_i2c_master_init();
    //i2c_2_master_init();
}
lv_obj_t * ta1;

void IRAM_ATTR button_isr_handler(void *arg){
    xTaskResumeFromISR(ISR);
}

/*void IRAM_ATTR touch_isr_handler(void *arg){*/
/*    xTaskResumeFromISR(TouchISR);*/
/*}*/
/*void touch_task(void *args) {*/
/*    uint8_t touch_data[14];*/
/*    enum gestures gest = NOTHING ;*/
/*    char buffer[50];*/
/*    uint8_t n_points = 0;*/
/*    while (1){*/
/*        vTaskSuspend(NULL);*/
/*        get_touch(touch_data);*/
/*        gest = get_gesture(touch_data);*/
/*        //if (gest != NOTHING) {*/
/*            sprintf(buffer, "gesture: %2X ", gest);*/
/*            lv_textarea_add_text(ta1, buffer);*/
/*        //}*/
/*        n_points = num_points(touch_data);*/
/*        if (n_points >0){*/
/*            sprintf(buffer, "touched:  %i\n", n_points);*/
/*            //lv_textarea_add_text(ta1, buffer);*/
/*        }*/
/*    }*/
/*}*/
void button_task(void *args){
    clear_irq();
    uint8_t irq[5] = {0xff, 0xff, 0xff,0xff,0xff };
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    float bat_dis_cur;
    char buffer[32];
    enable_pek_irq();
    clear_irq();
    read_irq(irq);
    while(1) {
        vTaskSuspend(NULL);
        read_irq(irq);
        clear_irq();
        enable_pek_irq();
        printf("irq triggered\n");
        lv_textarea_add_text(ta1, "power irq trigger\n");
        if (axp_is_pek_long_press(irq)){
            printf("irq pek long press\n");
            lv_textarea_add_text(ta1, "irq pek long press\n");
           continue;

        }
        if (axp_is_pek_short_press(irq)){
            printf("irq pek short press\n");
            lv_textarea_add_text(ta1, "irq pek short press\n");
            bat_dis_cur = axp_battery_discharge_current();
            uint8_t high, low ;
            i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x7c, &high, 1);
            i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x7c, &low, 1);
            sprintf(buffer, "discharge reg: %2X, %2X\n", high, low);
            lv_textarea_add_text(ta1, buffer);
            sprintf(buffer, "discharge current: %4.2f\n", bat_dis_cur);
            lv_textarea_add_text(ta1, buffer);
            i2c_master_read_slave_reg(AXP_I2C_PORT, AXP202_ADDR, 0x82, &low, 1);
            sprintf(buffer, "ADC reg: %2X\n", low);
            lv_textarea_add_text(ta1, buffer);
            esp_sleep_enable_ext0_wakeup(35,0);
            st7789_send_cmd(0x10);
            power_disable(AXP_LDO2);
            power_disable(AXP_LDO3);
            power_disable(AXP_LDO4);
            power_disable(AXP_EXTEN);
            esp_light_sleep_start();
            //esp_deep_sleep_start();
            st7789_send_cmd(0x11);
            power_enable(AXP_LDO2);
            power_enable(AXP_LDO3);
            power_enable(AXP_LDO4);
            power_enable(AXP_EXTEN);
            clear_irq();
            enable_pek_irq();
           continue;
        }

    }

}
void app_main()
{
    pereph_init();

    power_enable(AXP_EXTEN);
    power_enable(AXP_LDO4);
    power_enable(AXP_DC2);
    power_enable(AXP_LDO3);
    power_enable(AXP_LDO2);
    power_enable(AXP_DC3);

    enable_pek_irq();

    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_pad_select_gpio(35); // axp202
    gpio_set_direction(35, GPIO_MODE_INPUT);



    //gpio_set_intr_type(35, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(35, GPIO_INTR_NEGEDGE);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, &ISR);

    /*gpio_pad_select_gpio(38); // ft6236*/
    /*gpio_set_direction(38, GPIO_MODE_INPUT);*/
    /*gpio_set_intr_type(38, GPIO_INTR_NEGEDGE);*/

    gpio_install_isr_service(0);
    gpio_isr_handler_add(35, button_isr_handler, NULL);
    //gpio_isr_handler_add(38, touch_isr_handler, NULL);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    //xTaskCreate(touch_task, "touch_task", 4096, NULL, 10, &TouchISR);



    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_12, 1);
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");


    vTaskDelay(1000 / portTICK_PERIOD_MS);
    uint8_t reg = 0;
    char buffer[50];
    i2c_master_read_slave_reg(I2C_NUM_1, 0x38, 0x02, &reg, 1);
    sprintf(buffer, "touch  register: %2X", reg);
    lv_textarea_set_text(ta1, buffer);
    uint8_t val = 1;
    //i2c_master_write_slave_reg(I2C_NUM_1, 0x38, 0xA4, &val, 1);

    /*uint8_t touch_buffer[5];*/
    /*esp_err_t err=ESP_OK;*/
    /*err = i2c_master_read_slave_reg(I2C_NUM_1, 0x38, 0x02, touch_buffer, 5);*/
    /*if (err !=ESP_OK) {*/

    /*    lv_textarea_add_text(ta1, "can't read status");*/
    /*}*/
    /*while (0){*/
    /*    for (uint8_t i=0; i<5; i++){*/

    /*        sprintf(buffer, "touch  status value: %2X\n", touch_buffer[i]);*/
    /*        lv_textarea_set_text(ta1, buffer);*/
    /*    }*/
    /*    vTaskDelay(1000 / portTICK_PERIOD_MS);*/
    /*}*/



   /* Set the GPIO as a push/pull output */
    /*for (int i = 10; i >= 0; i--) {*/
    /*    printf("Restarting in %d seconds...\n", i);*/
    /*    vTaskDelay(1000 / portTICK_PERIOD_MS);*/
    /*}*/
    /*printf("Restarting now.\n");*/
    /*fflush(stdout);*/
    /*esp_restart();*/
}
static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, NULL);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}
static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();
    st7789_init();

    static lv_color_t buf1[DISP_BUF_SIZE];

    /* Use double buffered when not working with monochrome displays */
    static lv_color_t buf2[DISP_BUF_SIZE];

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;


    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig */


    /*touch driver*/
    ft6x06_init(0x38);
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = ft6x36_read;
    lv_indev_drv_register(&indev_drv);
    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
    ta1 = lv_textarea_create(lv_scr_act(), NULL);
    static lv_style_t st;
    lv_style_init(&st);
    lv_style_set_text_font(&st, LV_STATE_DEFAULT, &lv_font_montserrat_14);
    lv_obj_add_style(ta1, LV_TEXTAREA_PART_BG, &st);
    lv_obj_set_size(ta1, 240, 240);
    lv_obj_align(ta1, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_set_event_cb(btn, btn_event_cb);                 /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn, NULL);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");

    //lv_textarea_set_text(ta1, "A text in a Text Area");

    /* Create the demo application */
    //create_demo_application();

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    vTaskDelete(NULL);
}

static void create_demo_application(void)
{
    printf("demo application \n");
    /* When using a monochrome display we only show "Hello World" centered on the
     * screen */
    /* use a pretty small demo for monochrome displays */
    /* Get the current screen  */
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);

    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(scr, NULL);

    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello\nworld");

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

