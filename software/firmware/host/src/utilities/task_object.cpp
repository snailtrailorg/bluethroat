#include <esp_log.h>
#include "utilities/task_object.h"
#include "bluethroat_msg_proc.h"

#define TASK_OBJ_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define TASK_OBJ_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define TASK_OBJ_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define TASK_OBJ_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define TASK_OBJ_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define TASK_OBJ_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define TASK_OBJ_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define TASK_OBJ_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define TASK_OBJ_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define TASK_OBJ_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define TASK_OBJ_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			TASK_OBJ_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define TASK_OBJ_ASSERT(condition, format, ...)
#endif

static const char *TAG = "TASK_OBJ";

TaskObject::TaskObject() : m_p_task_param(NULL), m_task_handle(NULL), m_queue_handle(NULL) {
    m_p_object_name = TAG;
}

TaskObject::~TaskObject() {
}

esp_err_t TaskObject::Init() {
	return this->init_device();
}

esp_err_t TaskObject::Deinit() {
	return this->deinit_device();
}

esp_err_t TaskObject::Start(const TaskParam_t *p_task_param, QueueHandle_t queue_handle) {
	m_p_task_param = p_task_param;
	m_queue_handle = queue_handle;

	if (this->m_p_task_param != NULL) {
    	return this->create_task();
	} else {
		return ESP_FAIL;
	}
}

esp_err_t TaskObject::Stop() {
	return this->delete_task();
}

void TaskObject::SetMessageQueue(QueueHandle_t queue_handle) {
	m_queue_handle = queue_handle;
}

esp_err_t TaskObject::create_task() {
	TASK_OBJ_LOGI("Create task for object %s.", this->m_p_object_name);
    TASK_OBJ_ASSERT(this->m_task_handle == NULL, "Task %s has been created, can not create again.", this->m_p_task_param->task_name);
	TASK_OBJ_ASSERT(this->m_p_task_param != NULL && this->m_p_task_param->task_name != NULL, "Invalid task parameter pointer, can not create task for object %s.", this->m_p_object_name);
	if (pdPASS == xTaskCreatePinnedToCore(task_c_entry, this->m_p_task_param->task_name, this->m_p_task_param->task_stack_size, this, this->m_p_task_param->task_priority, &(this->m_task_handle), this->m_p_task_param->task_core_id)) {
		TASK_OBJ_LOGI("Create object task %s success.", this->m_p_task_param->task_name);
		return ESP_OK;
	} else {
		TASK_OBJ_LOGE("Create object task %s failed.", this->m_p_task_param->task_name);
		return ESP_FAIL;
	}
}

esp_err_t TaskObject::delete_task() {
	TASK_OBJ_ASSERT(this->m_p_task_param != NULL && this->m_p_task_param->task_name != NULL, "Invalid task parameter pointer, can not delete task for object %s.", this->m_p_object_name);
	TASK_OBJ_ASSERT(this->m_task_handle != NULL, "Task %s has not been created, can not delete.", this->m_p_task_param->task_name);
	TASK_OBJ_LOGI("Delete object task %s.", this->m_p_task_param->task_name);
	vTaskDelete(this->m_task_handle);
	return ESP_OK;
}

void task_c_entry(void *p_param) {
	TaskObject *p_object = (TaskObject *)p_param;
	p_object->task_cpp_entry();
}
