#pragma once

#include <driver/uart.h>

#include "utilities/task_object.h"
#include "bluethroat_message.h"

#define MNEA_SENTENCE_MAX_SIZE          (0x80)
#define MNEA_SENTENCE_MAX_FIELDS        (0x10)
#define UART_RECEIVE_BUFFER_SIZE        (0x400)
#define UART_BAUDRATE_CYCLE_PER_BYTE    (10)
#define UART_EVENT_QUEUE_SIZE           (0x40)
#define UART_PATTERN_QUEUE_SIZE         (0x20)
#define UART_RECEIVE_TIMEOUT            pdMS_TO_TICKS(20)

#ifdef CONFIG_GNSS_UART_PORT
    #if CONFIG_GNSS_UART_PORT == 0
        #define GNSS_UART_PORT          UART_NUM_0
    #elif CONFIG_GNSS_UART_PORT == 1
        #define GNSS_UART_PORT          UART_NUM_1
    #elif CONFIG_GNSS_UART_PORT == 2
        #define GNSS_UART_PORT          UART_NUM_2
    #elif CONFIG_GNSS_UART_PORT == 3
        #define GNSS_UART_PORT          UART_NUM_3
    #endif
#else
    #define GNSS_UART_PORT              CONFIG_GNSS_UART_PORT
#endif

class NeoM9nGnss : public TaskObject{
public:
    uart_port_t m_uart_port;
    gpio_num_t m_uart_tx_pin;
    gpio_num_t m_uart_rx_pin;
    gpio_num_t m_uart_rts_pin;
    gpio_num_t m_uart_cts_pin;
    int m_uart_baudrate;
   	QueueHandle_t m_uart_queue;

public:
    NeoM9nGnss();
    ~NeoM9nGnss();

public:
    esp_err_t Init(uart_port_t uart_port, gpio_num_t tx_pin, gpio_num_t rx_pin, gpio_num_t rts_pin, gpio_num_t cts_pin, int baudrate);
    esp_err_t Deinit();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual void task_cpp_entry();

private:
    void process_gnss_sentence(char *sentence);
    int splite_sentence(char *sentence, char *fields[], int max_fields);
};
