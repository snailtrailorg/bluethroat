#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <drivers/ft6x36u_touch.h>

#define FT6X36U_TOUCH_LOGE(format, ...)                 ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define FT6X36U_TOUCH_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define FT6X36U_TOUCH_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define FT6X36U_TOUCH_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define FT6X36U_TOUCH_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define FT6X36U_TOUCH_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define FT6X36U_TOUCH_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define FT6X36U_TOUCH_ASSERT(condition, format, ...)    \
	do                                                  \
	{                                                   \
		if (!(condition))                               \
		{                                               \
			FT6X36U_TOUCH_LOGE(format, ##__VA_ARGS__);  \
			assert(0);                                  \
		}                                               \
	} while (0)
#else
#define FT6X36U_TOUCH_ASSERT(condition, format, ...)
#endif

static const char *TAG = "FT6X36U_TOUCH";

Ft6x36uTouch *g_pFt6x36uTouch = NULL;

Ft6x36uTouch::Ft6x36uTouch() : I2cDevice(), m_long_press_time(BUTTON_DEFAULT_LONG_PRESS_TIME) {
	m_p_object_name = TAG;
	FT6X36U_TOUCH_LOGI("Create %s device.", m_p_object_name);
	g_pFt6x36uTouch = this;
}

Ft6x36uTouch::~Ft6x36uTouch() {

}

esp_err_t Ft6x36uTouch::init_device() {
    return ESP_OK;
}

esp_err_t Ft6x36uTouch::deinit_device() {
    return ESP_OK;
}

esp_err_t Ft6x36uTouch::fetch_data(uint8_t *data, uint8_t size) {
	(void)data;
	(void)size;
	return ESP_OK;
}

esp_err_t Ft6x36uTouch::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
	(void)in_data;
	(void)in_size;
	(void)p_message;
	return ESP_OK;
}

void Ft6x36uTouch::set_long_press_time(TickType_t long_press_time) {
	 m_long_press_time = long_press_time;
}

esp_err_t Ft6x36uTouch::read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size) {
	// According to the demond of lvgl driver, read ft6336u register data
	esp_err_t result = I2cDevice::read_buffer(reg_addr, buffer, size);

	// Determine whether it is reading touch data, if not, return directly
	if (unlikely(reg_addr != FT6X36U_REG_ADDR_TD_STATUS || size != FT6X36U_TOUCH_DATA_LENGTH)) {
		return result;
	}

	// If read touch data fail, return directly
	if (unlikely(result != ESP_OK)) {
		return result;
	}

	// Lvgl driver reading touch date duccesfully, intercept touch data for additional processing 
	// to implement customized touch screen buttons outside the TFT screen area

	// Static variables to store last touch state, last button index, and last press time
	static TouchState_t last_touch_state = TOUCH_STATE_RELEASED;
	static ButtonIndex_t last_button_index = BUTTON_INDEX_NONE;
	static TickType_t last_press_time = 0;

	// Dynamic variables to store current touch state, current button index
	TouchState_t touch_state;
	ButtonIndex_t button_index;

	// Get current touch state: pressed or released
	touch_state = (buffer[0]) ? TOUCH_STATE_PRESSED : TOUCH_STATE_RELEASED;

	// If the touch state is released, and the last touch state is also released, return directly
	if (touch_state == TOUCH_STATE_RELEASED && last_touch_state == TOUCH_STATE_RELEASED) {
		return ESP_OK;
	}

	// If the touch state is pressed, or touch state is changed, calculate the button index
	// In order to improve efficiency, determine the y coordinate first, then determine the x coordinate.

	// Get the touch point y coordinates
	uint16_t touch_y = (((uint16_t)(buffer[3]) & FT6X36U_TOUCH_DATA_MSB_MASK) << FT6X36U_TOUCH_DATA_MSB_SHIFT) | (((uint16_t)(buffer[4]) & FT6X36U_TOUCH_DATA_LSB_MASK) << FT6X36U_TOUCH_DATA_LSB_SHIFT);
	// Determine whether the touch point y coordinates is in the buttons area
	if (touch_y < BUTTON_BORDER_TOP || touch_y > BUTTON_BORDER_BOTTOM) {
		// y coordinates is not in the buttons area, no button is pressed
		button_index = BUTTON_INDEX_NONE;
	} else {
		// y coordinates is in the buttons area, get the touch point x coordinates
		uint16_t touch_x = (((uint16_t)(buffer[1]) & FT6X36U_TOUCH_DATA_MSB_MASK) << FT6X36U_TOUCH_DATA_MSB_SHIFT) | (((uint16_t)(buffer[2]) & FT6X36U_TOUCH_DATA_LSB_MASK) << FT6X36U_TOUCH_DATA_LSB_SHIFT);
		// Determine whether the touch point x coordinates is in the buttons area, and set the button index
		#define IN_RANGE(x, a, b) ((x) >= (a) && (x) <= (b))
		button_index = 
			(IN_RANGE(touch_x, BUTTON_LEFT_BORDER_LEFT, BUTTON_LEFT_BORDER_RIGHT) ? BUTTON_INDEX_LEFT : 
			(IN_RANGE(touch_x, BUTTON_MIDDLE_BORDER_LEFT, BUTTON_MIDDLE_BORDER_RIGHT) ? BUTTON_INDEX_MIDDLE : 
			(IN_RANGE(touch_x, BUTTON_RIGHT_BORDER_LEFT, BUTTON_RIGHT_BORDER_RIGHT) ? BUTTON_INDEX_RIGHT : 
			BUTTON_INDEX_NONE)));
	}

	// If the touch state is pressed, and the last touch state is also pressed, continue process
	if (touch_state == TOUCH_STATE_PRESSED && last_touch_state == TOUCH_STATE_PRESSED) {
		// If the touch point is in the same button area as the last touch point, return directly
		if (button_index == BUTTON_INDEX_NONE || button_index != last_button_index) {
			return ESP_OK;
		}

		// If press time is less than long press time, return directly
		if (xTaskGetTickCount() - last_press_time < m_long_press_time) {
			return ESP_OK;
		}

		// If press time is greater than long press time, long press occurs, continue process
		// Don't update last touch state to released here, prevent an unexpected new press event occurs until the button is really released
		// Just update the last button index to prevent long press event occurs repeatedly
		last_button_index = BUTTON_INDEX_NONE;

		// Send long press message to message process task
		BluethroatMsg_t message;
		message.type = BLUETHROAT_MSG_TYPE_BUTTON;
		message.button_data.index = button_index;
		message.button_data.act = BUTTON_ACT_LONG_PRESSED;
		if (this->m_queue_handle != NULL) {
			FT6X36U_TOUCH_LOGD("Send button message to message process task, button index: %d, button act: %d", message.button_data.index, message.button_data.act);
			xQueueSend(m_queue_handle, &message, 0);
		} else {
			// Queue handle is invalid, Add direct processing code here or do nothing
		}

		return ESP_OK;

	// If the touch state is pressed, and the last touch state is released, continue process
	} else if (touch_state == TOUCH_STATE_PRESSED && last_touch_state == TOUCH_STATE_RELEASED) {
		// Update last touch state to pressed
		last_touch_state = TOUCH_STATE_PRESSED;

		// If the touch point is not in the buttons area, return directly
		if (button_index == BUTTON_INDEX_NONE) {
			last_button_index = BUTTON_INDEX_NONE;
			return ESP_OK;
		}

		// If the touch point is in the buttons area, record current state
		last_button_index = button_index;
		last_press_time = xTaskGetTickCount();

		return ESP_OK;

	// If the touch state is released, and the last touch state is pressed, continue process
	} else {
		// Update last touch state to released
		last_touch_state = TOUCH_STATE_RELEASED;

		// If the touch point is not in the same button area as the last touch point, return directly
		if (button_index == BUTTON_INDEX_NONE || button_index != last_button_index) {
			return ESP_OK;
		}

		// Same button pressed and released, send short press message to message process task
		BluethroatMsg_t message;
		message.type = BLUETHROAT_MSG_TYPE_BUTTON;
		message.button_data.index = button_index;
		message.button_data.act = BUTTON_ACT_PRESSED;
		if (this->m_queue_handle != NULL) {
			FT6X36U_TOUCH_LOGD("Send button message to Bluethroat task, button index: %d, button act: %d", message.button_data.index, message.button_data.act);
			xQueueSend(m_queue_handle, &message, 0);
		} else {
			// Queue handle is invalid, Add direct processing code here
		}

		return ESP_OK;
	}
}
