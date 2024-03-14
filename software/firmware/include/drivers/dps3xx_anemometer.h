#include "drivers/dps3xx_barometer.h"

class Dps3xxAnemometer : public Dps3xxBarometer {
public:
    Dps3xxBarometer *m_p_barometer;

public:
    Dps3xxAnemometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle, Dps3xxBarometer *p_barometer);
    ~Dps3xxAnemometer();

public:
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
};