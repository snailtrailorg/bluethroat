#pragma once

#include <driver/i2c.h>

#include "utilities/task_object.h"
#include "utilities/i2c_master.h"

#define MAX_RAW_DATA_BUFFER_LENGTH		(32)

class I2cDevice : public TaskObject{
public:
    I2cMaster *m_p_i2c_master;
    uint16_t m_device_addr;
    const gpio_num_t *m_p_int_pins;

public:
    I2cDevice();
    ~I2cDevice();

public:
    static esp_err_t CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr);
    esp_err_t Init(I2cMaster *p_i2c_master, uint16_t device_addr, const gpio_num_t *p_int_pins);
    esp_err_t Deinit();

public:
    virtual void task_cpp_entry();

public:
    virtual esp_err_t init_device() = 0;
    virtual esp_err_t deinit_device() = 0;
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size) = 0;
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) = 0;

public:
    esp_err_t read_byte(uint32_t reg_addr, uint8_t *p_byte);
    esp_err_t write_byte(uint32_t reg_addr, const uint8_t byte_value);
    esp_err_t read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size);
    esp_err_t write_buffer(uint32_t reg_addr, const uint8_t *buffer, uint16_t size);
};
