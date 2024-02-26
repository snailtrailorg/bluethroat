#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>

#include "drivers/i2c_master.h"
#include "drivers/task_param.h"
#include "bluethroat_msg_proc.h"

class I2cDevice {
public:
    I2cMaster *m_p_i2c_master;
    uint16_t m_device_addr;
    const TaskParam_t *m_p_task_param;
    TaskHandle_t m_task_handle;
    QueueHandle_t m_queue_handle;

public:
    I2cDevice(I2cMaster *p_i2c_master, uint16_t device_addr);
    I2cDevice(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    ~I2cDevice();
    esp_err_t Start();
    esp_err_t Stop();

public:
    esp_err_t read_byte(uint32_t reg_addr, uint8_t *p_byte);
    esp_err_t write_byte(uint32_t reg_addr, const uint8_t byte_value);
    esp_err_t read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size);
    esp_err_t write_buffer(uint32_t reg_addr, const uint8_t *buffer, uint16_t size);

private:
    esp_err_t create_task();
    esp_err_t delete_task();
    virtual esp_err_t init_device() = 0;
    virtual esp_err_t deinit_device() = 0;
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size) = 0;
    virtual esp_err_t calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) = 0;

public:
    void task_loop();
};

extern "C" void i2c_device_task_func(void *p_param);
