#include "main.h"
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
#include "helpers.h"

#include "lvgl.h"
#include "lvgl_tft/st7789.h"
#include "driver/gpio.h"
#include <driver/i2c.h>
#include "axp202.h"
#include "esp_intr_alloc.h"
#include "pereph.h"
#include "isr.h"
#include "pcf8563.h"
#include "time.h"
#include "ft6230u.h"

static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_demo_application(void);
SemaphoreHandle_t xGuiSemaphore;

xTaskHandle current_window_handle = NULL;



void main_window_task(void* arg){
    BaseType_t notification;
    uint8_t gauge = 0;
    lv_obj_t *tileview = (lv_obj_t *) arg;

    lv_obj_t * tile1 = lv_obj_create(tileview, NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);
    lv_tileview_add_element(tileview, tile1);

    lv_obj_t * blink_label  = (lv_obj_t*)malloc(sizeof(lv_obj_t));
    blink_label = lv_label_create(tile1, NULL);
    lv_label_set_text(blink_label, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_align(blink_label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t *percent_label = lv_label_create(tile1, NULL);
    lv_obj_align(percent_label, blink_label , LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *discharge_current_label = lv_label_create(tile1, NULL);
    lv_obj_align(discharge_current_label, percent_label, LV_ALIGN_OUT_RIGHT_MID, 5,0);
    while(1){
        if (axp_charging()){
            lv_obj_set_hidden(blink_label, lv_obj_is_visible(blink_label));
        }
        else {
            lv_obj_set_hidden(blink_label, false);
        }
        gauge = get_fuel_guage();
        switch ((int)(gauge/20)) {
            case 0:
                lv_label_set_text(blink_label, LV_SYMBOL_BATTERY_EMPTY);
                break;
            case 1:
                lv_label_set_text(blink_label, LV_SYMBOL_BATTERY_1);
                break;
            case 2:
                lv_label_set_text(blink_label, LV_SYMBOL_BATTERY_2);
                break;
            case 3:
                lv_label_set_text(blink_label, LV_SYMBOL_BATTERY_3);
                break;
            case 4:
                lv_label_set_text(blink_label, LV_SYMBOL_BATTERY_FULL);
                break;

        }

        lv_label_set_text_fmt(percent_label, "%i%" , gauge);
        lv_label_set_text_fmt(discharge_current_label, "%2.3f", axp_battery_discharge_current());

        notification = xTaskNotifyWait(0xffffffff, 0xffffffff, NULL, pdMS_TO_TICKS(1000));
        if (notification == pdTRUE ) {
            break;
        }
    }
    printf("kill\n");
    printf("free memory: %i\n", xPortGetFreeHeapSize());
    //lv_obj_del(tile1);
    //vTaskDelay(pdMS_TO_TICKS(200));
    vTaskDelete(NULL);

}


void tile_view_cb(lv_obj_t * obj, lv_event_t event){

	if (event == LV_EVENT_VALUE_CHANGED){
        if(current_window_handle != NULL) {
            xTaskNotify(current_window_handle, 0, eNoAction);
        }
		const uint32_t *event_value = lv_event_get_data();
		if(*event_value == 0) {
            xTaskCreatePinnedToCore(main_window_task, "mw_task", 2048,(void *) obj, 1, &current_window_handle,1);
		}

	}

}

void lv_ex_tileview_1(void)
{
    static lv_point_t valid_pos[] = {{0,0}, {0, 1}};
    lv_obj_t *tileview;
    tileview = lv_tileview_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(tileview, tile_view_cb);
    lv_tileview_set_valid_positions(tileview, valid_pos, 2);
    lv_tileview_set_edge_flash(tileview, false);

    lv_obj_t * list = lv_list_create(tileview, NULL);
    lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(list, 0, LV_VER_RES);
    lv_list_set_scroll_propagation(list, true);
    lv_list_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);

    lv_list_add_btn(list, NULL, "One");
    lv_list_add_btn(list, NULL, "Two");
    lv_list_add_btn(list, NULL, "Three");
    lv_list_add_btn(list, NULL, "Four");
    lv_list_add_btn(list, NULL, "Five");
    lv_list_add_btn(list, NULL, "Six");
    lv_list_add_btn(list, NULL, "Seven");
    lv_list_add_btn(list, NULL, "Eight");
}
void app_main() {
    pereph_init();

    register_button_isr();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    //lv_textarea_set_text(ta1, buffer);
}

static void btn_event_cb(lv_obj_t * btn, lv_event_t event) {
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

    /*touch driver*/
    ft_touch_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = read_touch_cb;
    lv_indev_drv_register(&indev_drv);
    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui",
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /*ta1 = lv_textarea_create(lv_scr_act(), NULL);*/
    /*static lv_style_t st;*/
    /*lv_style_init(&st);*/
    /*lv_style_set_text_font(&st, LV_STATE_DEFAULT, &lv_font_montserrat_14);*/
    /*lv_obj_add_style(ta1, LV_TEXTAREA_PART_BG, &st);*/
    /*lv_obj_set_size(ta1, 240, 240);*/
    /*lv_obj_align(ta1, NULL, LV_ALIGN_CENTER, 0, 0);*/

    /*lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     [>Add a button the current screen<]*/
    /*lv_obj_set_pos(btn, 10, 10);                            [>Set its position<]*/
    /*lv_obj_set_size(btn, 120, 50);                          [>Set its size<]*/
    /*lv_obj_set_event_cb(btn, btn_event_cb);                 [>Assign a callback to the button<]*/

    /*lv_obj_t * label = lv_label_create(btn, NULL);          [>Add a label to the button<]*/
    /*lv_label_set_text(label, "Button");*/

    //lv_textarea_set_text(ta1, "A text in a Text Area");

    lv_ex_tileview_1();
    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(20));

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

