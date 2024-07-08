#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <time.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "utilities/sme_float.h"
#include "bluethroat_bluetooth.h"

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
    char sentence[MNEA_SENTENCE_MAX_SIZE];
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
                    BluetoothSendGnssNmea(sentence);

                    {
                        BluethroatMsg_t message;

                        if (strncmp(sentence, "$GNRMC", strlen("$GNRMC")) == 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
                            struct tm time = {0};
#pragma GCC diagnostic pop
                            char status;
                            uint32_t latitude[2];
                            char latitude_direction;
                            uint32_t longitude[2];
                            char longitude_direction;
                            float course;
                            char buffer[16];
                            float second; 

                            /* $GNRMC,080152.00,A,2236.01533,N,11400.47834,E,0.949,179.38,070724,,,D,V*0E */
                            if (sscanf(sentence, "$GNRMC,%2d%2d%2d.%*d,%c,%ld.%ld,%c,%ld.%ld,%c,%*f,%f,%2d%2d%2d", 
                            &(time.tm_hour), &time.tm_min, &time.tm_sec, &status,
                            &(latitude[0]), &(latitude[1]), &latitude_direction, 
                            &(longitude[0]), &(longitude[1]), &longitude_direction, 
                            &course, &time.tm_mday, &time.tm_mon, &time.tm_year) == 14 && status == 'A') {
                                message.type = BLUETHROAT_MSG_TYPE_GNSS_ZDA_DATA;
                                message.gnss_zda_data.second = time.tm_sec;
                                message.gnss_zda_data.minute = time.tm_min;
                                message.gnss_zda_data.hour = time.tm_hour;
                                message.gnss_zda_data.day = time.tm_mday;
                                message.gnss_zda_data.month = time.tm_mon;
                                message.gnss_zda_data.year = time.tm_year;

                                (void)xQueueSend(m_queue_handle, &message, 0);

                                NEO_M9N_GNSS_LOGD("Report GNSS ZDA data, time: %04d-%02d-%02d %02d:%02d:%02d", 
                                    message.gnss_zda_data.year, message.gnss_zda_data.month, message.gnss_zda_data.day, 
                                    message.gnss_zda_data.hour, message.gnss_zda_data.minute, message.gnss_zda_data.second);
                            
                                message.type = BLUETHROAT_MSG_TYPE_GNSS_RMC_DATA;
                                message.gnss_rmc_data.latitude_degree = latitude[0] / 100;
                                message.gnss_rmc_data.latitude_minute = latitude[0] % 100;
                                message.gnss_rmc_data.latitude_direction = (latitude_direction == 'N') ? GNSS_LATITUDE_DIRECTION_NORTH : GNSS_LATITUDE_DIRECTION_SOUTH;
                                sprintf(buffer, "0.%ld", latitude[1]); sscanf(buffer, "%f", &second);
                                message.gnss_rmc_data.latitude_second = (float)(float32_t(second) * float32_t(60.0F));
                                message.gnss_rmc_data.langitude_degree = longitude[0] / 100;
                                message.gnss_rmc_data.langitude_minute = longitude[0] % 100;
                                message.gnss_rmc_data.langitude_direction = (longitude_direction == 'E') ? GNSS_LONGITUDE_DIRECTION_EAST : GNSS_LONGITUDE_DIRECTION_WEST;
                                sprintf(buffer, "0.%ld", longitude[1]); sscanf(buffer, "%f", &second);
                                message.gnss_rmc_data.langitude_second = (float)(float32_t(second) * float32_t(60.0F));
                                message.gnss_rmc_data.course = course;

                                (void)xQueueSend(m_queue_handle, &message, 0);

                                NEO_M9N_GNSS_LOGD("Report GNSS RMC data, latitude:%d°%d'%f\" %c, longitude:%d°%d'%f\" %c, course:%f", 
                                    message.gnss_rmc_data.latitude_degree, message.gnss_rmc_data.latitude_minute, message.gnss_rmc_data.latitude_second, 
                                    (message.gnss_rmc_data.latitude_direction == GNSS_LATITUDE_DIRECTION_NORTH) ? 'N' : 'S',
                                    message.gnss_rmc_data.langitude_degree, message.gnss_rmc_data.langitude_minute, message.gnss_rmc_data.langitude_second, 
                                    (message.gnss_rmc_data.langitude_direction == GNSS_LONGITUDE_DIRECTION_EAST) ? 'E' : 'W',
                                    message.gnss_rmc_data.course);

                            } else {
                                NEO_M9N_GNSS_LOGD("Parse GNSS RMC data failed.");
                            }
                        } else if (strncmp(sentence, "$GNGGA", strlen("$GNGGA")) == 0) {
                            uint32_t latitude[2];
                            char latitude_direction;
                            uint32_t longitude[2];
                            char longitude_direction;
                            float altitude;
                            float undulation;
                            char buffer[16];
                            float second;

                            /* $GNGGA,080152.00,2236.01533,N,11400.47834,E,2,12,1.00,159.0,M,-2.5,M,,0000*53 */
                            if (sscanf(sentence, "$GNGGA,%*f,%ld.%ld,%c,%ld.%ld,%c,%*d,%*d,%*f,%f,M,%f,M", 
                            &(latitude[0]), &(latitude[1]), &latitude_direction, 
                            &(longitude[0]), &(longitude[1]), &longitude_direction, 
                            &altitude, &undulation) == 8) {
                                message.type = BLUETHROAT_MSG_TYPE_GNSS_GGA_DATA;
                                message.gnss_gga_data.latitude_degree = latitude[0] / 100;
                                message.gnss_gga_data.latitude_minute = latitude[0] % 100;
                                message.gnss_gga_data.latitude_direction = (latitude_direction == 'N') ? GNSS_LATITUDE_DIRECTION_NORTH : GNSS_LATITUDE_DIRECTION_SOUTH;
                                sprintf(buffer, "0.%ld", latitude[1]); sscanf(buffer, "%f", &second);
                                message.gnss_gga_data.latitude_second = (float)(float32_t(second) * float32_t(60.0F));
                                message.gnss_gga_data.langitude_degree = longitude[0] / 100;
                                message.gnss_gga_data.langitude_minute = longitude[0] % 100;
                                message.gnss_gga_data.langitude_direction = (longitude_direction == 'E') ? GNSS_LONGITUDE_DIRECTION_EAST : GNSS_LONGITUDE_DIRECTION_WEST;
                                sprintf(buffer, "0.%ld", longitude[1]); sscanf(buffer, "%f", &second);
                                message.gnss_gga_data.langitude_second = (float)(float32_t(second) * float32_t(60.0F));
                                message.gnss_gga_data.altitude = float(float32_t(altitude) + float32_t(undulation));

                                (void)xQueueSend(m_queue_handle, &message, 0);

                                NEO_M9N_GNSS_LOGD("Report GNSS GGA data, latitude:%d°%d'%f\" %c, longitude:%d°%d'%f\" %c, altitude:%f", 
                                    message.gnss_gga_data.latitude_degree, message.gnss_gga_data.latitude_minute, message.gnss_gga_data.latitude_second, 
                                    (message.gnss_gga_data.latitude_direction == GNSS_LATITUDE_DIRECTION_NORTH) ? 'N' : 'S',
                                    message.gnss_gga_data.langitude_degree, message.gnss_gga_data.langitude_minute, message.gnss_gga_data.langitude_second, 
                                    (message.gnss_gga_data.langitude_direction == GNSS_LONGITUDE_DIRECTION_EAST) ? 'E' : 'W',
                                    message.gnss_gga_data.altitude);
                            } else {
                                NEO_M9N_GNSS_LOGD("Parse GNSS GGA data failed.");
                            }
                        } else if (strncmp(sentence, "$GNVTG", strlen("$GNVTG")) == 0) {
                            /* $GNVTG,179.38,T,,M,0.949,N,1.757,K,D*22 */
                            if (sscanf(sentence, "$GNVTG,%f,%*sM,%f,N,%f,K", &(message.gnss_vtg_data.course), &(message.gnss_vtg_data.speed_knot), &(message.gnss_vtg_data.speed_kmh)) == 3) {
                                message.type = BLUETHROAT_MSG_TYPE_GNSS_VTG_DATA;

                                (void)xQueueSend(m_queue_handle, &message, 0);

                                NEO_M9N_GNSS_LOGD("Report GNSS VTG data, course:%f, speed(knot):%f, speed(kmh):%f", 
                                    message.gnss_vtg_data.course, message.gnss_vtg_data.speed_knot, message.gnss_vtg_data.speed_kmh);
                            } else {
                                NEO_M9N_GNSS_LOGD("Parse GNSS VTG data failed.");
                            }
                        } else {
                            NEO_M9N_GNSS_LOGV("Unknown GNSS data.");
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
	}
}