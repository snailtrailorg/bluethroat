#include <esp_err.h>
#include <esp_log.h>

#include "drivers/axp192_pmu.h"

#define AXP192_PMU_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define AXP192_PMU_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define AXP192_PMU_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define AXP192_PMU_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define AXP192_PMU_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define AXP192_PMU_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define AXP192_PMU_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			AXP192_PMU_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define AXP192_PMU_ASSERT(condition, format, ...)
#endif

static const char *TAG = "AXP192_PMU";

Axp192Pmu::Axp192Pmu() : I2cDevice() {
	AXP192_PMU_LOGI("Create axp192 pmu device.");
}

Axp192Pmu::~Axp192Pmu() {

}

esp_err_t Axp192Pmu::init_device() {
    return ESP_OK;
}

esp_err_t Axp192Pmu::deinit_device() {
    return ESP_OK;
}

esp_err_t Axp192Pmu::fetch_data(uint8_t *data, uint8_t size) {
    return ESP_OK;
}

esp_err_t Axp192Pmu::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    return ESP_OK;
}
