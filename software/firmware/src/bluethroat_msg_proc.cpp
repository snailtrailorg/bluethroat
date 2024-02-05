#include <esp_log.h>
#include "bluethroat_msg_proc.h"

#define MSG_PROC_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define MSG_PROC_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define MSG_PROC_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define MSG_PROC_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define MSG_PROC_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define MSG_PROC_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define MSG_PROC_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define MSG_PROC_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define MSG_PROC_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define MSG_PROC_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define MSG_PROC_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			MSG_PROC_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define MSG_PROC_ASSERT(condition, format, ...) void
#endif

static const char * TAG = "MSG_PROC";

BluethroatMsgProc::BluethroatMsgProc(char * task_name, uint32_t task_stack_size, UBaseType_t task_priority, BaseType_t task_core_id, TickType_t task_interval) : 
m_task_name(task_name), m_task_stack_size(task_stack_size), m_task_priority(task_priority), m_task_core_id(task_core_id) {
	MSG_PROC_LOGI("Create message process task %s", this->m_task_name);

}

BluethroatMsgProc::~BluethroatMsgProc() {
    MSG_PROC_ASSERT(false, "Message process instance should not be destroyed in any condition.");
}

inline QueueHandle_t BluethroatMsgProc::GetMsgQueueHandle() {
    return m_queue_handle;
}

void BluethroatMsgProc::create_task() {
	if (pdPASS == xTaskCreatePinnedToCore(bluethroat_msg_proc_func, this->m_task_name, this->m_task_stack_size, this, this->m_task_priority, &(this->m_task_handle), m_task_core_id)) {
		MSG_PROC_LOGV("Create message process task %s successfully.", this->m_task_name);
	} else {
		MSG_PROC_LOGV("Create message process task %s failed.", this->m_task_name);
	}
}

void BluethroatMsgProc::task_loop() {
    
}

static void bluethroat_msg_proc_func(void * p_param) {
	BluethroatMsgProc * p_bluethroat_msg_proc = (BluethroatMsgProc *)p_param;
    p_bluethroat_msg_proc->task_loop();
}