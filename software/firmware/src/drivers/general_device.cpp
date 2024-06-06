#include <esp_log.h>
#include "drivers/general_device.h"
#include "bluethroat_msg_proc.h"

#define GEN_DEVICE_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define GEN_DEVICE_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define GEN_DEVICE_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define GEN_DEVICE_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define GEN_DEVICE_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define GEN_DEVICE_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define GEN_DEVICE_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define GEN_DEVICE_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define GEN_DEVICE_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define GEN_DEVICE_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define GEN_DEVICE_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			GEN_DEVICE_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define GEN_DEVICE_ASSERT(condition, format, ...)
#endif

static const char *TAG = "GEN_DEVICE";

GeneralDevice::GeneralDevice() : m_p_task_param(NULL), m_task_handle(NULL), m_queue_handle(NULL) {
    m_p_device_name = TAG;
}

GeneralDevice::~GeneralDevice() {
}

esp_err_t GeneralDevice::Init() {
	return this->init_device();
}

esp_err_t GeneralDevice::Deinit() {
	return this->deinit_device();
}

esp_err_t GeneralDevice::Start(const TaskParam_t *p_task_param, QueueHandle_t queue_handle) {
	m_p_task_param = p_task_param;
	m_queue_handle = queue_handle;

	if (this->m_p_task_param != NULL) {
    	return this->create_task();
	} else {
		return ESP_FAIL;
	}
}

esp_err_t GeneralDevice::Stop() {
	return this->delete_task();
}

esp_err_t GeneralDevice::create_task() {
	GEN_DEVICE_LOGI("Create task for device %s.", this->m_p_device_name);
    GEN_DEVICE_ASSERT(this->m_task_handle == NULL, "Task %s has been created, can not create again.", this->m_p_task_param->task_name);
	GEN_DEVICE_ASSERT(this->m_p_task_param != NULL && this->m_p_task_param->task_name != NULL, "Invalid task parameter pointer, can not create task for device %s.", this->m_p_device_name);
	if (pdPASS == xTaskCreatePinnedToCore(device_task_c_entry, this->m_p_task_param->task_name, this->m_p_task_param->task_stack_size, this, this->m_p_task_param->task_priority, &(this->m_task_handle), this->m_p_task_param->task_core_id)) {
		GEN_DEVICE_LOGI("Create device task %s success.", this->m_p_task_param->task_name);
		return ESP_OK;
	} else {
		GEN_DEVICE_LOGE("Create device task %s failed.", this->m_p_task_param->task_name);
		return ESP_FAIL;
	}
}

esp_err_t GeneralDevice::delete_task() {
	GEN_DEVICE_ASSERT(this->m_p_task_param != NULL && this->m_p_task_param->task_name != NULL, "Invalid task parameter pointer, can not delete task for device %s.", this->m_p_device_name);
	GEN_DEVICE_ASSERT(this->m_task_handle != NULL, "Task %s has not been created, can not delete.", this->m_p_task_param->task_name);
	GEN_DEVICE_LOGI("Delete device task %s.", this->m_p_task_param->task_name);
	vTaskDelete(this->m_task_handle);
	return ESP_OK;
}

#define MAX_RAW_DATA_BUFFER_LENGTH		(128)
void GeneralDevice::device_task() {
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
				GEN_DEVICE_LOGE("Task %s process data failed.", this->m_p_task_param->task_name);
			}
		} else {
			GEN_DEVICE_LOGE("Task %s fetch data failed.", this->m_p_task_param->task_name);
		}

		if (this->m_p_task_param->task_interval > 0) {
			vTaskDelay(this->m_p_task_param->task_interval);
		}
	}
}

void device_task_c_entry(void *p_param) {
	GeneralDevice *p_device = (GeneralDevice *)p_param;
	p_device->device_task();
}
