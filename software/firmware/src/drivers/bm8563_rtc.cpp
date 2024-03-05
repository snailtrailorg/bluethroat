#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <esp_err.h>
#include <esp_log.h>

#include "drivers/i2c_device.h"
#include "drivers/bm8563_rtc.h"

#define BM8563_RTC_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define BM8563_RTC_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define BM8563_RTC_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define BM8563_RTC_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define BM8563_RTC_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define BM8563_RTC_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define BM8563_RTC_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define BM8563_RTC_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define BM8563_RTC_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define BM8563_RTC_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define BM8563_RTC_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			BM8563_RTC_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define BM8563_RTC_ASSERT(condition, format, ...)
#endif

static const char *TAG = "BM8563_RTC";

#define BM8563_DATETIME_REGS_ADDRESS   (0x51)

typedef union {
    uint8_t bytes[0];
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
} __attribute__ ((packed)) bm8563rtc_time_regs_t;

Bm8563Rtc::Bm8563Rtc(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle) : 
I2cDevice(p_i2c_master, device_addr, p_task_param, queue_handle) {

}

Bm8563Rtc::~Bm8563Rtc() {

}

esp_err_t Bm8563Rtc::CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr) {
	(void)p_i2c_master;
	(void)device_addr;
	return ESP_OK;
}

esp_err_t Bm8563Rtc::GetRtcTime(struct tm *stm_time) {
    BM8563_RTC_ASSERT(stm_time != NULL, "Get RTC time with NULL parameter.");
    bm8563rtc_time_regs_t regs;
    if (ESP_OK == this->fetch_data(regs.bytes, sizeof(regs))) {
        stm_time->tm_sec = bcd_to_uint8(regs.second);
        stm_time->tm_sec = bcd_to_uint8(regs.minute);
        stm_time->tm_sec = bcd_to_uint8(regs.hour);
        stm_time->tm_sec = regs.weekday;
        stm_time->tm_sec = bcd_to_uint8(regs.day);
        stm_time->tm_sec = bcd_to_uint8(regs.month) - 1;
        stm_time->tm_sec = bcd_to_uint8(regs.year) + ((regs.c == 0) ? 100 : 200);
        stm_time->tm_yday = this->calc_year_day(stm_time->tm_year + 1900, stm_time->tm_mon, stm_time->tm_mday);

        return ESP_OK;
    } else {
        BM8563_RTC_LOGE("Get RTC time filed!");
        return ESP_FAIL;
    }
}

esp_err_t Bm8563Rtc::SetRtcTime(struct tm *stm_time) {
    BM8563_RTC_ASSERT(stm_time != NULL, "Set RTC time with NULL parameter.");

    bm8563rtc_time_regs_t regs;
    regs.second = stm_time->tm_sec;
    regs.vl = 0;
    regs.minute = stm_time->tm_min;
    regs.hour = stm_time->tm_hour;
    regs.day = stm_time->tm_mday;
    regs.weekday = stm_time->tm_wday;
    regs.month = stm_time->tm_mon;
    regs.c = ((stm_time->tm_year > 199) ? 1 : 0);
    regs.year = stm_time->tm_year - ((stm_time->tm_year > 199) ? 100 : 200);

    if (ESP_OK != this->write_buffer(BM8563_DATETIME_REGS_ADDRESS, regs.bytes, sizeof(regs))) {
        BM8563_RTC_LOGE("Set RTC time filed!");
        return ESP_FAIL;
    } else {
        BM8563_RTC_LOGD("Set RTC time ok %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", stm_time->tm_year + 1900, stm_time->tm_mon, stm_time->tm_mday, stm_time->tm_hour, stm_time->tm_min, stm_time->tm_sec);
        return ESP_OK;
    }
}

esp_err_t Bm8563Rtc::SetSysTime(struct tm *stm_time) {
    BM8563_RTC_ASSERT(stm_time != NULL, "Set system time with NULL parameter.");
    struct timeval stv_time = {.tv_sec = mktime(stm_time), .tv_usec = 0};
    
    if (0 != settimeofday(&stv_time, NULL)) {
        BM8563_RTC_LOGE("Set system time filed!");
        return ESP_FAIL;
    } else {
        BM8563_RTC_LOGD("Set system time ok %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", stm_time->tm_year + 1900, stm_time->tm_mon, stm_time->tm_mday, stm_time->tm_hour, stm_time->tm_min, stm_time->tm_sec);
        return ESP_OK;
    }
}

esp_err_t Bm8563Rtc::init_device() {
    return ESP_OK;
}

esp_err_t Bm8563Rtc::deinit_device() {
    return ESP_OK;
}

esp_err_t Bm8563Rtc::fetch_data(uint8_t *data, uint8_t size) {
    BM8563_RTC_ASSERT(size >= sizeof(bm8563rtc_time_regs_t), "Buffer size is not enough to contain datetime structure.");
    return this->read_buffer(BM8563_DATETIME_REGS_ADDRESS, data, sizeof(bm8563rtc_time_regs_t));
}

esp_err_t Bm8563Rtc::calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    BM8563_RTC_ASSERT(in_size >= sizeof(bm8563rtc_time_regs_t), "Buffer size is not enough to contain datetime structure.");
    bm8563rtc_time_regs_t *regs = (bm8563rtc_time_regs_t *)in_data;

    p_message->type = BLUETHROAT_MSG_RTC;
    p_message->rtc_data.second = bcd_to_uint8(regs->second);
    p_message->rtc_data.minute = bcd_to_uint8(regs->minute);
    p_message->rtc_data.hour = bcd_to_uint8(regs->hour);
    p_message->rtc_data.weekday = bcd_to_uint8(regs->weekday);
    p_message->rtc_data.day = bcd_to_uint8(regs->day);
    p_message->rtc_data.month = bcd_to_uint8(regs->month) - 1;
    p_message->rtc_data.year = bcd_to_uint8(regs->year) + ((regs->c == 0) ? 100 : 200);

    return ESP_OK;
}

inline uint8_t Bm8563Rtc::bcd_to_uint8(uint8_t bcd) {
    union {
        uint8_t bcd;
        struct {
            uint8_t low : 4;
            uint8_t high : 4;
        } __attribute__ ((packed));
    } converter;

    converter.bcd = bcd;

    return
        converter.high * 10 
        + converter.low
    ;
}

inline uint16_t Bm8563Rtc::calc_year_day(uint16_t year, uint8_t month, uint8_t day) {
    static const int8_t month_day_diff[12] = {0, 1, -1, 0, 0, 1, 1, 2, 3, 3, 4, 4};
    return
        day 
        + 30 * month 
        + month_day_diff[month] 
        + (month > 1) * (year % 4 == 0) 
        - (month > 1) * (year % 100 == 0 && year % 400 != 0)
    ;
}

