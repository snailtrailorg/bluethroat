#include <esp_log.h>
#include <time.h>

#include "bluethroat_msg_proc.h"
#include "drivers/bm8563_rtc.h"

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
#define MSG_PROC_ASSERT(condition, format, ...)
#endif

static const char *TAG = "MSG_PROC";

BluethroatMsgProc::BluethroatMsgProc(char *task_name, uint32_t task_stack_size, UBaseType_t task_priority, BaseType_t task_core_id, TickType_t task_interval) : 
m_task_name(task_name), m_task_stack_size(task_stack_size), m_task_priority(task_priority), m_task_core_id(task_core_id) {
	MSG_PROC_LOGI("Start blurthraot message procedure.");
	this->m_queue_handle = xQueueCreate(BLUETHROAT_MSG_QUEUE_LENGTH, sizeof(BluethroatMsg_t));
	if (this->m_queue_handle != NULL) {
		MSG_PROC_LOGI("Create message queue %s success.", this->m_task_name);
	} else {
		MSG_PROC_LOGE("Create message queue %s failed", this->m_task_name);
	}

	if (pdPASS == xTaskCreatePinnedToCore(message_loop_c_entry, this->m_task_name, this->m_task_stack_size, this, this->m_task_priority, &(this->m_task_handle), m_task_core_id)) {
		MSG_PROC_LOGI("Create message task %s successfully.", this->m_task_name);
	} else {
		MSG_PROC_LOGE("Create message task %s failed.", this->m_task_name);
	}
}

BluethroatMsgProc::~BluethroatMsgProc() {
    MSG_PROC_ASSERT(false, "Message process instance should not be destroyed in any condition.");
}

void BluethroatMsgProc::message_loop() {
    MSG_PROC_ASSERT(this->m_queue_handle != NULL, "Invalid message queue handle.");
	static BluethroatMsg_t message;

	for ( ; ; ) {
		if (pdTRUE == xQueueReceive(this->m_queue_handle, &message, portMAX_DELAY)) {
			switch (message.type) {
			case BLUETHROAT_MSG_RTC:
				struct tm stm_time;
				stm_time.tm_sec = message.rtc_data.second,
				stm_time.tm_min = message.rtc_data.minute,
				stm_time.tm_hour = message.rtc_data.hour,
				stm_time.tm_mday = message.rtc_data.day,
				stm_time.tm_mon = message.rtc_data.month,
				stm_time.tm_year =message.rtc_data.year,
				Bm8563Rtc::SetSysTime(&stm_time);
				break;

			case BLUETHROAT_MSG_BAROMETER:
				break;

    		case BLUETHROAT_MSG_HYGROMETER:
				break;

    		case BLUETHROAT_MSG_ANEMOMETER:
				break;

    		case BLUETHROAT_MSG_ACCELERATION:
				break;

    		case BLUETHROAT_MSG_ROTATION:
				break;

    		case BLUETHROAT_MSG_GEOMAGNATIC:
				break;

    		case BLUETHROAT_MSG_POWER:
				break;

    		case BLUETHROAT_MSG_GPS:
				break;
				
    		default:
				MSG_PROC_ASSERT(false, "Unknown message type."); 
			}
		} else {
			MSG_PROC_LOGD("Receive message from queue timeout.");
		}
	}
}

void message_loop_c_entry(void *p_param) {
	BluethroatMsgProc *p_bluethroat_msg_proc = (BluethroatMsgProc *)p_param;
    p_bluethroat_msg_proc->message_loop();
}