#include "lv_conf.h"
#include "lv_core/lv_style.h"
#include "tt_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"


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
#include "pereph.h"
#include "isr.h"
//#include "ft6230u.h"
#define LV_TICK_PERIOD_MS 1
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_demo_application(void);
SemaphoreHandle_t xGuiSemaphore;
lv_obj_t * ta1;


void app_main()
{
    pereph_init();

    register_button_isr();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    printf("Hello world!\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    uint8_t reg = 0;
    char buffer[50];
    i2c_master_read_slave_reg(I2C_NUM_1, 0x38, 0x02, &reg, 1);
    sprintf(buffer, "touch  register: %2X", reg);
    lv_textarea_set_text(ta1, buffer);
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


static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

