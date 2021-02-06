#include "freertos/FreeRTOS.h"
#include "isr.h"
#include "esp_attr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "lvgl_tft/st7789.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_wifi.h"

#include "lvgl.h"
#include "main.h"

#include "axp202.h"


static void IRAM_ATTR button_isr_handler(void *arg){
    TaskHandle_t * handle;
    handle = (TaskHandle_t *) arg;
    xTaskResumeFromISR(*handle);
}

static void button_task(void *args){
    //uint8_t irq[5] = {0xff, 0xff, 0xff,0xff,0xff };
    AXP_IRQ irq;
    while(1) {
        vTaskSuspend(NULL);
        read_irq(&irq);
        clear_irq();
        enable_pek_irq();
        printf("irq triggered\n");
        if (irq.pek_long_press == 1){
            printf("irq pek long press\n");
            continue;
        }
        if (irq.pek_short_press == 1){
            printf("irq pek short press\n");
            vTaskDelay(pdMS_TO_TICKS(10));
            esp_sleep_enable_ext0_wakeup(35,0);
            st7789_send_cmd(0x10);
            axp_sleep();
            esp_bluedroid_disable();
            esp_bt_controller_disable();
            esp_wifi_stop();

            esp_light_sleep_start();
            //esp_deep_sleep_start();

            st7789_send_cmd(0x11);
            axp_power_on();
            esp_timer_stop(periodic_timer);
            clear_irq();
            enable_pek_irq();
            lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
            lv_task_handler();
            ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
            continue;
        }

    }

}
void IRAM_ATTR touch_isr_handler(void *arg){
    TaskHandle_t * handle;
    handle = (TaskHandle_t *) arg;
    xTaskResumeFromISR(*handle);
}


void touch_task(void *args) {
    touch_queue = xQueueCreate(5, sizeof(TouchPosition));
    TouchPosition *pos  = (TouchPosition*)pvPortMalloc(sizeof(TouchPosition));
    bool got_touch = false;
    while (1){
        vTaskSuspend(NULL);
        got_touch = get_position(pos);
        if (got_touch) {
            xQueueSend(touch_queue, (void*)&pos, ( TickType_t ) 5);
        }
    }
}

void register_button_isr(){
    TaskHandle_t *button_handle = (TaskHandle_t *) pvPortMalloc(
            sizeof(TaskHandle_t));

    xTaskCreate(button_task, "button_task", 4096, NULL, 10, button_handle);

    TaskHandle_t *touch_handle = (TaskHandle_t *) pvPortMalloc(
            sizeof(TaskHandle_t));
    xTaskCreate(touch_task, "touch_task", 4096, NULL, 10, touch_handle);

    gpio_isr_handler_add(35, button_isr_handler, (void *)button_handle);

    gpio_isr_handler_add(38, touch_isr_handler, (void*)touch_handle);

}


