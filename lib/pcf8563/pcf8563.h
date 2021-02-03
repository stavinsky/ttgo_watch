#pragma once


#define RTC_ADDRESS 81
#define RTC_I2C_PORT 0

#include "time.h"

struct tm read_time();

