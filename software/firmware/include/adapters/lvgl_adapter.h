#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/* Screen resolution definition, if it was defined elsewhere, use that definition.
   If not defined elsewhere, check and use the definition in sdkconfig.
   If not defined in sdkconfig, define it for M5StickC Plus as default value. */
#ifndef LV_HOR_RES_MAX
    #ifdef CONFIG_LV_HOR_RES_MAX
        #define LV_HOR_RES_MAX CONFIG_LV_HOR_RES_MAX
    #else
        #define LV_HOR_RES_MAX 240
    #endif
#endif
#ifndef LV_VER_RES_MAX
    #ifdef CONFIG_LV_VER_RES_MAX
        #define LV_VER_RES_MAX CONFIG_LV_VER_RES_MAX
    #else
        #define LV_VER_RES_MAX 135
    #endif
#endif

/* LV_TICK_PERIOD_MS is not a tick period of system or FreeRTOS, it is used just for lvgl.
   A lvgl timer will be created  with the period of LV_TICK_PERIOD_MS.
   And every timeout, the lvgl ticks will be added by LV_TICK_PERIOD_MS. */
#define LV_TICK_PERIOD_MS                   (1)

/* LVGL mutex, to ensure the serialization of LVGL tasks and user code, 
   any call to the LVGL library must obtain the mutex in advance, 
   and release it after the operation is completed */
extern SemaphoreHandle_t lvgl_token;

/* Macro definition to acquire and release the of LVGL mutex. */
#define lvgl_acquire_token() xSemaphoreTake(lvgl_token, portMAX_DELAY)
#define lvgl_release_token() xSemaphoreGive(lvgl_token)

/* LVGL initialization routine */
void lvgl_init(void);
