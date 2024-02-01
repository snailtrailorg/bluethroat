#pragma once

#include <driver/i2c.h>
#include "drivers/i2c_master.h"

class I2cDevice {
private:
    I2cMaster * m_pI2cMaster;

public:
    I2cDevice(I2cMaster * pI2cMaster);
    esp_err_t Read(uint16_t addr, uint32_t reg, uint8_t *buffer, uint16_t size);
    esp_err_t Write(uint16_t addr, uint32_t reg, const uint8_t *buffer, uint16_t size);
};