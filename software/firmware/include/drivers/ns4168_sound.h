#include <stdint.h>

#include "drivers/i2s_master.h"
#include "drivers/task_object.h"

#define NS4168_SOUND_BUFFER_SIZE        (2048)

#define ACCEL_TONE_FREQ_HZ              (300)
#define ACCEL_TONE_PERIOD_MS            (1000)

#define SPEED_LIFT_TONE_FREQ_HZ_BASE    (450)
#define SPEED_LIFT_TONE_FREQ_HZ_STEP    (50)
#define SPEED_LIFT_BEEP_PERIOD_MS_BASE  (1050)
#define SPEED_LIFT_BEEP_PERIOD_MS_STEP  (-50)

#define SPEED_SINK_TONE_FREQ_HZ_BASE    (750)
#define SPEED_SINK_TONE_FREQ_HZ_STEP    (50)

#define GGRADIENT_PERIOD_MS             (5)

class Ns4168Sound : public TaskObject {
public:
    I2sMaster *m_p_i2s_master;

    static int8_t m_sin_table[256];

    uint32_t m_sample_rate;
    uint32_t m_sample_bits;

    int32_t m_volume;
    bool m_speaker_enabled;

    int32_t m_vertical_accel;
    int32_t m_vertical_speed;

public:
    Ns4168Sound(I2sMaster *i2s_master, uint32_t sample_rate, uint32_t sample_bits);
    ~Ns4168Sound();

public:
    esp_err_t init_device();
    esp_err_t deinit_device();
    void task_cpp_entry();
};

extern Ns4168Sound *g_pNs4168Sound;

void SoundSetVolume(uint8_t volume);
void SoundSetVerticalAccel(float vertical_accel);
void SoundSetVerticalSpeed(float vertical_speed);
