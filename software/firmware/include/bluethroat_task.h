#pragma once

#include <esp_types.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

typedef enum {
    TASK_INDEX_LVGL,
    TASK_INDEX_MSG_PROC,
    TASK_INDEX_AXP192_PMU,
    TASK_INDEX_BM8563_RTC,
    TASK_INDEX_DPS3XX_BAROMETER,
    TASK_INDEX_DPS3XX_ANEMOMETER,
    TASK_INDEX_NEO_M9N_GNSS,
    TASK_INDEX_MAX,
} TaskIndex_t;

typedef enum {
    TASK_CORE_0 = 0,
    TASK_CORE_1 = 1,
    TASK_CORE_ANY = tskNO_AFFINITY,
} TaskCoreId_t;

typedef struct {
    /* task_name must not be NULL, can be "" if noname specified.
    other parameters follow the rules of freertos */
    const char *task_name;
    uint32_t task_stack_size;
    UBaseType_t task_priority;
    TaskCoreId_t task_core_id;
    TickType_t task_interval;
} TaskParam_t;
