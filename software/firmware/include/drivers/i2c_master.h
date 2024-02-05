#pragma once

#include <esp_err.h>
#include <freertos/semphr.h>
#include <driver/i2c.h>

#define I2C_DEFAULT_SDA_PULLUP_ENABLED 		((bool)(false))
#define I2C_DEFAULT_SCL_PULLUP_ENABLED 		((bool)(false))
#define I2C_DEFAULT_CLOCK_SPEED 			((uint32_t)(400000))

class I2cMaster {
private:
    static I2cMaster * m_instance[I2C_NUM_MAX];
    SemaphoreHandle_t m_mutex;
    i2c_port_t m_port;
    TickType_t m_lock_timeout;
    TickType_t m_timeout;

public:
    I2cMaster(i2c_port_t port, int sda_io_num, int scl_io_num, bool sda_pullup_en, bool scl_pullup_en, uint32_t clk_speed, uint16_t lock_timeout, uint16_t timeout);
    ~I2cMaster();
    esp_err_t ProbeDevice(int16_t device_addr);
    esp_err_t ReadBuffer(uint16_t device_addr, uint32_t reg_addr, uint8_t *buffer, uint16_t size);
    esp_err_t WriteBuffer(uint16_t device_addr, uint32_t reg_addr, const uint8_t *buffer, uint16_t size);
    inline esp_err_t ReadByte(uint16_t device_addr, uint32_t reg_addr, uint8_t * byte);
    inline esp_err_t WriteByte(uint16_t device_addr, uint32_t reg_addr, const uint8_t byte);

private:
    esp_err_t init_controller(i2c_port_t port, int sda_io_num, int scl_io_num, bool sda_pullup_en, bool scl_pullup_en, uint32_t clk_speed, uint16_t lock_timeout, uint16_t timeout);
    esp_err_t deinit_controller();
    inline esp_err_t lock();
    inline esp_err_t unlock();
    static inline void send_address(i2c_cmd_handle_t cmd, uint16_t device_addr, i2c_rw_t rw);
    static inline void send_register(i2c_cmd_handle_t cmd, uint32_t reg_addr);
};
