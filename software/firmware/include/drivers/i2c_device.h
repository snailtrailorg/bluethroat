#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>
#include "drivers/i2c_master.h"

class I2cDevice {
private:
    I2cMaster * m_p_i2c_master;
    uint16_t m_device_addr;
    TaskHandle_t m_task_handle;
    uint32_t m_task_stack_size;
    UBaseType_t m_task_priority;
    BaseType_t m_task_core_id;

public:
    const char * m_task_name;
    TickType_t m_task_interval;
    QueueHandle_t m_queue_handle;

public:
    I2cDevice(I2cMaster * p_i2c_master, uint16_t device_addr, char * task_name, uint32_t task_stack_size, UBaseType_t task_priority, BaseType_t task_core_id, TickType_t task_interval, QueueHandle_t queue_handle);
    ~I2cDevice();

private:
    inline esp_err_t read_byte(uint16_t device_addr, uint32_t reg_addr, uint8_t * p_byte);
    inline esp_err_t write_byte(uint16_t device_addr, uint32_t reg_addr, const uint8_t byte_value);
    inline esp_err_t read_buffer(uint16_t device_addr, uint32_t reg_addr, uint8_t * buffer, uint16_t size);
    inline esp_err_t write_buffer(uint16_t device_addr, uint32_t reg_addr, const uint8_t * buffer, uint16_t size);
    inline esp_err_t create_task();
    inline esp_err_t delete_task();
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t query_data(uint8_t * data, uint8_t size);
    virtual esp_err_t calculate_data(uint8_t * in_data, uint8_t in_size, uint8_t * out_data, uint8_t ut_size);

public:
    inline void task_loop();
};

extern "C" static void i2c_device_task_func(void * p_param);
