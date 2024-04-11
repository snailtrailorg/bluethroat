#pragma once

#include "drivers/task_message.h"
#include "drivers/task_param.h"
#include "drivers/i2c_device.h"

#define FT6X36U_REG_ADDR_TD_STATUS      (0x02)
#define FT6X36U_REG_ADDR_TOUCH1_XH      (0x03)
#define FT6X36U_REG_ADDR_TOUCH1_XL      (0x04)
#define FT6X36U_REG_ADDR_TOUCH1_YH      (0x05)
#define FT6X36U_REG_ADDR_TOUCH1_YL      (0x06)

#define FT6X36U_TOUCH_DATA_LENGTH       (0x05)

#define BUTTON_LEFT_BORDER_LEFT         (5)
#define BUTTON_LEFT_BORDER_TOP          (240)
#define BUTTON_LEFT_BORDER_RIGHT        (104)
#define BUTTON_LEFT_BORDER_BOTTOM       (279)

#define BUTTON_MIDDLE_BORDER_LEFT       (110)
#define BUTTON_MIDDLE_BORDER_TOP        (240)
#define BUTTON_MIDDLE_BORDER_RIGHT      (209)
#define BUTTON_MIDDLE_BORDER_BOTTOM     (279)

#define BUTTON_RIGHT_BORDER_LEFT        (215)
#define BUTTON_RIGHT_BORDER_TOP         (240)
#define BUTTON_RIGHT_BORDER_RIGHT       (314)
#define BUTTON_RIGHT_BORDER_BOTTOM      (279)

#define BUTTON_DEFAULT_LONG_PRESS_TIME  (pdMS_TO_TICKS(3000))

typedef enum {
    TOUCH_STATE_RELEASED = 0,
    TOUCH_STATE_PRESSED,
} TouchState_t;

class Ft6x36uTouch : public I2cDevice {
public:
    TickType_t m_long_press_time;
    TouchState_t m_lsat_touch_state;
    TickType_t m_last_press_time;

public:
    Ft6x36uTouch(I2cMaster *p_i2c_master, uint16_t device_addr, const gpio_num_t *p_int_pins, const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    ~Ft6x36uTouch();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    esp_err_t read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size);
};
