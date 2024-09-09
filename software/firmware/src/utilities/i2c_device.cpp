#include <esp_log.h>
#include "utilities/i2c_device.h"
#include "bluethroat_msg_proc.h"

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
#define I2C_DEVICE_ASSERT(condition, format, ...)
#endif

static const char *TAG = "I2C_DEVICE";

I2cDevice::I2cDevice() : TaskObject(), m_p_i2c_master(NULL),	m_device_addr(0), m_p_int_pins(NULL) {
	m_p_object_name = TAG;
}

I2cDevice::~I2cDevice() {
}

esp_err_t I2cDevice::CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr) {
	(void)p_i2c_master;
	(void)device_addr;

	return ESP_OK;
}

esp_err_t I2cDevice::Init(I2cMaster *p_i2c_master, uint16_t device_addr, const gpio_num_t *p_int_pins) {
	m_p_i2c_master = p_i2c_master;
	m_device_addr = device_addr;
	m_p_int_pins = p_int_pins;

	return TaskObject::Init();
}

esp_err_t I2cDevice::Deinit() {
	return TaskObject::Deinit();
}

void I2cDevice::task_cpp_entry() {
	uint8_t raw_data[MAX_RAW_DATA_BUFFER_LENGTH];
	BluethroatMsg_t  message;
	for ( ; ; ) {
		if (ESP_OK == this->fetch_data(raw_data, sizeof(raw_data))) {
			if(ESP_OK == this->process_data(raw_data, MAX_RAW_DATA_BUFFER_LENGTH, &message)) {
				if (this->m_queue_handle != NULL && message.type != BLUETHROAT_MSG_INVALID) {
					(void)xQueueSend(this->m_queue_handle, &message, 0);
				} else {
					; // not valid queue handle provided, needn't send message
				}
			} else {
				I2C_DEVICE_LOGE("Task %s process data failed.", this->m_p_task_param->task_name);
			}
		} else {
			I2C_DEVICE_LOGE("Task %s fetch data failed.", this->m_p_task_param->task_name);
		}

		if (this->m_p_task_param->task_interval > 0) {
			vTaskDelay(this->m_p_task_param->task_interval);
		} else {
			//taskYIELD();
		}
	}
}

esp_err_t I2cDevice::read_byte(uint32_t reg_addr, uint8_t *p_byte) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->ReadByte(this->m_device_addr, reg_addr, p_byte);
}

esp_err_t I2cDevice::write_byte(uint32_t reg_addr, const uint8_t byte_value) {

    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->WriteByte(this->m_device_addr, reg_addr, byte_value);
}

esp_err_t I2cDevice::read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->ReadBuffer(this->m_device_addr, reg_addr, buffer, size);
}

esp_err_t I2cDevice::write_buffer(uint32_t reg_addr, const uint8_t *buffer, uint16_t size) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->WriteBuffer(this->m_device_addr, reg_addr, buffer, size);
}
