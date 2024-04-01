#include "drivers/dps3xx_anemometer.h"

#define DPS3XX_ANEMO_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define DPS3XX_ANEMO_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define DPS3XX_ANEMO_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define DPS3XX_ANEMO_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define DPS3XX_ANEMO_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define DPS3XX_ANEMO_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define DPS3XX_ANEMO_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define DPS3XX_ANEMO_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define DPS3XX_ANEMO_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define DPS3XX_ANEMO_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define DPS3XX_ANEMO_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			DPS3XX_ANEMO_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define DPS3XX_ANEMO_ASSERT(condition, format, ...)
#endif

static const char *TAG = "DPS3XX_ANEMO";

Dps3xxAnemometer::Dps3xxAnemometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle, Dps3xxBarometer *p_barometer)
 : Dps3xxBarometer(p_i2c_master, device_addr, p_task_param, queue_handle),
   m_p_barometer(p_barometer) {
    DPS3XX_ANEMO_LOGI("Create DPS3XX anemometer at port %d, device_addr 0x%3x", this->m_p_i2c_master->m_port, this->m_device_addr);
}

Dps3xxAnemometer::~Dps3xxAnemometer() {
}

esp_err_t Dps3xxAnemometer::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    if (Dps3xxBarometer::process_data(in_data, in_size, p_message) != ESP_OK) {
        DPS3XX_ANEMO_LOGE("Failed to process barometer data in anemometer class");
        return ESP_FAIL;
    }

    float32_t total_pressure = float32_t(POSITIVE, this->m_p_deep_filter->GetAverage(), -10);
    float32_t static_pressure = float32_t(POSITIVE, this->m_p_barometer->m_p_deep_filter->GetAverage(), -10);

    DPS3XX_ANEMO_LOGD("%f %f %f %f", p_message->barometer_data.temperature, (float)total_pressure, (float)static_pressure, (float)(total_pressure - static_pressure));

    p_message->type = BLUETHROAT_MSG_ANEMOMETER;
    // It is not necessary to copy the temperature from barometer data to anemometer data since they are in the same place in the union
    // p_message->anemometer_data.temperature = p_message->barometer_data.temperature;
    p_message->anemometer_data.total_pressure = (float)total_pressure;
    p_message->anemometer_data.static_pressure = (float)static_pressure;

    return ESP_OK;
}
