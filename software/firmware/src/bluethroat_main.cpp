#include <esp_log.h>

#include "adapters/lvgl_adapter.h"

#include "drivers/task_param.h"
#include "drivers/i2c_master.h"
#include "drivers/bm8563_rtc.h"
#include "drivers/dps3xx_barometer.h"

#include "bluethroat_global.h"
#include "bluethroat_ui.h"
#include "bluethroat_msg_proc.h"
#include "bluethroat_clock.h"

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
    BluethroatMsgProc * pBluethroatMsgProc = new BluethroatMsgProc(&(g_TaskParam[TASK_ID_MSG_PROC]));

    /* fourth step: init i2c bus master and devices */
    BLUETHROAT_MAIN_ASSERT(I2C_NUM_MAX == 2 && CONFIG_I2C_PORT_0_ENABLED && CONFIG_I2C_PORT_1_ENABLED, "Invalid I2C configuration, run menuconfig and reconfigure it properly");
    I2cMaster *p_i2c_master[I2C_NUM_MAX] = {
        new I2cMaster(I2C_NUM_0, CONFIG_I2C_PORT_0_SDA, CONFIG_I2C_PORT_0_SCL, CONFIG_I2C_PORT_0_PULLUPS, CONFIG_I2C_PORT_0_PULLUPS, CONFIG_I2C_PORT_0_FREQ_HZ, CONFIG_I2C_PORT_0_LOCK_TIMEOUT, CONFIG_I2C_PORT_0_TIMEOUT), 
        new I2cMaster(I2C_NUM_1, CONFIG_I2C_PORT_1_SDA, CONFIG_I2C_PORT_1_SCL, CONFIG_I2C_PORT_1_PULLUPS, CONFIG_I2C_PORT_1_PULLUPS, CONFIG_I2C_PORT_1_FREQ_HZ, CONFIG_I2C_PORT_1_LOCK_TIMEOUT, CONFIG_I2C_PORT_1_TIMEOUT), 
    };

    for (int i=0; g_I2cDeviceMap[i].model != I2C_DEVICE_MODEL_INVALID; i++) {
        if (ESP_OK == p_i2c_master[g_I2cDeviceMap[i].port]->ProbeDevice(g_I2cDeviceMap[i].addr)) {
            switch (g_I2cDeviceMap[i].model) {
            case I2C_DEVICE_MODEL_BM8563_RTC:
                if (Bm8563Rtc::CheckDeviceId(p_i2c_master[g_I2cDeviceMap[i].port], g_I2cDeviceMap[i].addr) == ESP_OK) {
                    Bm8563Rtc *p_bm8563_rtc = new Bm8563Rtc(p_i2c_master[g_I2cDeviceMap[i].port], g_I2cDeviceMap[i].addr, &(g_TaskParam[TASK_ID_BM8563_RTC]), pBluethroatMsgProc->m_queue_handle);
                    p_bm8563_rtc->Start();
                }
                break;
            case I2C_DEVICE_MODEL_DPS3XX_BAROMETER:
                if (Dps3xxBarometer::CheckDeviceId(p_i2c_master[g_I2cDeviceMap[i].port], g_I2cDeviceMap[i].addr) == ESP_OK) {
                    Dps3xxBarometer *p_dps3xx_barometer = new Dps3xxBarometer(p_i2c_master[g_I2cDeviceMap[i].port], g_I2cDeviceMap[i].addr, &(g_TaskParam[TASK_ID_DPS3XX_BAROMETER]), pBluethroatMsgProc->m_queue_handle);
                    p_dps3xx_barometer->Start();
                }
                break;
            case I2C_DEVICE_MODEL_DPS3XX_ANEMOMETER:
                break;
            default:
                BLUETHROAT_MAIN_LOGE("Invalid device model %d", g_I2cDeviceMap[i].model);
                break;
            }
        }
    }

    //bluethroat_clock_init();
}
