#pragma once

#include <sdkconfig.h>
#include <driver/i2c.h>

#ifdef CONFIG_I2C_PORT_0_ENABLED
    #ifndef CONFIG_I2C_PORT_0_PULLUPS
        #define CONFIG_I2C_PORT_0_PULLUPS false
    #endif
#endif

#ifdef CONFIG_I2C_PORT_1_ENABLED
    #ifndef CONFIG_I2C_PORT_1_PULLUPS
        #define CONFIG_I2C_PORT_1_PULLUPS false
    #endif
#endif

typedef enum {
    I2C_DEVICE_MODEL_AXP192_PMU,
    I2C_DEVICE_MODEL_BM8563_RTC,
    I2C_DEVICE_MODEL_DPS3XX_BAROMETER,
    I2C_DEVICE_MODEL_DPS3XX_ANEMOMETER,
    I2C_DEVICE_MODEL_BMP280_BAROMETER,
    I2C_DEVICE_MODEL_BMI270_ACCELEROMETER,
    I2C_DEVICE_MODEL_SHT3X_HYGROMETER,
    I2C_DEVICE_MODEL_INVALID,
} I2cDeviceModel_t;

typedef struct {
    i2c_port_t port;
    uint16_t addr;
    I2cDeviceModel_t model;
} I2cDevice_t;

extern const I2cDevice_t g_I2cDeviceMap[];

#include "drivers/task_param.h"

typedef enum {
    TASK_ID_LVGL,
    TASK_ID_MSG_PROC,
    TASK_ID_BM8563_RTC,
    TASK_ID_DPS3XX_BAROMETER,
    TASK_ID_DPS3XX_ANEMOMETER,
    TASK_ID_MAX,
} TaskIndex_t;

extern const TaskParam_t g_TaskParam[TASK_ID_MAX];
