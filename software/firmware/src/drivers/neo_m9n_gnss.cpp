#include <esp_log.h>
#include <driver/gpio.h>

#include "drivers/neo_m9n_gnss.h"

#define NEO_M9N_GNSS_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define NEO_M9N_GNSS_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define NEO_M9N_GNSS_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define NEO_M9N_GNSS_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define NEO_M9N_GNSS_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define NEO_M9N_GNSS_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define NEO_M9N_GNSS_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define NEO_M9N_GNSS_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define NEO_M9N_GNSS_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define NEO_M9N_GNSS_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define NEO_M9N_GNSS_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			NEO_M9N_GNSS_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define NEO_M9N_GNSS_ASSERT(condition, format, ...)
#endif

static const char *TAG = "NEO_M9N_GNSS";

NeoM9nGnss::NeoM9nGnss() : GeneralDevice(), m_uart_port(UART_NUM_MAX), m_uart_tx_pin(GPIO_NUM_NC), m_uart_rx_pin(GPIO_NUM_NC), m_uart_rts_pin(GPIO_NUM_NC), m_uart_cts_pin(GPIO_NUM_NC), m_uart_baudrate(0) {
	m_p_device_name = TAG;
    NEO_M9N_GNSS_LOGD("Create %s device.", m_p_device_name);
}

NeoM9nGnss::~NeoM9nGnss() {
}

esp_err_t NeoM9nGnss::Init(uart_port_t uart_port, gpio_num_t tx_pin, gpio_num_t rx_pin, gpio_num_t rts_pin, gpio_num_t cts_pin, int baudrate) {
    m_uart_port = uart_port;
    m_uart_tx_pin = tx_pin;
    m_uart_rx_pin = rx_pin;
    m_uart_rts_pin = rts_pin;
    m_uart_cts_pin = cts_pin;
    m_uart_baudrate = baudrate;

	return GeneralDevice::Init();
}

esp_err_t NeoM9nGnss::Deinit() {
	return GeneralDevice::Deinit();
}

esp_err_t NeoM9nGnss::init_device() {
	return ESP_OK;
}

esp_err_t NeoM9nGnss::deinit_device() {
	return ESP_OK;
}

esp_err_t NeoM9nGnss::fetch_data(uint8_t *data, uint8_t size) {
	(void)data;
	(void)size;
	return ESP_OK;
}

esp_err_t NeoM9nGnss::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
	(void)in_data;
	(void)in_size;
	(void)p_message;
	return ESP_OK;
}
