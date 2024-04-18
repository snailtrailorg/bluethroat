#include "drivers/dps3xx_barometer.h"

class Dps3xxAnemometer : public Dps3xxBarometer {
public:
    Dps3xxBarometer *m_p_barometer;

public:
    Dps3xxAnemometer(Dps3xxBarometer *p_barometer);
    ~Dps3xxAnemometer();

public:
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
};