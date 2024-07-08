#pragma once

#include <stdint.h>

typedef enum {
    BLUETHROAT_MSG_TYPE_BUTTON_DATA = 0,
    BLUETHROAT_MSG_TYPE_RTC_DATA,
    BLUETHROAT_MSG_TYPE_BAROMETER_DATA,
    BLUETHROAT_MSG_TYPE_ANEMOMETER_DATA,
    BLUETHROAT_MSG_TYPE_HYGROMETER_DATA,
    BLUETHROAT_MSG_TYPE_ACCELERATION_DATA,
    BLUETHROAT_MSG_TYPE_ROTATION_DATA,
    BLUETHROAT_MSG_TYPE_GEOMAGNATIC_DATA,
    BLUETHROAT_MSG_TYPE_POWER_DATA,
    BLUETHROAT_MSG_TYPE_GNSS_ZDA_DATA,
    BLUETHROAT_MSG_TYPE_GNSS_RMC_DATA,
    BLUETHROAT_MSG_TYPE_GNSS_GGA_DATA,
    BLUETHROAT_MSG_TYPE_GNSS_VTG_DATA,
    BLUETHROAT_MSG_TYPE_BLUETOOTH_STATE,
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
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} __attribute__ ((packed)) GnssZdaData_t;

#define GNSS_LATITUDE_DIRECTION_NORTH           (0)
#define GNSS_LATITUDE_DIRECTION_SOUTH           (1)
#define GNSS_LONGITUDE_DIRECTION_EAST           (0)
#define GNSS_LONGITUDE_DIRECTION_WEST           (1)

typedef struct {
    uint8_t latitude_degree        :7;
    uint8_t                        :1;
    uint8_t latitude_minute        :6;
    uint8_t latitude_direction     :1;
    uint8_t                        :1;
    uint8_t langitude_degree       :8;
    uint8_t langitude_minute       :6;
    uint8_t langitude_direction    :1;
    uint8_t                        :1;
    float latitude_second;
    float langitude_second;
    float course;
} __attribute__ ((packed)) GnssRmcData_t;

typedef struct {
    uint8_t latitude_degree        :7;
    uint8_t                        :1;
    uint8_t latitude_minute        :6;
    uint8_t latitude_direction     :1;
    uint8_t                        :1;
    uint8_t langitude_degree       :8;
    uint8_t langitude_minute       :6;
    uint8_t langitude_direction    :1;
    uint8_t                        :1;
    float latitude_second;
    float langitude_second;
    float altitude;
} __attribute__ ((packed)) GnssGgaData_t;

typedef struct {
    float course;
    float speed_knot;
    float speed_kmh;
} __attribute__ ((packed)) GnssVtgData_t;

typedef enum {
    SERVICE_STATE_DISCONNECTED,
    SERVICE_STATE_CONNECTED,
} ServiceState_t;

typedef struct {
    ServiceState_t environment_service_state;
    ServiceState_t nordic_uart_service_state;
} __attribute__ ((packed)) BluetoothState_t;

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
        GnssZdaData_t gnss_zda_data;
        GnssRmcData_t gnss_rmc_data;
        GnssGgaData_t gnss_gga_data;
        GnssVtgData_t gnss_vtg_data;
        BluetoothState_t bluetooth_state;
    };
} BluethroatMsg_t;

