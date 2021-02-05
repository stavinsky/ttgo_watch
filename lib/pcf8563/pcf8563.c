#include "pcf8563.h"
#include "time.h"
#include "esp_err.h"
#include "helpers.h"

static const char* TAG = "ttgo_rtc";


struct tm read_time(){
    struct tm time;
    uint8_t result[7];
    ESP_ERROR_CHECK(i2c_master_read_slave_reg(RTC_I2C_PORT, RTC_ADDRESS, 0x2, result, 7));
    time.tm_sec  = BCD_INT(result[0] & 0x7F);
    time.tm_min  = BCD_INT(result[1] & 0x7F);
    time.tm_hour = BCD_INT(result[2] & 0x3F);
    time.tm_mday = BCD_INT(result[3] & 0x3F);
    time.tm_wday = result[4] & 0x07;
    time.tm_mon  = BCD_4_LOW(result[5]) - 1;
    if ((result[5] & (1<<4)) >0 ){
        printf("month\n");
        time.tm_mon = time.tm_mon + 10;
    }
    time.tm_year =BCD_INT(result[6]);
    printf("%2x\n", result[0]);
    printf("%2x\n", result[1]);
    printf("%2x\n", result[2]);
    printf("%2x\n", result[3]);
    printf("%2x\n", result[4]);
    printf("%2x\n", result[5]);
    printf("%2x\n", result[6]);
    if ((result[5] & (1<<7)) >0 ){
        printf("over 100\n");
        time.tm_year = time.tm_year + 100;
    }

    return time;
}

