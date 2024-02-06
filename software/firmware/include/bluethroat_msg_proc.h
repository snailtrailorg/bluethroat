#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define BLUETHROAT_MSG_QUEUE_LENGTH     (32)

typedef enum {
    BLUETHROAT_MSG_PRESSURE = 0,
    BLUETHROAT_MSG_TEMPERATURE,
    BLUETHROAT_MSG_HUMIDITY,
    BLUETHROAT_MSG_AIR_SPEED,
    BLUETHROAT_MSG_ACCELERATION,
    BLUETHROAT_MSG_ROTATION,
    BLUETHROAT_MSG_GEOMAGNATIC,
    BLUETHROAT_MSG_POWER,
    BLUETHROAT_MSG_GPS,
} BluethroatMsgType_t;

typedef struct {

} GpsData_t;

typedef struct {

} AccelerationData_t;

typedef struct {

} RotationData_t;

typedef struct {

} GeomagneticData_t;

typedef struct {

} PowerData_t;

typedef struct {
    uint8_t bcd_hour;
    uint8_t bcd_minute;
    uint8_t bcd_second;
} ClockData_t;

typedef struct {
    BluethroatMsgType_t type;
    union {
        uint32_t pressure;
        uint32_t temperature;
        uint32_t humidity;
        uint32_t air_speed;
        AccelerationData_t acceleration;
        RotationData_t rotation;
        GeomagneticData_t geomagnatic;
        PowerData_t power;
        GpsData_t gps;
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

public:
	void message_loop();

};

extern "C" static void message_loop_c_entry(void * p_param);