#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/bm8563_rtc.h"
#include "bluethroat_config.h"
#include "bluethroat_ui.h"

#include "bluethroat_clock.h"

#define SYS_CLOCK_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define SYS_CLOCK_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define SYS_CLOCK_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define SYS_CLOCK_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define SYS_CLOCK_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define SYS_CLOCK_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define SYS_CLOCK_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define SYS_CLOCK_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define SYS_CLOCK_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define SYS_CLOCK_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define SYS_CLOCK_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			SYS_CLOCK_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define SYS_CLOCK_ASSERT(condition, format, ...)
#endif

static const char *TAG = "SYS_CLOCK";

static int32_t g_n_time_zone = TIME_ZONE_DEFAULT;

void SetTimeZone(int32_t n_time_zone) {
    g_n_time_zone = n_time_zone;
}

static void bluethroat_clock_task(void *arg);

void bluethroat_clock_init(void) {
    if (g_pBluethroatConfig->GetInteger("system", "time_zone", &g_n_time_zone) != ESP_OK) {
        SYS_CLOCK_LOGE("Get time zone failed, use default value 8.");
    }

    xTaskCreate(bluethroat_clock_task, "bluethroat_clock_task", 2048*2, NULL, 0, NULL);
}

static uint32_t counter = 660;

static void bluethroat_clock_task(void *arg) {
    (void) arg;
    while (pdTRUE) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (counter >= 660) {
            counter = 0;

            struct tm stm_time;
            if (GetRtcTime(&stm_time) == ESP_OK) {
                SYS_CLOCK_LOGD("Get RTC time ok %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", stm_time.tm_year + 1900, stm_time.tm_mon, stm_time.tm_mday, stm_time.tm_hour, stm_time.tm_min, stm_time.tm_sec);
                struct timeval stv_time = {.tv_sec = mktime(&stm_time), .tv_usec = 0};
                
                if (0 != settimeofday(&stv_time, NULL)) {
                    SYS_CLOCK_LOGE("Set system time filed!");
                } else {
                    SYS_CLOCK_LOGD("Set system time ok %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", stm_time.tm_year + 1900, stm_time.tm_mon, stm_time.tm_mday, stm_time.tm_hour, stm_time.tm_min, stm_time.tm_sec);
                }
            } else {
                SYS_CLOCK_LOGE("Get RTC time failed!");
            }
        } else {
            counter++;
        }

        time_t now = time(NULL);
        now += g_n_time_zone * 3600;
        char clock_string[16];

        if (strftime(clock_string, sizeof(clock_string), "%T", localtime(&now)) > 0) {
            UiSetClock(clock_string);
        }
    }
}
