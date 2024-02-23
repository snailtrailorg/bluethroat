#pragma once

#include <driver/i2c.h>

#include <sdkconfig.h>

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
    I2C_DEVICE_MODEL_BM8563_RTC,
    I2C_DEVICE_MODEL_DPS310_BAROMETER,
    I2C_DEVICE_MODEL_DPS310_ANEMOMETER,
} I2cDeviceModel_t;

typedef struct {
    i2c_port_t port;
    uint16_t addr;
    I2cDeviceModel_t model;
} I2cDevice_t;

extern const I2cDevice_t g_I2cDeviceMap[];
