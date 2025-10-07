#include <esp_err.h>
#include <esp_log.h>

#include "utilities/i2s_master.h"

#define I2S_MASTER_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define I2S_MASTER_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define I2S_MASTER_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define I2S_MASTER_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define I2S_MASTER_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define I2S_MASTER_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define I2S_MASTER_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define I2S_MASTER_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define I2S_MASTER_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define I2S_MASTER_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define I2S_MASTER_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			I2S_MASTER_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define I2S_MASTER_ASSERT(condition, format, ...)
#endif

static const char *TAG = "I2S_MASTER";

I2sMaster::I2sMaster(i2s_port_t port, gpio_num_t mclk_pin, gpio_num_t bclk_pin, gpio_num_t ws_pin, gpio_num_t din_pin, gpio_num_t dout_pin, uint32_t sample_rate, i2s_data_bit_width_t bit_per_sample, uint8_t channel_num) {
	m_read_handle = NULL;
	m_write_handle = NULL;
	(void)this->init_controller(port, mclk_pin, bclk_pin, ws_pin, din_pin, dout_pin, sample_rate, bit_per_sample, channel_num);
}

I2sMaster::~I2sMaster() {
	(void)this->deinit_controller();
}

esp_err_t I2sMaster::init_controller(i2s_port_t port, gpio_num_t mclk_pin, gpio_num_t bclk_pin, gpio_num_t ws_pin, gpio_num_t din_pin, gpio_num_t dout_pin, uint32_t sample_rate, i2s_data_bit_width_t bit_per_sample, uint8_t channel_num) {
	I2S_MASTER_ASSERT(port >= 0 && port < SOC_I2S_NUM,  "Invalid I2S port number: %d, Maximum valid port number is: %d.", port, SOC_I2S_NUM-1);
	I2S_MASTER_LOGI("Starting initialize I2S master instance.");

    i2s_chan_config_t i2s_channel_cfg = I2S_CHANNEL_DEFAULT_CONFIG(port, I2S_ROLE_MASTER);
    esp_err_t result = i2s_new_channel(&i2s_channel_cfg, &m_write_handle, &m_read_handle);
	if (result != ESP_OK) {
		I2S_MASTER_LOGE("I2S channel creation failed, error number: %d", result);
		m_read_handle = NULL;
		m_write_handle = NULL;

		return result;
	} else {
		I2S_MASTER_LOGI("I2S channel creation succeeded.");
	}

    i2s_std_config_t i2s_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)bit_per_sample, ((channel_num == 1) ? I2S_SLOT_MODE_MONO : I2S_SLOT_MODE_STEREO)),
        .gpio_cfg = {
            .mclk = mclk_pin,
            .bclk = bclk_pin,
            .ws   = ws_pin,
            .dout = dout_pin,
            .din  = din_pin,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

	this->m_port = port;
    result = i2s_channel_init_std_mode(m_write_handle, &i2s_std_cfg);
	if (result != ESP_OK) {
		I2S_MASTER_LOGE("I2S transmite channel initialization failed, error number: %d", result);
		i2s_del_channel(m_read_handle);
		i2s_del_channel(m_write_handle);
		m_read_handle = NULL;
		m_write_handle = NULL;

		return result;
	} else {
		I2S_MASTER_LOGI("I2S transmite channel initialization succeeded.");
	}

    result = i2s_channel_init_std_mode(m_read_handle, &i2s_std_cfg);
	if (result != ESP_OK) {
		I2S_MASTER_LOGE("I2S receive channel initialization failed, error number: %d", result);
		i2s_del_channel(m_read_handle);
		i2s_del_channel(m_write_handle);
		m_read_handle = NULL;
		m_write_handle = NULL;

		return result;
	} else {
		I2S_MASTER_LOGI("I2S receive channel initialization succeeded.");
	}

	result = i2s_channel_enable(m_read_handle);
	if (result != ESP_OK) {
		I2S_MASTER_LOGE("I2S receive channel enable failed, error number: %d", result);
		i2s_del_channel(m_read_handle);
		i2s_del_channel(m_write_handle);
		m_read_handle = NULL;
		m_write_handle = NULL;

		return result;
	} else {
		I2S_MASTER_LOGI("I2S receive channel enable succeeded.");
	}

	result = i2s_channel_enable(m_write_handle);
	if (result != ESP_OK) {
		I2S_MASTER_LOGE("I2S transmite channel enable failed, error number: %d", result);
		i2s_del_channel(m_read_handle);
		i2s_del_channel(m_write_handle);
		m_read_handle = NULL;
		m_write_handle = NULL;

		return result;
	} else {
		I2S_MASTER_LOGI("I2S transmite channel enable succeeded.");
	}

	return ESP_OK;
}

esp_err_t I2sMaster::deinit_controller() {
	I2S_MASTER_LOGI("Dinitializing I2C master instance.");
	if (m_read_handle != NULL) {
		i2s_channel_disable(m_read_handle);
		i2s_del_channel(m_read_handle);
		m_read_handle = NULL;
	}

	if (m_write_handle != NULL) {
		i2s_channel_disable(m_write_handle);
		i2s_del_channel(m_write_handle);
		m_write_handle = NULL;
	}

	return ESP_OK;
}
