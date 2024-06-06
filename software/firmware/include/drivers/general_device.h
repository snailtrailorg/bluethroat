#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "bluethroat_message.h"
#include "bluethroat_task.h"

class GeneralDevice {
public:
    const char *m_p_device_name;
    const TaskParam_t *m_p_task_param;
    TaskHandle_t m_task_handle;
    QueueHandle_t m_queue_handle;

public:
    GeneralDevice();
    ~GeneralDevice();

public:
    esp_err_t Init();
    esp_err_t Deinit();
    esp_err_t Start(const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    esp_err_t Stop();

public:
    esp_err_t create_task();
    esp_err_t delete_task();

public:
    virtual esp_err_t init_device() = 0;
    virtual esp_err_t deinit_device() = 0;
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size) = 0;
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) = 0;

public:
    void device_task();
};

extern "C" void device_task_c_entry(void *p_param);
