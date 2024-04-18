#pragma once

#include <sdkconfig.h>

#include "bluethroat_config.h"
#include "bluethroat_message.h"
#include "bluethroat_task.h"
#include "bluethroat_device.h"
#include "drivers/dps3xx_barometer.h"
#include "drivers/ft6x36u_touch.h"

extern const I2cDevice_t g_I2cDeviceMap[];
extern const TaskParam_t g_TaskParam[];
