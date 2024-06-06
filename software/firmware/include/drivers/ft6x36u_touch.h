#pragma once

#include "drivers/i2c_device.h"

#define FT6X36U_REG_ADDR_TD_STATUS      (0x02)
#define FT6X36U_REG_ADDR_TOUCH1_X_MSB   (0x03)
#define FT6X36U_REG_ADDR_TOUCH1_X_LSB   (0x04)
#define FT6X36U_REG_ADDR_TOUCH1_Y_MSB   (0x05)
#define FT6X36U_REG_ADDR_TOUCH1_Y_LSB   (0x06)
#define FT6X36U_TOUCH_DATA_MSB_MASK     (0x0F)
#define FT6X36U_TOUCH_DATA_MSB_SHIFT    (8)
#define FT6X36U_TOUCH_DATA_LSB_MASK     (0xFF)
#define FT6X36U_TOUCH_DATA_LSB_SHIFT    (0)

#define FT6X36U_TOUCH_DATA_LENGTH       (0x05)

#define BUTTON_BORDER_TOP               (240)
#define BUTTON_BORDER_BOTTOM            (279)
#define BUTTON_LEFT_BORDER_LEFT         (5)
#define BUTTON_LEFT_BORDER_RIGHT        (104)
#define BUTTON_MIDDLE_BORDER_LEFT       (110)
#define BUTTON_MIDDLE_BORDER_RIGHT      (209)
#define BUTTON_RIGHT_BORDER_LEFT        (215)
#define BUTTON_RIGHT_BORDER_RIGHT       (314)

#define BUTTON_DEFAULT_LONG_PRESS_TIME  (pdMS_TO_TICKS(5000))

typedef enum {
    TOUCH_STATE_RELEASED = 0,
    TOUCH_STATE_PRESSED,
} TouchState_t;

class Ft6x36uTouch : public I2cDevice {
public:
    TickType_t m_long_press_time;

public:
    Ft6x36uTouch();
    ~Ft6x36uTouch();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
    void set_long_press_time(TickType_t long_press_time);
    esp_err_t read_buffer(uint32_t reg_addr, uint8_t *buffer, uint16_t size);
};

extern Ft6x36uTouch *g_pFt6x36uTouch;
