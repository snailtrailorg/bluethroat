#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "esp_err.h"

#ifdef CONFIG_LV_M5STICKC_HANDLE_AXP192
    #include "lvgl_i2c/i2c_manager.h"
#endif

#include "drivers/esp_i2c.h"
#include "drivers/bm8563.h"

static uint8_t bcd_to_uint8(uint8_t bcd);
static inline uint16_t calc_year_day(uint16_t year, uint8_t month, uint8_t day);

typedef union {
    uint8_t regs[7];
    struct {
        uint8_t second:7;
        uint8_t vl:1;
        uint8_t minute:7;
        uint8_t:1;
        uint8_t hour:6;
        uint8_t:2;
        uint8_t day:7;
        uint8_t:1;
        uint8_t weekday:3;
        uint8_t:5;
        uint8_t month:5;
        uint8_t:2;
        uint8_t c:1;
        uint8_t year:8;
    };
} __attribute__ ((packed)) bcd_datetime_regs_t;

extern esp_err_t esp_i2c_read(i2c_port_t port, uint16_t addr, uint32_t reg, uint8_t *buffer, uint16_t size);

void bm_8563_init(void) {
    ;
}

bool bm_8563_read_time(struct tm * t) {
    bcd_datetime_regs_t bcd_datetime_regs;
    esp_err_t result = esp_i2c_read(1, 0x51, 2, &bcd_datetime_regs, 7);

    if (ESP_OK == result) {
  	    t->tm_sec = bcd_to_uint8(bcd_datetime_regs.second);
        t->tm_min = bcd_to_uint8(bcd_datetime_regs.minute);
        t->tm_hour = bcd_to_uint8(bcd_datetime_regs.hour);
        t->tm_mday = bcd_to_uint8(bcd_datetime_regs.day);
        t->tm_mon = bcd_to_uint8(bcd_datetime_regs.month) - 1;
        t->tm_year = bcd_to_uint8(bcd_datetime_regs.year) + ((bcd_datetime_regs.c == 0) ? 1900 : 2000);
        t->tm_wday = bcd_to_uint8(bcd_datetime_regs.weekday);
        t->tm_yday = calc_year_day(t->tm_year, t->tm_mon, t->tm_mday);
        t->tm_isdst = 0;
#ifdef __TM_GMTOFF
        t->__TM_GMTOFF = 0;
#endif
#ifdef __TM_ZONE
        t->__TM_ZONE = NULL;
#endif

        return true;
    } else {
        return false;
    }
}

static uint8_t bcd_to_uint8(uint8_t bcd) {
    union {
        uint8_t bcd;
        struct {
            uint8_t low : 4;
            uint8_t high : 4;
        };
    } converter;

    converter.bcd = bcd;
    return converter.high * 10 + converter.low;
}

/* month: 0~11
   day: 1~31
   return: xxxx-01-01 returns 1 */
static inline uint16_t calc_year_day(uint16_t year, uint8_t month, uint8_t day){
    static const uint8_t month_day_diff[12] = {0, 1, -1, 0, 0, 1, 1, 2, 3, 3, 4, 4};
    return day + 30 * month + month_day_diff[month] + (month > 1) * (year % 4 == 0) - (month > 1) * (year % 100 == 0 && year % 400 != 0);
}
