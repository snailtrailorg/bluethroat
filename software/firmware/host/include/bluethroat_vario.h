#include <stdint.h>

class BluethraotVario {
private:
    uint16_t m_latitude_degree;
    uint16_t m_latitude_minute;
    float m_latitude_second;
    uint16_t m_longitude_degree;
    uint16_t m_longitude_minute;
    float m_longitude_second;
    uint16_t m_altitude;

    float m_last_temperature;
    float m_last_pressure;
    uint32_t m_last_timestamp;

public:
    BluethraotVario();
    ~BluethraotVario();

public:
    float CalculateVerticalSpeed(float temperature, float pressure, uint32_t timestamp);
};

extern BluethraotVario *g_pBluethraotVario;

float CalculateVerticalSpeed(float temperature, float pressure, uint32_t timestamp);
