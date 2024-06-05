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
#define I2C_DEVICE_ASSERT(condition, format, ...)
#endif

static const char *TAG = "I2C_DEVICE";

I2cDevice::I2cDevice() : 
m_p_i2c_master(NULL), m_device_addr(0), m_p_int_pins(NULL),
m_p_task_param(NULL), m_task_handle(NULL), 
m_queue_handle(NULL) {
}

I2cDevice::~I2cDevice() {
}

esp_err_t I2cDevice::CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr) {
	(void)p_i2c_master;
	(void)device_addr;
	return ESP_OK;
}

esp_err_t I2cDevice::init_device() {
	return ESP_OK;
}

esp_err_t I2cDevice::deinit_device() {
	return ESP_OK;
}

esp_err_t I2cDevice::fetch_data(uint8_t *data, uint8_t size) {
	(void)data;
	(void)size;
	return ESP_OK;
}

esp_err_t I2cDevice::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
	(void)in_data;
	(void)in_size;
	(void)p_message;
	return ESP_OK;
}

esp_err_t I2cDevice::Init(I2cMaster *p_i2c_master, uint16_t device_addr, const gpio_num_t *p_int_pins) {
	m_p_i2c_master = p_i2c_master;
	m_device_addr = device_addr;
	m_p_int_pins = p_int_pins;

	I2C_DEVICE_LOGI("Initialize I2C device at port %d, device_addr 0x%3x.", this->m_p_i2c_master->m_port, this->m_device_addr);

	esp_err_t result = this->init_device();

	if (result != ESP_OK) {
		I2C_DEVICE_LOGE("Initialize I2C device failed, error code: %d.", result);
	}

	return result;
}

esp_err_t I2cDevice::Deinit() {
	I2C_DEVICE_LOGI("Deinitialize I2C device at port %d, device_addr 0x%3x.", this->m_p_i2c_master->m_port, this->m_device_addr);
	
	esp_err_t result = this->deinit_device();

	if (result != ESP_OK) {
		I2C_DEVICE_LOGE("Deinitialize I2C device failed, error code: %d.", result);
	}

	return result;
}

esp_err_t I2cDevice::Start(const TaskParam_t *p_task_param, QueueHandle_t queue_handle) {
	m_p_task_param = p_task_param;
	m_queue_handle = queue_handle;

	if (m_p_task_param != NULL) {
		I2C_DEVICE_LOGI("Create I2C device task %s", this->m_p_task_param->task_name);

		esp_err_t result = this->create_task();

		if (result != ESP_OK) {
			I2C_DEVICE_LOGE("Create I2C device task %s failed, error code: %d.", this->m_p_task_param->task_name, result);
		}

		return result;
	} else {
		I2C_DEVICE_LOGD("Null I2C device task parameter pointer, no task created.");

		return ESP_OK;
	}
}

esp_err_t I2cDevice::Stop() {
	I2C_DEVICE_LOGI("Delete I2C device task %s", this->m_p_task_param->task_name);

	esp_err_t result = this->delete_task();

	if (result != ESP_OK) {
		I2C_DEVICE_LOGE("Delete I2C device task %s failed, error code: %d.", this->m_p_task_param->task_name, result);
	}

	return result;
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

esp_err_t I2cDevice::create_task() {
	I2C_DEVICE_ASSERT(this->m_p_task_param != NULL, "Invalid I2C device task parameter pointer.");
	if (pdPASS == xTaskCreatePinnedToCore(i2c_device_task_func, this->m_p_task_param->task_name, this->m_p_task_param->task_stack_size, this, this->m_p_task_param->task_priority, &(this->m_task_handle), this->m_p_task_param->task_core_id)) {
		I2C_DEVICE_LOGV("Create I2C device task %s successfully.", this->m_p_task_param->task_name);
		return ESP_OK;
	} else {
		I2C_DEVICE_LOGV("Create I2C device task %s failed.", this->m_p_task_param->task_name);
		return ESP_FAIL;
	}
}

esp_err_t I2cDevice::delete_task() {
	vTaskDelete(this->m_task_handle);
	return ESP_OK;
}

#define MAX_RAW_DATA_BUFFER_LENGTH		(32)
void I2cDevice::task_loop() {
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
				I2C_DEVICE_LOGE("Task %s process I2C data failed.", (this->m_p_task_param->task_name) ? this->m_p_task_param->task_name : "UNKNOWN");
			}
		} else {
			I2C_DEVICE_LOGE("Task %s fetch I2C data failed.", (this->m_p_task_param->task_name) ? this->m_p_task_param->task_name : "UNKNOWN");
		}

		if (this->m_p_task_param->task_interval > 0) {
			vTaskDelay(this->m_p_task_param->task_interval);
		}
	}
}

void i2c_device_task_func(void *p_param) {
	I2cDevice *p_device = (I2cDevice *)p_param;
	p_device->task_loop();
}
