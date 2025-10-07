#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "bluethroat_message.h"
#include "bluethroat_task.h"

class TaskObject {
public:
    const char *m_p_object_name;
    const TaskParam_t *m_p_task_param;
    TaskHandle_t m_task_handle;
    QueueHandle_t m_queue_handle;

public:
    TaskObject();
    ~TaskObject();

public:
    esp_err_t Init();
    esp_err_t Deinit();
    esp_err_t Start(const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    esp_err_t Stop();
    void SetMessageQueue(QueueHandle_t queue_handle);

public:
    esp_err_t create_task();
    esp_err_t delete_task();

public:
    virtual esp_err_t init_device() = 0;
    virtual esp_err_t deinit_device() = 0;
    virtual void task_cpp_entry() = 0;
};

extern "C" void task_c_entry(void *p_param);
