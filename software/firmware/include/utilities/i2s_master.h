#pragma once

#include <esp_err.h>
#include <driver/i2s_std.h>

class I2sMaster {
public:
    i2s_port_t m_port;
    i2s_chan_handle_t m_read_handle;
    i2s_chan_handle_t m_write_handle;

public:
    I2sMaster(i2s_port_t port, gpio_num_t mclk_pin, gpio_num_t bclk_pin, gpio_num_t ws_pin, gpio_num_t din_pin, gpio_num_t dout_pin, uint32_t sample_rate, i2s_data_bit_width_t bit_per_sample, uint8_t channel_num);
    ~I2sMaster();

private:
    esp_err_t init_controller(i2s_port_t port, gpio_num_t mclk_pin, gpio_num_t bclk_pin, gpio_num_t ws_pin, gpio_num_t din_pin, gpio_num_t dout_pin, uint32_t sample_rate, i2s_data_bit_width_t bit_per_sample, uint8_t channel_num);
    esp_err_t deinit_controller();

public:
    esp_err_t Read(void *dest, size_t size, size_t *bytes_read, uint32_t timeout_ms) {
        return i2s_channel_read(m_read_handle, dest, size, bytes_read, timeout_ms);
    }

    esp_err_t Write(const void *src, size_t size, size_t *bytes_written, uint32_t timeout_ms) {
        return i2s_channel_write(m_write_handle, src, size, bytes_written, timeout_ms);
    }
};
