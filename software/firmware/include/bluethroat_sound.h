#include <stdint.h>

#include "drivers/i2s_master.h"
#include "drivers/task_object.h"

#define ACCEL_TONE_FREQ             1000
#define ACCEL_BEEP_PERIOD           1000
#define SPEED_TONE_FREQ_BASE        1000

class BluethroatSound : public TaskObject {
public:
    I2sMaster *m_i2s_master;

    static uint8_t m_sin_table[1024];

    uint32_t m_sample_rate;
    uint32_t m_sample_bits;

    uint8_t m_volume;
    bool m_speaker_enabled;

public:
    BluethroatSound(I2sMaster *i2s_master, uint32_t sample_rate, uint32_t sample_bits);
    ~BluethroatSound();

public:
    esp_err_t init_device();
    esp_err_t deinit_device();
    void task_cpp_entry();
};

extern BluethroatSound *g_pBluethroatSound;

void SetVolume(uint8_t volume);
void SetVerticalAccelTone(float vertical_accel);
void SetVerticalSpeedTone(float vertical_speed);
