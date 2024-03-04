#include <stdbool.h>
#include <stdint.h>

#include <esp_err.h>
#include <esp_log.h>

#include "drivers/i2c_device.h"
#include "utilities/low_pass_filter.h"
#include "drivers/dps3xx_barometer.h"

#define DPS3XX_BARO_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define DPS3XX_BARO_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define DPS3XX_BARO_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define DPS3XX_BARO_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define DPS3XX_BARO_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define DPS3XX_BARO_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define DPS3XX_BARO_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			DPS3XX_BARO_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define DPS3XX_BARO_ASSERT(condition, format, ...)
#endif

static const char *TAG = "DPS3XX_BARO";

Dps3xxBarometer::Dps3xxBarometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle) : 
I2cDevice(p_i2c_master, device_addr, p_task_param, queue_handle) { 
    m_p_fir_filter = new FirFilter<uint32_t, uint32_t>(FILTER_DEPTH_POWER_16, AIR_PRESSURE_DEFAULT_VALUE);
}

Dps3xxBarometer::~Dps3xxBarometer() {
    delete m_p_fir_filter;
}

esp_err_t Dps3xxBarometer::init_device() {
    return ESP_OK;
}

esp_err_t Dps3xxBarometer::deinit_device() {
    return ESP_OK;
}

esp_err_t Dps3xxBarometer::fetch_data(uint8_t *data, uint8_t size) {
    DPS3XX_BARO_ASSERT(size >= sizeof(bm8563rtc_time_regs_t), "Buffer size is not enough to contain datetime structure.");
    return this->read_buffer(BM8563_DATETIME_REGS_ADDRESS, data, sizeof(bm8563rtc_time_regs_t));
}

esp_err_t Dps3xxBarometer::calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    DPS3XX_BARO_ASSERT(in_size >= sizeof(bm8563rtc_time_regs_t), "Buffer size is not enough to contain datetime structure.");
    bm8563rtc_time_regs_t *regs = (bm8563rtc_time_regs_t *)in_data;

    p_message->type = BLUETHROAT_MSG_BAROMETER;

    return ESP_OK;
}
