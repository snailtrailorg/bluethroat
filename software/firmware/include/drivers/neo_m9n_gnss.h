#pragma once

#include <driver/uart.h>

#include "drivers/general_device.h"
#include "bluethroat_message.h"

class NeoM9nGnss : public GeneralDevice{
public:
    uart_port_t m_uart_port;
    gpio_num_t m_uart_tx_pin;
    gpio_num_t m_uart_rx_pin;
    gpio_num_t m_uart_rts_pin;
    gpio_num_t m_uart_cts_pin;
    int m_uart_baudrate;

public:
    NeoM9nGnss();
    ~NeoM9nGnss();

public:
    esp_err_t Init(uart_port_t uart_port, gpio_num_t tx_pin, gpio_num_t rx_pin, gpio_num_t rts_pin, gpio_num_t cts_pin, int baudrate);
    esp_err_t Deinit();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
};
