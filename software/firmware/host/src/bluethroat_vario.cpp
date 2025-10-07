#include <stddef.h>
#include <math.h>

#include <esp_log.h>

#include "bluethroat_vario.h"

#define BLUETHROAT_VARIO_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_VARIO_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_VARIO_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_VARIO_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_VARIO_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define BLUETHROAT_VARIO_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define BLUETHROAT_VARIO_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define BLUETHROAT_VARIO_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define BLUETHROAT_VARIO_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define BLUETHROAT_VARIO_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define BLUETHROAT_VARIO_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			BLUETHROAT_VARIO_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define BLUETHROAT_VARIO_ASSERT(condition, format, ...)
#endif

static const char *TAG = "BLUETHROAT_VARIO";

BluethraotVario::BluethraotVario() : m_latitude_degree(0), m_latitude_minute(0), m_latitude_second(0.0f), m_longitude_degree(0), m_longitude_minute(0), m_longitude_second(0.0f), m_altitude(0), m_last_temperature(0.0f), m_last_pressure(0.0f), m_last_timestamp(0) {
    g_pBluethraotVario = this;
}

BluethraotVario::~BluethraotVario() {
    g_pBluethraotVario = NULL;
}

float BluethraotVario::CalculateVerticalSpeed(float temperature, float pressure, uint32_t timestamp) {
    (void)temperature;
    float vertical_speed = 0.0f;

    if (m_last_pressure != 0.0f) {
        float elevation = 44330.0f * (1.0f - pow(pressure / m_last_pressure, 0.1903f));
        float delta_time = (float)(timestamp - m_last_timestamp) / 1000.0f;
        vertical_speed = elevation / delta_time;
    }

    BLUETHROAT_VARIO_LOGD("last_temp:%f, last_pres:%f, last_time:%ld, temp:%f, pres:%f, time:%ld, vertical_speed:%f",
        m_last_temperature, m_last_pressure, m_last_timestamp, temperature, pressure, timestamp, vertical_speed);

    m_last_temperature = temperature;
    m_last_pressure = pressure;
    m_last_timestamp = timestamp;

    return vertical_speed;
}

BluethraotVario *g_pBluethraotVario = new BluethraotVario();

float CalculateVerticalSpeed(float temperature, float pressure, uint32_t timestamp) {
    if (g_pBluethraotVario) {
        return g_pBluethraotVario->CalculateVerticalSpeed(temperature, pressure, timestamp);
    } else {
        BLUETHROAT_VARIO_LOGE("BluethraotVario instance is NULL");
        return 0.0f;
    }
}