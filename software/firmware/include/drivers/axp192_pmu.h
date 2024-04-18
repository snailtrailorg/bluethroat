#pragma once

#include "drivers/i2c_device.h"



class Axp192Pmu : public I2cDevice {
public:
    Axp192Pmu();
    ~Axp192Pmu();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
};
