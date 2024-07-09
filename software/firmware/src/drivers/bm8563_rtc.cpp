#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <esp_err.h>
#include <esp_log.h>

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

#define BM8563_DATETIME_REGS_ADDRESS   (0x02)

typedef union {
    uint8_t bytes[0];
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t second:7;
        uint8_t vl:1;
        uint8_t minute:7;
        uint8_t:1;
        uint8_t hour:6;
        uint8_t:2;
        uint8_t day:6;
        uint8_t:2;
        uint8_t weekday:3;
        uint8_t:5;
        uint8_t month:5;
        uint8_t:2;
        uint8_t c:1;
        uint8_t year:8;
#else
        uint8_t vl:1;
        uint8_t second:7;
        uint8_t:1;
        uint8_t minute:7;
        uint8_t:2;
        uint8_t hour:6;
        uint8_t:1;
        uint8_t day:7;
        uint8_t:5;
        uint8_t weekday:3;
        uint8_t c:1;
        uint8_t:2;
        uint8_t month:5;
        uint8_t year:8;
#endif
    };
} __attribute__ ((packed)) Bm8563rtcTimeRegs_t;

Bm8563Rtc::Bm8563Rtc() : I2cDevice() {
    m_p_object_name = TAG;
    BM8563_RTC_LOGI("Create %s device.", m_p_object_name);
}

Bm8563Rtc::~Bm8563Rtc() {

}

esp_err_t Bm8563Rtc::CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr) {
	(void)p_i2c_master;
	(void)device_addr;
	return ESP_OK;
}

esp_err_t Bm8563Rtc::get_time(struct tm *stm_time) {
    BM8563_RTC_ASSERT(stm_time != NULL, "Get RTC time with NULL parameter.");
    Bm8563rtcTimeRegs_t regs;
    
    if (ESP_OK == this->read_buffer(BM8563_DATETIME_REGS_ADDRESS, regs.bytes, sizeof(Bm8563rtcTimeRegs_t))) {
        stm_time->tm_sec = bcd_to_uint8(regs.second);
        stm_time->tm_min = bcd_to_uint8(regs.minute);
        stm_time->tm_hour = bcd_to_uint8(regs.hour);
        stm_time->tm_wday = regs.weekday;
        stm_time->tm_mday = bcd_to_uint8(regs.day);
        stm_time->tm_mon = bcd_to_uint8(regs.month) - 1;
        stm_time->tm_year = bcd_to_uint8(regs.year) + ((regs.c == 0) ? 100 : 200);
        stm_time->tm_yday = this->calc_year_day(stm_time->tm_year + 1900, stm_time->tm_mon, stm_time->tm_mday);

        BM8563_RTC_LOGD("Get RTC time ok %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", stm_time->tm_year + 1900, stm_time->tm_mon+1, stm_time->tm_mday, stm_time->tm_hour, stm_time->tm_min, stm_time->tm_sec);
        return ESP_OK;
    } else {
        BM8563_RTC_LOGE("Get RTC time filed!");
        return ESP_FAIL;
    }
}

esp_err_t Bm8563Rtc::set_time(struct tm *stm_time) {
    BM8563_RTC_ASSERT(stm_time != NULL, "Set RTC time with NULL parameter.");

    Bm8563rtcTimeRegs_t regs;
    regs.second = uint8_to_bcd(stm_time->tm_sec);
    regs.vl = 0;
    regs.minute = uint8_to_bcd(stm_time->tm_min);
    regs.hour = uint8_to_bcd(stm_time->tm_hour);
    regs.day = uint8_to_bcd(stm_time->tm_mday);
    regs.weekday = stm_time->tm_wday;
    regs.month = uint8_to_bcd(stm_time->tm_mon + 1);
    regs.c = ((stm_time->tm_year >= 200) ? 1 : 0);
    regs.year = uint8_to_bcd(stm_time->tm_year % 100);

    if (ESP_OK == this->write_buffer(BM8563_DATETIME_REGS_ADDRESS, regs.bytes, sizeof(Bm8563rtcTimeRegs_t))) {
        BM8563_RTC_LOGD("Set RTC time ok %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", stm_time->tm_year + 1900, stm_time->tm_mon+1, stm_time->tm_mday, stm_time->tm_hour, stm_time->tm_min, stm_time->tm_sec);
        return ESP_OK;
    } else {
        BM8563_RTC_LOGE("Set RTC time filed!");
        return ESP_FAIL;
    }
}

esp_err_t Bm8563Rtc::init_device() {
    g_pBm8563Rtc = this;
    return ESP_OK;
}

esp_err_t Bm8563Rtc::deinit_device() {
    g_pBm8563Rtc = NULL;
    return ESP_OK;
}

esp_err_t Bm8563Rtc::fetch_data(uint8_t *data, uint8_t size) {
    return ESP_OK;
}

esp_err_t Bm8563Rtc::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    return ESP_OK;
}

inline uint8_t Bm8563Rtc::bcd_to_uint8(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

inline uint8_t Bm8563Rtc::uint8_to_bcd(uint8_t uint8) {
    return (uint8 / 10) << 4 | (uint8 % 10);
}

inline uint16_t Bm8563Rtc::calc_year_day(uint16_t year, uint8_t month, uint8_t day) {
    static const int8_t month_day_diff[12] = {0, 1, -1, 0, 0, 1, 1, 2, 3, 3, 4, 4};
    return \
        day \
        + 30 * month \
        + month_day_diff[month] \
        + (month > 1) * (year % 4 == 0) \
        - (month > 1) * (year % 100 == 0 && year % 400 != 0) \
    ;
}

Bm8563Rtc *g_pBm8563Rtc = NULL;

esp_err_t GetRtcTime(struct tm *stm_time) {
    if (g_pBm8563Rtc != NULL) {
        return g_pBm8563Rtc->get_time(stm_time);
    } else {
        BM8563_RTC_LOGE("Get RTC time failed, RTC device is not initialized.");
        return ESP_FAIL;
    }
}

esp_err_t SetRtcTime(struct tm *stm_time) {
    if (g_pBm8563Rtc != NULL) {
        return g_pBm8563Rtc->set_time(stm_time);
    } else {
        BM8563_RTC_LOGE("Set RTC time failed, RTC device is not initialized.");
        return ESP_FAIL;
    }
}
