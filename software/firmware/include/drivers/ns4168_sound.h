#include <stdint.h>

#include "freertos/semphr.h"

#include "drivers/i2s_master.h"
#include "drivers/task_object.h"
/****************************************************************************************
 * Sound data buffer size, must be the multiple of 4.
****************************************************************************************/
#define NS4168_SOUND_BUFFER_SIZE                (4096)

/****************************************************************************************
 * Default volume value.
****************************************************************************************/
#define DEFAULT_VOLUME                          (10)

/****************************************************************************************
 * Default timeout disable sound.
 * If no beep is generated in this period, the speaker and i2s channels will be disabled.
****************************************************************************************/
#define DEFAULT_DISABLE_SOUND_TIMEOUT_MS        (5000)

/****************************************************************************************
 * Default timeout poweroff vario meter.
****************************************************************************************/
#define DEFAULT_POWER_OFF_TIMEOUT_MS            (60000)

/****************************************************************************************
 * Verticle acceleration and speed range.
 * VERTICAL_ACCEL_MIN and VERTICAL_SPEED_MIN must be a negative value.
 * VERTICAL_ACCEL_MAX and VERTICAL_SPEED_MAX must be a positive value.
****************************************************************************************/
#define VERTICAL_ACCEL_MIN                      (-10)
#define VERTICAL_ACCEL_MAX                      (10)
#define VERTICAL_SPEED_MIN                      (-10)
#define VERTICAL_SPEED_MAX                      (10)
# if (VERTICAL_ACCEL_MIN >= 0) || (VERTICAL_SPEED_MIN >= 0)
#error "VERTICAL_ACCEL_MIN and VERTICAL_SPEED_MIN must be a negative value."
# endif
# if (VERTICAL_ACCEL_MAX <= 0) || (VERTICAL_SPEED_MAX <= 0)
#error "VERTICAL_ACCEL_MAX and VERTICAL_SPEED_MAX must be a positive value."
# endif

/****************************************************************************************
 * Enumeration of waveform for sound generation.
****************************************************************************************/
typedef enum {
    WAVEFORM_SINE = 0,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_MAX,
} Waveform_t;

/****************************************************************************************
 * Default acceleration tone frequency and beep period.
****************************************************************************************/
#define DEFAULT_ACCEL_TONE_FREQ_HZ              (300)
#define DEFAULT_ACCEL_BEEP_PERIOD_MS            (500)
#define DEFAULT_ACCEL_TONE_WAVEFORM             WAVEFORM_SINE

/****************************************************************************************
 * Default lift tone frequency and beep period parameters.
 * Speed lift tone frequency must be a positive value.
 * Speed lift beep period must be a positive value.
****************************************************************************************/ 
#define DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_BASE    (800)
#define DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_STEP    (100)
#define DEFAULT_SPEED_LIFT_TONE_WAVEFORM        WAVEFORM_SINE
#if (DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_STEP <= 0 || DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_BASE + DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_STEP * VERTICAL_SPEED_MAX <= 0)
#error "Speed lift tone frequency must be a positive value."
#endif
#define DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE  (500)
#define DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_STEP  (-20)
#if (DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE <= 0 || DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE + DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_STEP * VERTICAL_SPEED_MAX <= 0)
#error "Speed lift beep period must be a positive value."
#endif
/* IMPORTANT: DO NOT USE BRACKET HERE */
#define SPEED_LIFT_BEEP_DUTY_RATIO              618 / 1000
#if (2 * SPEED_LIFT_BEEP_DUTY_RATIO < 1)
#error "IMPORTANT: DO NOT USE BRACKET HERE"
#endif

/****************************************************************************************
 * Default sink tone frequency and beep period parameters.
 * Sink tone is continuous tone, so the beep period is just a peroiod of data transfer.
 * Speed sink tone frequency must more than 100.
****************************************************************************************/
#define DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE    (800)
#define DEFAULT_SPEED_SINK_TONE_FREQ_HZ_STEP    (-100)
#define DEFAULT_SPEED_SINK_TONE_WAVEFORM        WAVEFORM_SINE
#define SPEED_SINK_BEEP_PERIOD_MS               (500)
#if (DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE <= 100 || DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE + DEFAULT_SPEED_SINK_TONE_FREQ_HZ_STEP * VERTICAL_SPEED_MIN <= 100)
#error "Speed sink tone frequency must more than 100."
#endif

/****************************************************************************************
 * Gradient amplitude when beep beginning and ending, to eliminate harmonics.
****************************************************************************************/
#define DEFAULT_GGRADIENT_PERIOD_MS             (5)
#if (DEFAULT_GGRADIENT_PERIOD_MS < 0 || DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE * SPEED_LIFT_BEEP_DUTY_RATIO < 2 * DEFAULT_GGRADIENT_PERIOD_MS || (DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE + DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_STEP * VERTICAL_SPEED_MAX) * SPEED_LIFT_BEEP_DUTY_RATIO < 2 * DEFAULT_GGRADIENT_PERIOD_MS)
#error "Gradient period must be a positive value and must less than half of beep duty period."
#endif

/****************************************************************************************
 * Default waveform types for sound generation.
****************************************************************************************/
#define DEFAULT_WAVEFORM_SPEED_LIFT             WAVE_SINE
#define DEFAULT_WAVEFORM_SPEED_SINK             WAVE_SINE
#define DEFAULT_WAVEFORM_ACCEL                  WAVE_SINE

/****************************************************************************************
 * Waveform table size.
****************************************************************************************/
#define WAVEFORM_TABLE_SIZE                     (256)

/****************************************************************************************
 * Structure of sound parameters.
****************************************************************************************/
typedef struct {
    uint32_t tone_freq;
    uint32_t beep_period_samples;
    uint32_t sound_samples;
    uint32_t grad_samples;
    uint32_t grad_stop_sample_at_beginning;
    uint32_t grad_start_sample_at_end;
} SpeedLiftParams_t;

typedef struct {
    uint32_t tone_freq;
    uint32_t beep_period_samples;
} SpeedSinkParams_t;

typedef struct {
    uint32_t tone_freq;
    uint32_t beep_period_samples;
    uint32_t sound_samples;
} AccelParams_t;

/****************************************************************************************
 * @brief Axp192 PMU class
****************************************************************************************/
class Ns4168Sound : public TaskObject {
public:
    /* Construction member variables */
    I2sMaster *m_p_i2s_master;
    uint32_t m_sample_rate;
    uint32_t m_sample_bits;

    /* Configuration member variables */
    int32_t m_volume;
    TickType_t m_disable_sound_timeout_ticks;
    TickType_t m_power_off_timeout_ticks;
    int32_t m_accel_tone_freq_hz;
    int32_t m_accel_tone_waveform;
    int32_t m_accel_beep_period_ms;
    int32_t m_speed_lift_tone_freq_hz_base;
    int32_t m_speed_lift_tone_freq_hz_step;
    int32_t m_speed_lift_tone_waveform;
    int32_t m_speed_lift_beep_period_ms_base;
    int32_t m_speed_lift_beep_period_ms_step;
    int32_t m_speed_sink_tone_freq_hz_base;
    int32_t m_speed_sink_tone_freq_hz_step;
    int32_t m_speed_sink_tone_waveform;
    int32_t m_gradient_period_ms;

    /* Static member variables */
    static int8_t m_sine_wave_table[WAVEFORM_TABLE_SIZE];
    static int8_t m_square_wave_table[WAVEFORM_TABLE_SIZE];
    static int8_t m_triangle_wave_table[WAVEFORM_TABLE_SIZE];
    static int8_t m_sawtooth_wave_table[WAVEFORM_TABLE_SIZE];

    /* Runtime member variables */
    SemaphoreHandle_t m_sound_mutex;
    int8_t *m_p_speed_lift_wave_table;
    int8_t *m_p_speed_sink_wave_table;
    int8_t *m_p_accel_wave_table;
    SpeedLiftParams_t m_speed_lift_params[VERTICAL_SPEED_MAX];
    SpeedSinkParams_t m_speed_sink_params[0-VERTICAL_SPEED_MIN];
    AccelParams_t m_accel_params;

    int32_t m_vertical_accel;
    int32_t m_vertical_speed;

    bool m_sound_enabled;
    TickType_t m_last_beep_time;

public:
    Ns4168Sound(I2sMaster *i2s_master, uint32_t sample_rate, uint32_t sample_bits);
    ~Ns4168Sound();

public:
    esp_err_t init_device();
    esp_err_t deinit_device();
    void task_cpp_entry();
};

extern Ns4168Sound *g_pNs4168Sound;

#ifdef __cplusplus
extern "C" {
#endif
void SoundCalculateSpeedLiftParams(Ns4168Sound *pSound);
void SoundCalculateSpeedSinkParams(Ns4168Sound *pSound);
void SoundCalculateAccelParams(Ns4168Sound *pSound);
void SoundSetSpeedLiftWaveform(Ns4168Sound *pSound, Waveform_t waveform);
void SoundSetSpeedLiftWaveform(Ns4168Sound *pSound, Waveform_t waveform);
void SoundSetSpeedLiftWaveform(Ns4168Sound *pSound, Waveform_t waveform);
#ifdef __cplusplus
}
#endif

void SoundSetVolume(uint8_t volume);
void SoundSetVerticalAccel(float vertical_accel);
void SoundSetVerticalSpeed(float vertical_speed);
