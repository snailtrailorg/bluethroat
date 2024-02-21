#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "bluethroat_msg_proc.h"
#include "drivers/i2c_device.h"

class Dps3xxBarometer : public I2cDevice {
public:
    Dps3xxBarometer(I2cMaster *p_i2c_master, uint16_t device_addr, char *task_name, uint32_t task_stack_size, UBaseType_t task_priority, BaseType_t task_core_id, TickType_t task_interval, QueueHandle_t queue_handle);
    ~Dps3xxBarometer();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);

private:
    static inline uint8_t bcd_to_uint8(uint8_t bcd); 
    static inline uint16_t calc_year_day(uint16_t year, uint8_t month, uint8_t day);
};
