#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "bluethroat_global.h"

#define BLUETHROAT_MSG_QUEUE_LENGTH     (32)

typedef enum {
    BLUETHROAT_MSG_RTC = 0,
    BLUETHROAT_MSG_BAROMETER,
    BLUETHROAT_MSG_HYGROMETER,
    BLUETHROAT_MSG_ANEMOMETER,
    BLUETHROAT_MSG_ACCELERATION,
    BLUETHROAT_MSG_ROTATION,
    BLUETHROAT_MSG_GEOMAGNATIC,
    BLUETHROAT_MSG_POWER,
    BLUETHROAT_MSG_GPS,
    // ensure to occupy 4 byte space to avoid efficiency reduction caused by misalignment
    BLUETHROAT_MSG_INVALID = 0x7fffffff,
} BluethroatMsgType_t;

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} RtcData_t;

typedef struct {
    float temperature;
    float pressure;
} BarometerData_t;

typedef struct {
    float temperature;
    float total_pressure;
    float static_pressure;
} AnemometerData_t;

typedef struct {
    float temperature;
    float humidity;
} HygrometerData_t;

typedef struct {
    float x;
    float y;
    float z;
} AccelerationData_t;

typedef struct {
    float x;
    float y;
    float z;
} RotationData_t;

typedef struct {
    float x;
    float y;
    float z;
} GeomagneticData_t;

typedef struct {
    float voltage;
    float current;
    float power;
    float energy;

} PowerData_t;

typedef struct {
    float latitude;
    float longitude;
    float altitude;
    float speed;
} GpsData_t;

typedef struct {
    BluethroatMsgType_t type;
    union {
        RtcData_t rtc_data;
        BarometerData_t barometer_data;
        AnemometerData_t anemometer_data;
        HygrometerData_t hygrometer_data;
        AccelerationData_t acceleration_data;
        RotationData_t rotation_data;
        GeomagneticData_t geomagnatic_data;
        PowerData_t power_data;
        GpsData_t gps_data;
    };
} BluethroatMsg_t;

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