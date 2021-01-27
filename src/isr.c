#include "freertos/FreeRTOS.h"
#include "isr.h"
#include "esp_attr.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "lvgl_tft/st7789.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_wifi.h"



#include "axp202.h"


static void IRAM_ATTR button_isr_handler(void *arg){
    TaskHandle_t * handle;
    handle = (TaskHandle_t *) arg;
    xTaskResumeFromISR(*handle);
}

static void button_task(void *args){
    uint8_t irq[5] = {0xff, 0xff, 0xff,0xff,0xff };
    while(1) {
        vTaskSuspend(NULL);
        read_irq(irq);
        clear_irq();
        enable_pek_irq();
        printf("irq triggered\n");
        if (axp_is_pek_long_press(irq)){
            printf("irq pek long press\n");
            continue;
        }
        if (axp_is_pek_short_press(irq)){
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

            clear_irq();
            enable_pek_irq();
            continue;
        }

    }

}

void register_button_isr(){
    TaskHandle_t *param = (TaskHandle_t *) pvPortMalloc(
            sizeof(TaskHandle_t));
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, param);

    gpio_isr_handler_add(35, button_isr_handler, (void *)param);
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
    /*gpio_pad_select_gpio(38); // ft6236*/
    /*gpio_set_direction(38, GPIO_MODE_INPUT);*/
    /*gpio_set_intr_type(38, GPIO_INTR_NEGEDGE);*/


    //gpio_isr_handler_add(38, touch_isr_handler, NULL);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    //xTaskCreate(touch_task, "touch_task", 4096, NULL, 10, &TouchISR);


