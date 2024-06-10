#include <esp_log.h>
#include <esp_err.h>
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

NeoM9nGnss::NeoM9nGnss() : TaskObject(), m_uart_port(UART_NUM_MAX), m_uart_tx_pin(GPIO_NUM_NC), m_uart_rx_pin(GPIO_NUM_NC), m_uart_rts_pin(GPIO_NUM_NC), m_uart_cts_pin(GPIO_NUM_NC), m_uart_baudrate(0) {
	m_p_object_name = TAG;
    NEO_M9N_GNSS_LOGI("Create %s device.", m_p_object_name);
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

	return TaskObject::Init();
}

esp_err_t NeoM9nGnss::Deinit() {
	return TaskObject::Deinit();
}

esp_err_t NeoM9nGnss::init_device() {
	NEO_M9N_GNSS_LOGI("Init %s device.", m_p_object_name);

	const uart_config_t uart_config = {
    	.baud_rate = m_uart_baudrate,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
	};

	ESP_ERROR_CHECK(uart_param_config(m_uart_port, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(m_uart_port, m_uart_tx_pin, m_uart_rx_pin, m_uart_rts_pin, m_uart_cts_pin));
	ESP_ERROR_CHECK(uart_driver_install(m_uart_port, UART_RECEIVE_BUFFER_SIZE, 0, UART_EVENT_QUEUE_SIZE, &m_uart_queue, 0));
	ESP_ERROR_CHECK(uart_enable_pattern_det_baud_intr(m_uart_port, '\n', 1, UART_BAUDRATE_CYCLE_PER_BYTE, 0, 0));
	ESP_ERROR_CHECK(uart_pattern_queue_reset(m_uart_port, UART_PATTERN_QUEUE_SIZE));
	//ESP_ERROR_CHECK(uart_enable_rx_intr(m_uart_port));

	return ESP_OK;
}

esp_err_t NeoM9nGnss::deinit_device() {
	return ESP_OK;
}

void NeoM9nGnss::task_cpp_entry() {
    uart_event_t event;
    size_t buffered_size;
    uint8_t sentence[MNEA_SENTENCE_MAX_SIZE];
	int position;
    uint32_t read_length;
    
	for ( ; ; ) {
        if (xQueueReceive(m_uart_queue, (void *)(&event), portMAX_DELAY)) {

            switch (event.type) {
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
                NEO_M9N_GNSS_LOGD("Uart[%d] fifo or buffer full", m_uart_port);
                uart_flush_input(m_uart_port);
                xQueueReset(m_uart_queue);
                break;
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(m_uart_port, &buffered_size);
                position = uart_pattern_pop_pos(m_uart_port);
                if (position == -1 || position >= MNEA_SENTENCE_MAX_SIZE || position >= buffered_size) {
                    NEO_M9N_GNSS_LOGD("Uart[%d] pattern position is invalid", m_uart_port);
					uart_flush_input(m_uart_port);
                    xQueueReset(m_uart_queue);
                } else {
					read_length = position + 1;
                    uart_read_bytes(m_uart_port, sentence, read_length, UART_RECEIVE_TIMEOUT);
					sentence[read_length-2] = '\0';
					NEO_M9N_GNSS_LOGD("Uart[%d] receive data: %s", m_uart_port, sentence);
                }
                break;
            default:
                break;
            }
        }
	}
}