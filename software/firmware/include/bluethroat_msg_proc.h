#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "drivers/task_message.h"
#include "drivers/task_param.h"

#define BLUETHROAT_MSG_QUEUE_LENGTH     (32)

class BluethroatMsgProc {
public:
    const TaskParam_t *m_p_task_param;
    TaskHandle_t m_task_handle;
    QueueHandle_t m_queue_handle;

public:
    BluethroatMsgProc(const TaskParam_t *p_task_param);
    ~BluethroatMsgProc();

public:
	void message_loop();

};

extern "C" void message_loop_c_entry(void *p_param);