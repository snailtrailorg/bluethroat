#pragma once

#include <stdint.h>

typedef enum {
    BLUETHROAT_MSG_TYPE_BUTTON = 0,
    BLUETHROAT_MSG_TYPE_PMU,
    BLUETHROAT_MSG_TYPE_RTC,
    BLUETHROAT_MSG_TYPE_BAROMETER,
    BLUETHROAT_MSG_TYPE_ANEMOMETER,
    BLUETHROAT_MSG_TYPE_HYGROMETER,
    BLUETHROAT_MSG_TYPE_ACCELERATION,
    BLUETHROAT_MSG_TYPE_ROTATION,
    BLUETHROAT_MSG_TYPE_GEOMAGNATIC,
    BLUETHROAT_MSG_TYPE_POWER,
    BLUETHROAT_MSG_TYPE_GPS,
    // ensure to occupy 4 byte space to avoid efficiency reduction caused by misalignment
    BLUETHROAT_MSG_INVALID = 0x7fffffff,
} BluethroatMsgType_t;

typedef enum {
    BUTTON_INDEX_LEFT = 0,
    BUTTON_INDEX_MIDDLE,
    BUTTON_INDEX_RIGHT,
    // ensure to occupy 4 byte space to avoid efficiency reduction caused by misalignment
    BUTTON_INDEX_NONE = 0x7fffffff,
} ButtonIndex_t;

typedef enum {
    BUTTON_ACT_PRESSED = 0,
    BUTTON_ACT_LONG_PRESSED,
    // ensure to occupy 4 byte space to avoid efficiency reduction caused by misalignment
    BUTTON_ACT_NONE = 0x7fffffff,
} ButtonAct_t;

typedef struct {
    ButtonIndex_t index;
    ButtonAct_t act;
} ButtonData_t;

typedef struct {
    uint16_t battery_voltage;
    int16_t battery_current;
    struct {
        uint8_t acin_present           : 1;
        uint8_t battery_charging       : 1;
        uint8_t battery_activiting     : 1;
        uint8_t charge_undercurrent    : 1;
        uint8_t                        : 4;
    };
} __attribute__ ((packed)) PmuData_t;

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} __attribute__ ((packed)) RtcData_t;

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
        ButtonData_t button_data;
        RtcData_t rtc_data;
        PmuData_t pmu_data;
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

