#include "bluethroat_global.h"

const I2cDeviceMap_t g_I2cPortAddrMap[] = {
#if CONFIG_BLUETHROAD_TARGET_DEVICE_M5STICKCPLUS
    {.port = I2C_NUM_1, .addr = 0x51, .model = I2C_DEVICE_MODEL_BM8563_RTC},
    {.port = I2C_NUM_0, .addr = 0x76, .model = I2C_DEVICE_MODEL_DPS310_BAROMETER},
    {.port = I2C_NUM_0, .addr = 0x77, .model = I2C_DEVICE_MODEL_DPS310_ANEMOMETER},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORE2AWS
    {.port = I2C_NUM_1, .addr = 0x51, .model = I2C_DEVICE_MODEL_BM8563_RTC},
    {.port = I2C_NUM_0, .addr = 0x76, .model = I2C_DEVICE_MODEL_DPS310_BAROMETER},
    {.port = I2C_NUM_0, .addr = 0x77, .model = I2C_DEVICE_MODEL_DPS310_ANEMOMETER},
#elif CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORES3
    {.port = I2C_NUM_1, .addr = 0x51, .model = I2C_DEVICE_MODEL_BM8563_RTC},
    {.port = I2C_NUM_0, .addr = 0x76, .model = I2C_DEVICE_MODEL_DPS310_BAROMETER},
    {.port = I2C_NUM_0, .addr = 0x77, .model = I2C_DEVICE_MODEL_DPS310_ANEMOMETER},
#else
    #error Invalid target device configuration, run menuconfig and reconfigure it properly
#endif
};
