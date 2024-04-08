#pragma once

#include "drivers/task_message.h"
#include "drivers/task_param.h"
#include "drivers/i2c_device.h"

class Ft6x36uTouch : public I2cDevice {
public:
    Ft6x36uTouch(I2cMaster *p_i2c_master, uint16_t device_addr, const gpio_num_t *p_int_pins);
    ~Ft6x36uTouch();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
};

extern  Ft6x36uTouch *g_p_ft6x36u_touch;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t ft6x36u_i2c_read(i2c_port_t port, uint16_t addr, uint32_t reg, uint8_t *buffer, uint16_t size);
#ifdef __cplusplus
}
#endif