#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_app_desc.h>

#include "adapters/lvgl_adapter.h"

#include "drivers/bm8563_rtc.h"
#include "drivers/dps3xx_barometer.h"
#include "drivers/dps3xx_anemometer.h"
#include "drivers/ft6x36u_touch.h"
#include "drivers/axp192_pmu.h"

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
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set("GEN_DEVICE", ESP_LOG_INFO);
    esp_log_level_set("I2C_MASTER", ESP_LOG_INFO);
    esp_log_level_set("I2C_DEVICE", ESP_LOG_INFO);
    esp_log_level_set("AXP192_PMU", ESP_LOG_INFO);
    esp_log_level_set("FT6X36", ESP_LOG_INFO);
    esp_log_level_set("MSG_PROC", ESP_LOG_INFO);
    esp_log_level_set("BM8563_RTC", ESP_LOG_INFO);
    esp_log_level_set("DPS3XX_BARO", ESP_LOG_INFO);
    esp_log_level_set("DPS3XX_ANEMO", ESP_LOG_INFO);

    /* step 0: print motd */
    BLUETHROAT_MAIN_LOGI("bluethroat paragliding variometer version %s, powered by snailtrail.org", esp_app_get_description()->version);
    BLUETHROAT_MAIN_LOGI("safe and happy flying all the time, pilots!");

    /* step 1: init nvs flash configuration */
    g_pBluethroatConfig = new BluethroatConfig();

    /* step 2: init i2c bus master */
    BLUETHROAT_MAIN_ASSERT(I2C_NUM_MAX == 2 && CONFIG_I2C_PORT_0_ENABLED && CONFIG_I2C_PORT_1_ENABLED, "Invalid I2C configuration, run menuconfig and reconfigure it properly");
    I2cMaster *p_i2c_master[I2C_NUM_MAX] = {
        new I2cMaster(I2C_NUM_0, CONFIG_I2C_PORT_0_SDA, CONFIG_I2C_PORT_0_SCL, CONFIG_I2C_PORT_0_PULLUPS, CONFIG_I2C_PORT_0_PULLUPS, CONFIG_I2C_PORT_0_FREQ_HZ, CONFIG_I2C_PORT_0_LOCK_TIMEOUT, CONFIG_I2C_PORT_0_TIMEOUT), 
        new I2cMaster(I2C_NUM_1, CONFIG_I2C_PORT_1_SDA, CONFIG_I2C_PORT_1_SCL, CONFIG_I2C_PORT_1_PULLUPS, CONFIG_I2C_PORT_1_PULLUPS, CONFIG_I2C_PORT_1_FREQ_HZ, CONFIG_I2C_PORT_1_LOCK_TIMEOUT, CONFIG_I2C_PORT_1_TIMEOUT), 
    };

    /* step 3: init axp192 pmu */
    const I2cDevice_t *pid_apx192_pmu = &(g_I2cDeviceMap[I2C_DEVICE_INDEX_AXP192_PMU]);
    I2cMaster *pim_apx192_pmu = p_i2c_master[pid_apx192_pmu->port];
    Axp192Pmu *p_Axp192Pmu = NULL;
    if (pim_apx192_pmu->ProbeDevice(pid_apx192_pmu->addr) == ESP_OK && Axp192Pmu::CheckDeviceId(pim_apx192_pmu, pid_apx192_pmu->addr) == ESP_OK) {
        (p_Axp192Pmu = new Axp192Pmu())->Init(pim_apx192_pmu, pid_apx192_pmu->addr, pid_apx192_pmu->int_pins);
    }

    /* step 4: init ft6x36u touch */
    const I2cDevice_t *pid_ft6x36_touch = &(g_I2cDeviceMap[I2C_DEVICE_INDEX_FT6X36_TOUCH]);
    I2cMaster *pim_ft6x36_touch = p_i2c_master[pid_ft6x36_touch->port];
    Ft6x36uTouch *p_Ft6x36uTouch = NULL;
    if (/*pim_ft6x36_touch->ProbeDevice(pid_ft6x36_touch->addr) == ESP_OK &&*/ Ft6x36uTouch::CheckDeviceId(pim_ft6x36_touch, pid_ft6x36_touch->addr) == ESP_OK) {
        (p_Ft6x36uTouch = new Ft6x36uTouch())->Init(pim_ft6x36_touch, pid_ft6x36_touch->addr, pid_ft6x36_touch->int_pins);
    }

    /* step 5: init lvgl driver fiand task */
    lvgl_init();

    /* step 6: init bluethroat ui elements */
    //bluethroat_ui_init();

    /* step 7: init bm5836 rtc */
    const I2cDevice_t *pid_bm8563_rtc = &(g_I2cDeviceMap[I2C_DEVICE_INDEX_BM8563_RTC]);
    I2cMaster *pim_bm8563_rtc = p_i2c_master[pid_bm8563_rtc->port];
    Bm8563Rtc *p_Bm8563Rtc = NULL;
    if (pim_bm8563_rtc->ProbeDevice(pid_bm8563_rtc->addr) == ESP_OK && Bm8563Rtc::CheckDeviceId(pim_bm8563_rtc, pid_bm8563_rtc->addr) == ESP_OK) {
        (p_Bm8563Rtc = new Bm8563Rtc())->Init(pim_bm8563_rtc, pid_bm8563_rtc->addr, pid_bm8563_rtc->int_pins);
    }

    /* step 8: init dps3xx barometer */
    const I2cDevice_t *pid_dps3xx_barometer = &(g_I2cDeviceMap[I2C_DEVICE_INDEX_DPS3XX_BAROMETER]);
    I2cMaster *pim_dps3xx_barometer = p_i2c_master[pid_dps3xx_barometer->port];
    Dps3xxBarometer *p_Dps3xxBarometer = NULL;
    if (pim_dps3xx_barometer->ProbeDevice(pid_dps3xx_barometer->addr) == ESP_OK && Dps3xxBarometer::CheckDeviceId(pim_dps3xx_barometer, pid_dps3xx_barometer->addr) == ESP_OK) {
        (p_Dps3xxBarometer = new Dps3xxBarometer())->Init(pim_dps3xx_barometer, pid_dps3xx_barometer->addr, pid_dps3xx_barometer->int_pins);
    }

    /* step 9: init dps3xx anemometer */
    const I2cDevice_t *pid_dps3xx_anemometer = &(g_I2cDeviceMap[I2C_DEVICE_INDEX_DPS3XX_ANEMOMETER]);
    I2cMaster *pim_dps3xx_anemometer = p_i2c_master[pid_dps3xx_anemometer->port];
    Dps3xxAnemometer *p_Dps3xxAnemometer = NULL;
    if (pim_dps3xx_anemometer->ProbeDevice(pid_dps3xx_anemometer->addr) == ESP_OK && Dps3xxAnemometer::CheckDeviceId(pim_dps3xx_anemometer, pid_dps3xx_anemometer->addr) == ESP_OK) {
        (p_Dps3xxAnemometer = new Dps3xxAnemometer(p_Dps3xxBarometer))->Init(pim_dps3xx_anemometer, pid_dps3xx_anemometer->addr, pid_dps3xx_anemometer->int_pins);
    }

    /* step 10: init ns4168 i2s sound */

    /* step 11: init bluethroat clock */
    //bluethroat_clock_init();

    /* step 12: init gps module */
    //bluethroat_gps_init();

    /* step 13: init wifi module */
    //bluethroat_wifi_init();

    /* step 14: init bluetooth */
    //bluethroat_mqtt_init();

    /* step 16: init main message process task */
    BluethroatMsgProc *pBluethroatMsgProc = new BluethroatMsgProc(&(g_TaskParam[TASK_INDEX_MSG_PROC]));

    /* step 17: start devices loop tasks */
    if (p_Axp192Pmu != NULL) p_Axp192Pmu->Start(&(g_TaskParam[TASK_INDEX_AXP192_PMU]), pBluethroatMsgProc->m_queue_handle);
    /* ft6x36u touch needs no task, it's driven by lvgl, but it needs queue handle to send message fo button event */
    if (p_Ft6x36uTouch != NULL) p_Ft6x36uTouch->Start(NULL, pBluethroatMsgProc->m_queue_handle);
    if (p_Bm8563Rtc != NULL) p_Bm8563Rtc->Start(&(g_TaskParam[TASK_INDEX_BM8563_RTC]), pBluethroatMsgProc->m_queue_handle);
    if (p_Dps3xxBarometer != NULL) p_Dps3xxBarometer->Start(&(g_TaskParam[TASK_INDEX_DPS3XX_BAROMETER]), pBluethroatMsgProc->m_queue_handle);
    if (p_Dps3xxAnemometer != NULL) p_Dps3xxAnemometer->Start(&(g_TaskParam[TASK_INDEX_DPS3XX_ANEMOMETER]), pBluethroatMsgProc->m_queue_handle);
}
