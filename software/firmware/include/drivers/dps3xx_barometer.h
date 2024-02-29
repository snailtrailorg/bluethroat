#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "bluethroat_msg_proc.h"
#include "drivers/task_param.h"
#include "utilities/low_pass_filter.h"
#include "drivers/i2c_device.h"

#define AIR_PRESSURE_DEFAULT_VALUE  (101325)

class Dps3xxBarometer : public I2cDevice {
public:
    FirFilter<uint32_t, uint32_t> *m_p_fir_filter;

public:
    Dps3xxBarometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    ~Dps3xxBarometer();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
};
