#pragma once

#include <sdkconfig.h>
#include <driver/i2c.h>

#define I2C_DEVICE_MAX_INT_PINS     (4)

typedef enum {
    I2C_DEVICE_MODEL_FT6X36_TOUCH,
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
    gpio_num_t int_pins[I2C_DEVICE_MAX_INT_PINS];
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

#include "bluethroat_config.h"

extern BluethroatConfig *g_pBluethroatConfig;

#include "drivers/dps3xx_barometer.h"

extern Dps3xxBarometer *p_dps3xx_barometer;

#include "drivers/ft6x36u_touch.h"

extern Ft6x36uTouch *p_ft6x36u_touch;