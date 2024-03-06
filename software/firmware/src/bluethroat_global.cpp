#include "bluethroat_global.h"

const I2cDevice_t g_I2cDeviceMap[] = {
#if CONFIG_BLUETHROAD_TARGET_DEVICE_M5STICKCPLUS
    {.port = I2C_NUM_1, .addr = 0x0051, .model = I2C_DEVICE_MODEL_BM8563_RTC},
    {.port = I2C_NUM_0, .addr = 0x0076, .model = I2C_DEVICE_MODEL_DPS3XX_BAROMETER},
    {.port = I2C_NUM_0, .addr = 0x0077, .model = I2C_DEVICE_MODEL_DPS3XX_ANEMOMETER},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORE2AWS
    {.port = I2C_NUM_1, .addr = 0x0051, .model = I2C_DEVICE_MODEL_BM8563_RTC},
    {.port = I2C_NUM_0, .addr = 0x0076, .model = I2C_DEVICE_MODEL_DPS310_BAROMETER},
    {.port = I2C_NUM_0, .addr = 0x0077, .model = I2C_DEVICE_MODEL_DPS310_ANEMOMETER},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORES3
    {.port = I2C_NUM_1, .addr = 0x0051, .model = I2C_DEVICE_MODEL_BM8563_RTC},
    {.port = I2C_NUM_0, .addr = 0x0076, .model = I2C_DEVICE_MODEL_DPS310_BAROMETER},
    {.port = I2C_NUM_0, .addr = 0x0077, .model = I2C_DEVICE_MODEL_DPS310_ANEMOMETER},
#else
    #error Invalid target device configuration, run menuconfig and reconfigure it properly
#endif 
    {.port = I2C_NUM_MAX, .addr = 0x0000, .model = I2C_DEVICE_MODEL_INVALID},
};

const TaskParam_t g_TaskParam[] = { /**/
    [TASK_ID_LVGL]               = {.task_name = "LVGL",         .task_stack_size = (4096 * 2),                  .task_priority = ((configMAX_PRIORITIES -  2) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_0,    .task_interval = (pdMS_TO_TICKS(             50))},
    [TASK_ID_MSG_PROC]           = {.task_name = "MSG_PROC",     .task_stack_size = configMINIMAL_STACK_SIZE,    .task_priority = ((configMAX_PRIORITIES -  4) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_0,    .task_interval = (pdMS_TO_TICKS(             50))},
    [TASK_ID_BM8563_RTC]         = {.task_name = "BM8563_RTC",   .task_stack_size = configMINIMAL_STACK_SIZE,    .task_priority = ((tskIDLE_PRIORITY     +  2) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_1,    .task_interval = (pdMS_TO_TICKS( 15 * 60 * 1000))},
    [TASK_ID_DPS3XX_BAROMETER]   = {.task_name = "DPS3XX_BARO",  .task_stack_size = configMINIMAL_STACK_SIZE,    .task_priority = ((configMAX_PRIORITIES -  8) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_1,    .task_interval = (pdMS_TO_TICKS(              0))},
    [TASK_ID_DPS3XX_ANEMOMETER]  = {.task_name = "DPS3XX_ANEMO", .task_stack_size = configMINIMAL_STACK_SIZE,    .task_priority = ((configMAX_PRIORITIES -  8) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_1,    .task_interval = (pdMS_TO_TICKS(              0))},
};
