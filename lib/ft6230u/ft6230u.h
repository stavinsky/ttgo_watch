#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

enum gestures {
    MOVE_UP = 0x10,
    MOVE_RIGHT = 0x14,
    MOVE_DOWN = 0x18,
    MOVE_LEFT = 0x1C,
    ZOOM_IN = 0x48,
    ZOOM_OUT = 0x49,
    NOTHING = 0x00,
};


void get_touch(uint8_t* data);
enum gestures get_gesture(uint8_t* data);
uint8_t num_points(uint8_t* data);



