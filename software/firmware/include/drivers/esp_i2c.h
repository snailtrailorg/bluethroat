#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sdkconfig.h>

// Only here to get the I2C_NUM_0 and I2C_NUM_1 defines.
#include <driver/i2c.h>

#define ESP_I2C_ADDR_10_BIT_FLAG    ((uint16_t)(0x8000))
#define ESP_I2C_REG_16_BIT_FLAG     ((uint32_t)(0x80000000))
#define ESP_I2C_NO_REG_FLAG         ((uint32_t)(0x40000000))

esp_err_t esp_i2c_init(i2c_port_t port);
esp_err_t esp_i2c_deinit(i2c_port_t port);

esp_err_t esp_i2c_read(i2c_port_t port, uint16_t addr, uint32_t reg, uint8_t * buffer, uint16_t size);
esp_err_t esp_i2c_write(i2c_port_t port, uint16_t addr, uint32_t reg, const uint8_t * buffer, uint16_t size);

#ifdef __cplusplus
}
#endif
