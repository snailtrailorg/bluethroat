#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "bluethroat_msg_proc.h"
#include "drivers/task_param.h"
#include "drivers/i2c_device.h"

class Bm8563Rtc : public I2cDevice {
public:
    Bm8563Rtc(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    ~Bm8563Rtc();

public:
    esp_err_t GetRtcTime(struct tm *stm_time);
    esp_err_t SetRtcTime(struct tm *stm_time);
    static esp_err_t SetSysTime(struct tm *stm_time);

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);

private:
    static inline uint8_t bcd_to_uint8(uint8_t bcd); 
    static inline uint16_t calc_year_day(uint16_t year, uint8_t month, uint8_t day);
};
