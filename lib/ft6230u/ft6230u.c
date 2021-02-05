#include "ft6230u.h"
#include "lvgl.h"
#include "esp_log.h"
#include "esp_err.h"
#include <stdlib.h>
#include "helpers.h"

#pragma pack(push,1)
typedef struct  {
    uint8_t points :4;
    uint8_t        :4;
    uint8_t xh     :4;
    uint8_t        :2;
    uint8_t event  :2;
    uint8_t xl     :8;
    uint8_t yh     :4;
    uint8_t        :4;
    uint8_t yl     :8;
} MyPoint;
#pragma pack(pop)


static const char* TAG = "axp202";


static esp_err_t register_read(uint8_t reg, uint8_t *result) {
  esp_err_t ret = ESP_OK;
  ret = i2c_master_read_slave_reg(I2C_NUM_1, 0x38, reg, result, 1);
  return ret;
}


void get_touch(uint8_t* data){
    esp_err_t ret = ESP_OK;
    ret = i2c_master_read_slave_reg(I2C_NUM_1, 0x38, 0x01, data, 14);
    if (ret != ESP_OK){
      ESP_LOGE(TAG, "Can't read from device");
    }
}
void ft_touch_init(){
    uint8_t val = 0x01;
    i2c_master_write_slave_reg(I2C_NUM_1, 0x38, 0x86, &val, 1);
}

bool get_position(TouchPosition* data){
    esp_err_t ret = ESP_OK;
    MyPoint my_point;
    ret = i2c_master_read_slave_reg(I2C_NUM_1, 0x38, 0x02, (void*)&my_point, sizeof(MyPoint));

    if (ret != ESP_OK || (my_point.points != 1)){
       return false;
    }
    data->x = (my_point.xh << 8) | my_point.xl;
    data->y = (my_point.yh << 8) | my_point.yl;
    return true;

}
bool read_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data){
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    TouchPosition *pos;
    if (touch_queue != NULL
            && xQueueReceive(touch_queue, &(pos), (TickType_t) 5) == pdTRUE){
        last_x = pos->x;
        last_y = pos->y;
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_PR;
        return true;
    }
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = LV_INDEV_STATE_REL;
    return false;

}

uint8_t num_points(uint8_t* data){
    return data[1] & 0x0F;
}
