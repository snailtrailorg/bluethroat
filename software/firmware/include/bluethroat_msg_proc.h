#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

typedef enum {
    BLUETHROAT_MSG_PRESSURE = 0,
} BluethroatMsgType_t;

typedef struct {
    BluethroatMsgType_t type;
    union {
        uint8_t  data[1];
    };
} BluethroatMsg_t;

class BluethroatMsgProc {
private:
    TaskHandle_t m_task_handle;
    uint32_t m_task_stack_size;
    UBaseType_t m_task_priority;
    BaseType_t m_task_core_id;

public:
    const char * m_task_name;
    TickType_t m_task_interval;
    QueueHandle_t m_queue_handle;

public:
    BluethroatMsgProc(char * task_name, uint32_t task_stack_size, UBaseType_t task_priority, BaseType_t task_core_id, TickType_t task_interval);
	~BluethroatMsgProc();
    inline QueueHandle_t GetMsgQueueHandle();

private:
	void create_task();

public:
	void task_loop();

};

extern "C" static void bluethroat_msg_proc_func(void * p_param);