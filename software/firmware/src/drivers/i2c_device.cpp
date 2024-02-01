#include "drivers/i2c_device.h"

#define I2C_DEVICE_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define I2C_DEVICE_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define I2C_DEVICE_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define I2C_DEVICE_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define I2C_DEVICE_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define I2C_DEVICE_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define I2C_DEVICE_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define I2C_DEVICE_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define I2C_DEVICE_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define I2C_DEVICE_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define I2C_DEVICE_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			I2C_DEVICE_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define I2C_DEVICE_ASSERT(condition, format, ...) void
#endif

static const char * TAG = "I2C_DEVICE";

I2cDevice::I2cDevice(I2cMaster * pI2cMaster) : m_pI2cMaster(pI2cMaster) {

}

esp_err_t I2cDevice::Read(uint16_t addr, uint32_t reg, uint8_t * buffer, uint16_t size) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer");
    m_pI2cMaster->Read(addr, reg, buffer, size);
}

esp_err_t I2cDevice::Write(uint16_t addr, uint32_t reg, const uint8_t * buffer, uint16_t size) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer");
    m_pI2cMaster->Write(addr, reg, buffer, size);
}
