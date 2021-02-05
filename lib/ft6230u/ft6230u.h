#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

QueueHandle_t touch_queue;

typedef struct {
    uint16_t x;
    uint16_t y;
}TouchPosition;


void get_touch(uint8_t* data);
uint8_t num_points(uint8_t* data);
bool read_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);


bool get_position(TouchPosition* data);

void ft_touch_init();
