#include <esp_log.h>
#include "drivers/i2c_device.h"
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
#define I2C_DEVICE_ASSERT(condition, format, ...) void
#endif

static const char *TAG = "I2C_DEVICE";

I2cDevice::I2cDevice(I2cMaster *p_i2c_master, uint16_t device_addr) : m_p_i2c_master(p_i2c_master), m_device_addr(device_addr), m_task_handle(NULL) {
	I2C_DEVICE_ASSERT(m_p_i2c_master != NULL, "Invalid I2C master pointer");
	I2C_DEVICE_LOGI("Create I2C device at port %d, device_addr 0x%3x");
	I2C_DEVICE_LOGI("Initialize I2C device");
	this->init_device();
}

I2cDevice::I2cDevice(I2cMaster *p_i2c_master, uint16_t device_addr, char *task_name, uint32_t task_stack_size, UBaseType_t task_priority, BaseType_t task_core_id, TickType_t task_interval, QueueHandle_t queue_handle) : 
m_p_i2c_master(p_i2c_master), m_device_addr(device_addr), 
m_task_name(task_name), m_task_stack_size(task_stack_size), m_task_priority(task_priority), m_task_core_id(task_core_id), 
m_queue_handle(queue_handle) {
	I2C_DEVICE_ASSERT(m_p_i2c_master != NULL, "Invalid I2C master pointer");
	I2C_DEVICE_LOGI("Create I2C device at port %d, device_addr 0x%3x");
	I2C_DEVICE_LOGI("Initialize I2C device");
	this->init_device();
	I2C_DEVICE_LOGI("Create I2C device task %s", this->m_task_name);
	this->create_task();
}

I2cDevice::~I2cDevice() {
	I2C_DEVICE_ASSERT(m_p_i2c_master != NULL, "Invalid I2C master pointer.");
	I2C_DEVICE_LOGI("Destroy I2C device at port %d, device_addr 0x%3x.");
	I2C_DEVICE_LOGI("Initialize I2C device.");
	this->deinit_device();
	if (this->m_task_handle != NULL) {
		I2C_DEVICE_LOGI("Delete I2C device task %s.", this->m_task_name);
		this->delete_task();
	}
}


inline esp_err_t I2cDevice::read_byte(uint32_t reg_addr, uint8_t *p_byte) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->ReadByte(this->m_device_addr, reg_addr, p_byte);
}

inline esp_err_t I2cDevice::write_byte(uint32_t reg_addr, const uint8_t byte_value) {

    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->WriteByte(this->m_device_addr, reg_addr, byte_value);
}

inline esp_err_t I2cDevice::read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->ReadBuffer(this->m_device_addr, reg_addr, buffer, size);
}

inline esp_err_t I2cDevice::write_buffer(uint32_t reg_addr, const uint8_t *buffer, uint16_t size) {
    I2C_DEVICE_ASSERT(this->m_pI2cMaster != NULL, "Invalid I2C master pointer.");
    return m_p_i2c_master->WriteBuffer(this->m_device_addr, reg_addr, buffer, size);
}

inline esp_err_t I2cDevice::create_task() {
	if (pdPASS == xTaskCreatePinnedToCore(i2c_device_task_func, this->m_task_name, this->m_task_stack_size, this, this->m_task_priority, &(this->m_task_handle), m_task_core_id)) {
		I2C_DEVICE_LOGV("Create I2C device task %s successfully.", this->m_task_name);
	} else {
		I2C_DEVICE_LOGV("Create I2C device task %s failed.", this->m_task_name);
	}
}

inline esp_err_t I2cDevice::delete_task() {
	vTaskDelete(this->m_task_handle);
}

#define MAX_RAW_DATA_BUFFER_LENGTH		(32)
inline void I2cDevice::task_loop() {
	uint8_t raw_data[MAX_RAW_DATA_BUFFER_LENGTH];
	BluethroatMsg_t  message;
	for ( ; ; ) {
		if (ESP_OK == this->fetch_data(raw_data, sizeof(raw_data))) {
			if(ESP_OK == this->calculate_data(raw_data, MAX_RAW_DATA_BUFFER_LENGTH, &message)) {
				if (this->m_queue_handle != NULL) {
					(void)xQueueSend(this->m_queue_handle, &message, 0);
				} else {
					; // not valid queue handle provided, needn't send message
				}
			} else {
				I2C_DEVICE_LOGD("Send message to queue failed.");
			}
		} else {
			I2C_DEVICE_LOGD("Fetch I2C data failed.");
		}
	}
}

static void i2c_device_task_func(void *p_param) {
	I2cDevice *p_device = (I2cDevice *)p_param;
	p_device->task_loop();
}
