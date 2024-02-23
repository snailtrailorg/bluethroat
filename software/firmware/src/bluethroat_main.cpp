#include "adapters/lvgl_adapter.h"

#include "drivers/i2c_master.h"
#include "drivers/bm8563_rtc.h"
#include "drivers/dps310_barometer.h"

#include "bluethroat_global.h"
#include "bluethroat_ui.h"
#include "bluethroat_msg_proc.h"
#include  "bluethroat_clock.h"

#define BLUETHROAT_MAIN_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_MAIN_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_MAIN_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_MAIN_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define BLUETHROAT_MAIN_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define BLUETHROAT_MAIN_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define BLUETHROAT_MAIN_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define BLUETHROAT_MAIN_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define BLUETHROAT_MAIN_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define BLUETHROAT_MAIN_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define BLUETHROAT_MAIN_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			BLUETHROAT_MAIN_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define BLUETHROAT_MAIN_ASSERT(condition, format, ...)
#endif

static const char *TAG = "BLUETHROAT_MAIN";


extern "C" void app_main(void);

void app_main() {
    /* first step: init lvgl driver fiand task */
    lvgl_init();

    /* second step: init bluethroat ui elements */
    bluethroat_ui_init();

    /* third step: init main message process task */
    BluethroatMsgProc * g_pBluethroatMsgProc = new BluethroatMsgProc("MSG_PROC", configMINIMAL_STACK_SIZE, configMAX_PRIORITIES - 5, tskNO_AFFINITY, pdMS_TO_TICKS(50));

    /* fourth step: init i2c bus master and devices */
    BLUETHROAT_MAIN_ASSERT(I2C_NUM_MAX == 2 && CONFIG_I2C_PORT_0_ENABLED && CONFIG_I2C_PORT_1_ENABLED, "Invalid I2C configuration, run menuconfig and reconfigure it properly");
    I2cMaster *p_i2c_master[I2C_NUM_MAX] = {
        new I2cMaster(I2C_NUM_0, CONFIG_I2C_PORT_0_SDA, CONFIG_I2C_PORT_0_SCL, CONFIG_I2C_PORT_0_PULLUPS, CONFIG_I2C_PORT_0_PULLUPS, CONFIG_I2C_PORT_0_FREQ_HZ, CONFIG_I2C_PORT_0_LOCK_TIMEOUT, CONFIG_I2C_PORT_0_TIMEOUT), 
        new I2cMaster(I2C_NUM_1, CONFIG_I2C_PORT_1_SDA, CONFIG_I2C_PORT_1_SCL, CONFIG_I2C_PORT_1_PULLUPS, CONFIG_I2C_PORT_1_PULLUPS, CONFIG_I2C_PORT_1_FREQ_HZ, CONFIG_I2C_PORT_1_LOCK_TIMEOUT, CONFIG_I2C_PORT_1_TIMEOUT), 
    };

    //Bm8563Rtc *g_pBm8563Rtc = new Bm8563Rtc(g_pI2cMaster, 10, "BM8563_RTC", 2048, tskIDLE_PRIORITY, tskNO_AFFINITY, pdMS_TO_TICKS(1000*60*15), g_pBluethroatMsgProc->m_queue_handle);
    //g_pBm8563Rtc->Start();

    bluethroat_clock_init();
}
