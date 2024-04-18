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
	FT6X36U_TOUCH_LOGI("Create ft6x36u touch device.");
}

Ft6x36uTouch::~Ft6x36uTouch() {

}

esp_err_t Ft6x36uTouch::init_device() {
    return ESP_OK;
}

esp_err_t Ft6x36uTouch::deinit_device() {
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
	} else if (unlikely(result != ESP_OK)) { // If fail, return directly
		return result;
	} else {
		// Lvgl driver reading touch date duccesfully, Intercept touch data for additional processing 
		// to implement customized touch screen buttons outside the TFT screen area

		// Static variables to store the last touch state, last button index, and last press time
		static ButtonIndex_t last_button_index = BUTTON_INDEX_NONE;
		static TouchState_t last_touch_state = TOUCH_STATE_RELEASED;
		static TickType_t last_press_time = xTaskGetTickCount();

		// Get current touch state: pressed or released
		TouchState_t touch_state = (buffer[0]) ? TOUCH_STATE_PRESSED : TOUCH_STATE_RELEASED;
		// If the touch state changes, continue processing
		if (touch_state != last_touch_state) {
			// Update the last touch state
			last_touch_state = touch_state;
			// Get the touch point y coordinates
 			uint16_t touch_y = (((uint16_t)(buffer[3]) & FT6X36U_TOUCH_DATA_MSB_MASK) << FT6X36U_TOUCH_DATA_MSB_SHIFT) | (((uint16_t)(buffer[4]) & FT6X36U_TOUCH_DATA_LSB_MASK) << FT6X36U_TOUCH_DATA_LSB_SHIFT);
			// Determine whether the touch point y coordinates is in the buttons area
			if (touch_y >= BUTTON_BORDER_TOP && touch_y <= BUTTON_BORDER_BOTTOM) {
				// y coordinates is in the buttons area, get the touch point x coordinates
				uint16_t touch_x = (((uint16_t)(buffer[1]) & FT6X36U_TOUCH_DATA_MSB_MASK) << FT6X36U_TOUCH_DATA_MSB_SHIFT) | (((uint16_t)(buffer[2]) & FT6X36U_TOUCH_DATA_LSB_MASK) << FT6X36U_TOUCH_DATA_LSB_SHIFT);
				// Determine whether the touch point x coordinates is in the buttons area, and set the button index
				#define IN_RANGE(x, a, b) ((x) >= (a) && (x) <= (b))
				ButtonIndex_t button_index = 
					(IN_RANGE(touch_x, BUTTON_LEFT_BORDER_LEFT, BUTTON_LEFT_BORDER_RIGHT) ? BUTTON_INDEX_LEFT : 
					(IN_RANGE(touch_x, BUTTON_MIDDLE_BORDER_LEFT, BUTTON_MIDDLE_BORDER_RIGHT) ? BUTTON_INDEX_MIDDLE : 
					(IN_RANGE(touch_x, BUTTON_RIGHT_BORDER_LEFT, BUTTON_RIGHT_BORDER_RIGHT) ? BUTTON_INDEX_RIGHT : 
					BUTTON_INDEX_NONE)));
				// If the button index is not BUTTON_INDEX_NONE, the touch point is in buttons area, continue processing
				if (button_index != BUTTON_INDEX_NONE) {
					// If the touch state is pressed, update the last button index and last press time
					if (touch_state == TOUCH_STATE_PRESSED) {
						last_button_index = button_index;
						last_press_time = xTaskGetTickCount();
					} else { // If the touch state is released, determine whether the touch point is in the same button area as the last touch point
						if (last_button_index == button_index) {
							// Construct message data of custom button
							BluethroatMsg_t message;
							message.type = BLUETHROAT_MSG_BUTTON;
							message.button_data.index = button_index;
							// If the time interval between the current touch point and the last touch point is greater than the long press time, it is a long press
							if (xTaskGetTickCount() - last_press_time >= m_long_press_time) {
								// long press
								message.button_data.act = BUTTON_ACT_LONG_PRESSED;
							} else {
								// short press
								message.button_data.act = BUTTON_ACT_PRESSED;
							}
							// Send the message to the Bluethroat task
							if (this->m_queue_handle != NULL) {
								FT6X36U_TOUCH_LOGD("Send button message to Bluethroat task, button index: %d, button act: %d", message.button_data.index, message.button_data.act);
								xQueueSend(m_queue_handle, &message, 0);
							} else {
								// Queue handle is invalid, Add direct processing code here
							}
						} else {
							// button index changed singce last touch state change
						}
					}
				} else {
					last_button_index = BUTTON_INDEX_NONE;
				}
			} else {
				last_button_index = BUTTON_INDEX_NONE;
			}
		} else {
			// state not changed
		}

		return ESP_OK;
	}
}
