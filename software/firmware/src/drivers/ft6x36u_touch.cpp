#include <esp_log.h>

#include <drivers/ft6x36u_touch.h>

#define FT6X36U_TOUCH_LOGE(format, ...)                 ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define FT6X36U_TOUCH_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define FT6X36U_TOUCH_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define FT6X36U_TOUCH_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define FT6X36U_TOUCH_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define FT6X36U_TOUCH_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define FT6X36U_TOUCH_ASSERT(condition, format, ...)    \
	do                                                  \
	{                                                   \
		if (!(condition))                               \
		{                                               \
			FT6X36U_TOUCH_LOGE(format, ##__VA_ARGS__);  \
			assert(0);                                  \
		}                                               \
	} while (0)
#else
#define FT6X36U_TOUCH_ASSERT(condition, format, ...)
#endif

static const char *TAG = "FT6X36U_TOUCH";

Ft6x36uTouch::Ft6x36uTouch(I2cMaster *p_i2c_master, uint16_t device_addr, const gpio_num_t *p_int_pins) : I2cDevice(p_i2c_master, device_addr, p_int_pins) {
    FT6X36U_TOUCH_LOGI("Create FT6X36U touch device at port %d, device_addr 0x%3x", this->m_p_i2c_master->m_port, this->m_device_addr);
}

Ft6x36uTouch::~Ft6x36uTouch() {

}

esp_err_t Ft6x36uTouch::init_device() {
    return ESP_OK;
}

esp_err_t Ft6x36uTouch::deinit_device() {
    return ESP_OK;
}

Ft6x36uTouch *g_p_ft6x36u_touch = NULL;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ft6x36u_i2c_read(i2c_port_t port, uint16_t addr, uint32_t reg, uint8_t *buffer, uint16_t size) {
	(void)port;
	(void)addr;

	if (g_p_ft6x36u_touch == NULL) {
		return ESP_FAIL;
	}

	return  g_p_ft6x36u_touch->read_buffer(reg, buffer, size);
}

#ifdef __cplusplus
}
#endif