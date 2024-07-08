#include <esp_log.h>
#include <time.h>
#include <math.h>

#include "bluethroat_msg_proc.h"
#include "bluethroat_ui.h"

#include "drivers/bm8563_rtc.h"
#include "bluethroat_bluetooth.h"

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

BluethroatMsgProc::BluethroatMsgProc(const TaskParam_t *p_task_param) : m_p_task_param(p_task_param) {
	MSG_PROC_LOGI("Start blurthraot message procedure.");
	MSG_PROC_ASSERT(this->m_p_task_param != NULL, "Invalid message procedure task parameter pointer");
	this->m_queue_handle = xQueueCreate(BLUETHROAT_MSG_QUEUE_LENGTH, sizeof(BluethroatMsg_t));
	if (this->m_queue_handle != NULL) {
		MSG_PROC_LOGI("Create message queue %s success.", this->m_p_task_param->task_name);
	} else {
		MSG_PROC_LOGE("Create message queue %s failed", this->m_p_task_param->task_name);
	}

	if (pdPASS == xTaskCreatePinnedToCore(message_loop_c_entry, this->m_p_task_param->task_name, this->m_p_task_param->task_stack_size, this, this->m_p_task_param->task_priority, &(this->m_task_handle), this->m_p_task_param->task_core_id)) {
		MSG_PROC_LOGI("Create message task %s success.", this->m_p_task_param->task_name);
	} else {
		MSG_PROC_LOGE("Create message task %s failed.", this->m_p_task_param->task_name);
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
			MSG_PROC_LOGV("Receive message from queue, message type:%d.", message.type);
			switch (message.type) {
			case BLUETHROAT_MSG_TYPE_BUTTON_DATA:
				break;
			case BLUETHROAT_MSG_TYPE_RTC_DATA:
				struct tm stm_time;
				stm_time.tm_sec = message.rtc_data.second,
				stm_time.tm_min = message.rtc_data.minute,
				stm_time.tm_hour = message.rtc_data.hour,
				stm_time.tm_mday = message.rtc_data.day,
				stm_time.tm_mon = message.rtc_data.month,
				stm_time.tm_year =message.rtc_data.year,
				Bm8563Rtc::SetSysTime(&stm_time);
				break;

			case BLUETHROAT_MSG_TYPE_BAROMETER_DATA:
				BluetoothSendPressure(message.barometer_data.pressure);
				break;

    		case BLUETHROAT_MSG_TYPE_HYGROMETER_DATA:
				break;

    		case BLUETHROAT_MSG_TYPE_ANEMOMETER_DATA:
				break;

    		case BLUETHROAT_MSG_TYPE_ACCELERATION_DATA:
				break;

    		case BLUETHROAT_MSG_TYPE_ROTATION_DATA:
				break;

    		case BLUETHROAT_MSG_TYPE_GEOMAGNATIC_DATA:
				break;

    		case BLUETHROAT_MSG_TYPE_POWER_DATA:
				MSG_PROC_LOGD("Receive power message, battery voltage:%d, battery charging:%d, battery activiting:%d, charge undercurrent:%d.", message.pmu_data.battery_voltage, message.pmu_data.battery_charging, message.pmu_data.battery_activiting, message.pmu_data.charge_undercurrent);
				UiSetBatteryState(message.pmu_data.battery_voltage, message.pmu_data.battery_charging, message.pmu_data.battery_activiting, message.pmu_data.charge_undercurrent);
				break;

    		case BLUETHROAT_MSG_TYPE_GNSS_ZDA_DATA:
				break;
				
			case BLUETHROAT_MSG_TYPE_GNSS_RMC_DATA:
				break;

			case BLUETHROAT_MSG_TYPE_GNSS_GGA_DATA:
				break;

			case BLUETHROAT_MSG_TYPE_GNSS_VTG_DATA:
				break;

			case BLUETHROAT_MSG_TYPE_BLUETOOTH_STATE:
				MSG_PROC_LOGD("Receive bluetooth state message, environment service state:%d, nordic uart service state:%d.", message.bluetooth_state.environment_service_state, message.bluetooth_state.nordic_uart_service_state);
				if (message.bluetooth_state.environment_service_state == SERVICE_STATE_CONNECTED || message.bluetooth_state.nordic_uart_service_state == SERVICE_STATE_CONNECTED) {
					UiSetBluetoothState(BLURTOOTH_STATE_CONNECTED);
				} else {
					UiSetBluetoothState(BLURTOOTH_STATE_DISCONNECTED);
				}
				break;

			case BLUETHROAT_MSG_INVALID:
				MSG_PROC_LOGE("Receive invalid message, message type:%d(invalid).", message.type);
				break;

    		default:
				MSG_PROC_LOGE("Receive invalid message, message type:%d(unknown).", message.type);
			}
		} else {
			MSG_PROC_LOGV("Receive message from queue timeout.");
		}
	}
}

void message_loop_c_entry(void *p_param) {
	BluethroatMsgProc *p_bluethroat_msg_proc = (BluethroatMsgProc *)p_param;
    p_bluethroat_msg_proc->message_loop();
}