#include "bluethroat_global.h"

const I2cDevice_t g_I2cDeviceMap[] = {
#if CONFIG_BLUETHROAD_TARGET_DEVICE_M5STICKCPLUS
    [I2C_DEVICE_INDEX_AXP192_PMU]           = {.port = I2C_NUM_0,       .addr = 0x0034,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_BM8563_RTC]           = {.port = I2C_NUM_0,       .addr = 0x0051,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_DPS3XX_BAROMETER]     = {.port = I2C_NUM_0,       .addr = 0x0076,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_DPS3XX_ANEMOMETER]    = {.port = I2C_NUM_0,       .addr = 0x0077,     .int_pins = {GPIO_NUM_NC}},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORE2AWS
    [I2C_DEVICE_INDEX_FT6X36_TOUCH]         = {.port = I2C_NUM_0,       .addr = 0x0038,     .int_pins = {GPIO_NUM_39, GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_AXP192_PMU]           = {.port = I2C_NUM_0,       .addr = 0x0034,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_BM8563_RTC]           = {.port = I2C_NUM_0,       .addr = 0x0051,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_DPS3XX_BAROMETER]     = {.port = I2C_NUM_0,       .addr = 0x0076,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_DPS3XX_ANEMOMETER]    = {.port = I2C_NUM_0,       .addr = 0x0077,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_BMP280_BAROMETER]     = {.port = I2C_NUM_0,       .addr = 0x0076,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_BMI270_ACCELEROMETER] = {.port = I2C_NUM_0,       .addr = 0x0077,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_SHT3X_HYGROMETER]     = {.port = I2C_NUM_0,       .addr = 0x0077,     .int_pins = {GPIO_NUM_NC}},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORES3
    [I2C_DEVICE_INDEX_AXP192_PMU]           = {.port = I2C_NUM_0,       .addr = 0x0034,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_BM8563_RTC]           = {.port = I2C_NUM_0,       .addr = 0x0051,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_DPS3XX_BAROMETER]     = {.port = I2C_NUM_0,       .addr = 0x0076,     .int_pins = {GPIO_NUM_NC}},
    [I2C_DEVICE_INDEX_DPS3XX_ANEMOMETER]    = {.port = I2C_NUM_0,       .addr = 0x0077,     .int_pins = {GPIO_NUM_NC}},
#else
    #error Invalid target device configuration, run menuconfig and reconfigure it properly
#endif
    [I2C_DEVICE_INDEX_MAX]                  = {.port = I2C_NUM_MAX,     .addr = 0x0000,     .int_pins = {GPIO_NUM_NC}},
};

const TaskParam_t g_TaskParam[] = {
    [TASK_INDEX_LVGL]                   = {.task_name = "LVGL",             .task_stack_size = (4096 * 2),      .task_priority = ((configMAX_PRIORITIES -  2) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_0,    .task_interval = (pdMS_TO_TICKS(             50))},
#if CONFIG_BLUETHROAD_TARGET_DEVICE_M5STICKCPLUS
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORE2AWS
    [TASK_INDEX_MSG_PROC]               = {.task_name = "MSG_PROC",         .task_stack_size = (2048 * 2),      .task_priority = ((configMAX_PRIORITIES -  4) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_0,    .task_interval = (pdMS_TO_TICKS(             50))},
    [TASK_INDEX_AXP192_PMU]             = {.task_name = "AXP192_PMU",       .task_stack_size = (2048 * 2),      .task_priority = ((configMAX_PRIORITIES +  2) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_0,    .task_interval = (pdMS_TO_TICKS(           1000))},
    [TASK_INDEX_BM8563_RTC]             = {.task_name = "BM8563_RTC",       .task_stack_size = (2048 * 2),      .task_priority = ((tskIDLE_PRIORITY     +  2) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_1,    .task_interval = (pdMS_TO_TICKS( 15 * 60 * 1000))},
    [TASK_INDEX_DPS3XX_BAROMETER]       = {.task_name = "DPS3XX_BARO",      .task_stack_size = (2048 * 2),      .task_priority = ((configMAX_PRIORITIES -  8) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_1,    .task_interval = (pdMS_TO_TICKS(              0))},
    [TASK_INDEX_DPS3XX_ANEMOMETER]      = {.task_name = "DPS3XX_ANEMO",     .task_stack_size = (2048 * 2),      .task_priority = ((configMAX_PRIORITIES -  8) | portPRIVILEGE_BIT),     .task_core_id = TASK_CORE_1,    .task_interval = (pdMS_TO_TICKS(              0))},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORES3
#else
    #error Invalid target device configuration, run menuconfig and reconfigure it properly
#endif 
    [TASK_INDEX_MAX]                    = {.task_name = NULL,               .task_stack_size = 0,               .task_priority = tskIDLE_PRIORITY,                                      .task_core_id = TASK_CORE_ANY,  .task_interval = (pdMS_TO_TICKS(              0))},
};
