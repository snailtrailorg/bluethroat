#include <stdint.h>

#include "freertos/semphr.h"

#include "utilities/i2s_master.h"
#include "utilities/task_object.h"
/****************************************************************************************
 * Sound data buffer size, must be the multiple of 4.
****************************************************************************************/
#define NS4168_SOUND_BUFFER_SIZE                (4096)

/****************************************************************************************
 * Range and default volume value.
****************************************************************************************/
#define MIN_VOLUME                              (0)
#define MAX_VOLUME                              (100)
#define DEFAULT_VOLUME                          (50)

/****************************************************************************************
 * Range and default timeout disable sound.
 * If no beep is generated in this period, the speaker and i2s channels will be disabled.
****************************************************************************************/
#define MIN_DISABLE_SOUND_TIMEOUT_MS            (5000)
#define MAX_DISABLE_SOUND_TIMEOUT_MS            (60000)
#define DEFAULT_DISABLE_SOUND_TIMEOUT_MS        (60000)

/****************************************************************************************
 * Range and default timeout poweroff vario meter.
****************************************************************************************/
#define MIN_POWER_OFF_TIMEOUT_MS                (300000)
#define MAX_POWER_OFF_TIMEOUT_MS                (3600000)
#define DEFAULT_POWER_OFF_TIMEOUT_MS            (600000)

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
 * Range of tone frequency and beep period.
****************************************************************************************/
#define MIN_TONE_FREQ_HZ                        (100)
#define MAX_TONE_FREQ_HZ                        (3000)
#define MIN_BEEP_PERIOD_MS                      (100)
#define MAX_BEEP_PERIOD_MS                      (3000)

/****************************************************************************************
 * Default acceleration tone frequency and beep period.
****************************************************************************************/
#define DEFAULT_ACCELERATION_TONE_FREQ_HZ       (300)
#define DEFAULT_ACCELERATION_BEEP_PERIOD_MS     (800)
#define SPEED_ACCELERATION_DUTY_RATIO           382 / 1000
#define DEFAULT_ACCELERATION_BEEP_COUNT         (2)
#define DEFAULT_ACCELERATION_BEEP_INTERVAL_MS   (10)
#define DEFAULT_ACCELERATION_TONE_WAVEFORM      WAVEFORM_SINE
#if DEFAULT_ACCELERATION_BEEP_COUNT < 2
#error "Beep count of acceleration per period must not less more than 2."
#endif

/****************************************************************************************
 * Default lift tone frequency and beep period parameters.
 * Speed lift tone frequency must be a positive value.
 * Speed lift beep period must be a positive value.
****************************************************************************************/ 
#define DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_BASE    (600)
#define DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_STEP    (100)
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
#define DEFAULT_SPEED_LIFT_TONE_WAVEFORM        WAVEFORM_SINE

/****************************************************************************************
 * Default sink tone frequency and beep period parameters.
 * Sink tone is continuous tone, so the beep period is just a peroiod of data transfer.
 * Speed sink tone frequency must more than 100.
****************************************************************************************/
#define DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE    (200)
#define DEFAULT_SPEED_SINK_TONE_FREQ_HZ_STEP    (-100)
#define SPEED_SINK_BEEP_PERIOD_MS               (500)
#if (DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE <= 100 || DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE + DEFAULT_SPEED_SINK_TONE_FREQ_HZ_STEP * VERTICAL_SPEED_MIN <= 100)
#error "Speed sink tone frequency must more than 100."
#endif
#define DEFAULT_SPEED_SINK_TONE_WAVEFORM        WAVEFORM_SINE

/****************************************************************************************
 * Gradient amplitude when beep beginning and ending, to eliminate harmonics.
****************************************************************************************/
#define GRADIENT_PERIOD_MS                      (5)
#if (GRADIENT_PERIOD_MS < 0 || DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE * SPEED_LIFT_BEEP_DUTY_RATIO < 2 * GRADIENT_PERIOD_MS || (DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE + DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_STEP * VERTICAL_SPEED_MAX) * SPEED_LIFT_BEEP_DUTY_RATIO < 2 * GRADIENT_PERIOD_MS)
#error "Gradient period must be a positive value and must less than half of beep duty period."
#endif
#if (GRADIENT_PERIOD_MS < 0 || SPEED_SINK_BEEP_PERIOD_MS < 2 * GRADIENT_PERIOD_MS)
#error "Gradient period must be a positive value and must less than half of beep duty period."
#endif
#if (GRADIENT_PERIOD_MS < 0 || DEFAULT_ACCELERATION_BEEP_PERIOD_MS < 2 * GRADIENT_PERIOD_MS)
#error "Gradient period must be a positive value and must less than half of beep duty period."
#endif

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
    uint32_t grad_samples;
} SpeedSinkParams_t;

typedef struct {
    uint32_t tone_freq;
    uint32_t beep_period_samples;
    uint32_t sound_samples;
    uint32_t grad_samples;
} AccelParams_t;

typedef enum {
    SOUND_IDLE = 0,
    SOUND_ACCELERATION,
    SOUND_SPEED_LIFT,
    SOUND_SPEED_SINK,
} SoundState_t;

/****************************************************************************************
 * Verticle acceleration and speed range.
 * VERTICAL_ACCELERATION_MIN and VERTICAL_SPEED_MIN must be a negative value.
 * VERTICAL_ACCELERATION_MAX and VERTICAL_SPEED_MAX must be a positive value.
****************************************************************************************/
#define VERTICAL_ACCELERATION_MIN               (-10)   // m/s^2
#define VERTICAL_ACCELERATION_MAX               (10)    // m/s^2
#define VERTICAL_SPEED_MIN                      (-10)   // m/s
#define VERTICAL_SPEED_MAX                      (10)    // m/s
# if (VERTICAL_ACCELERATION_MIN >= 0) || (VERTICAL_SPEED_MIN >= 0)
#error "VERTICAL_ACCELERATION_MIN and VERTICAL_SPEED_MIN must be a negative value."
# endif
# if (VERTICAL_ACCELERATION_MAX <= 0) || (VERTICAL_SPEED_MAX <= 0)
#error "VERTICAL_ACCELERATION_MAX and VERTICAL_SPEED_MAX must be a positive value."
# endif

/****************************************************************************************
 *  * Verticle acceleration and speed multiple and latch.
****************************************************************************************/
#define VERTICAL_ACCELERATION_MULTIPLE          (10)
#define VERTICAL_SPEED_MULTIPLE                 (10)
#define DEFAULT_ACCELERATION_LATCH_IN_MULTIPLE  (1)
#define DEFAULT_SPEED_LIFT_LATCH_IN_MULTIPLE    (2)
#define DEFAULT_SPEED_SINK_LATCH_IN_MULTIPLE    (-20)

/****************************************************************************************
 * @brief NS4168 Sound class for vario meter.
****************************************************************************************/
class Ns4168Sound : public TaskObject {
public:
    /* Construction member variables */
    I2sMaster *m_p_i2s_master;
    uint32_t m_sample_rate;
    uint32_t m_sample_bits;

    /* Global settings member variables */
    int32_t m_volume;
    TickType_t m_disable_sound_timeout_ticks;
    TickType_t m_power_off_timeout_ticks;

    /* Constant configuration namespace and key strings */
    static const char *m_conf_namespace;
    static const char *m_conf_key_volume;
    static const char *m_conf_key_disable_sound_timeout_ms;
    static const char *m_conf_key_power_off_timeout_ms;
    static const char *m_conf_key_acceleration_tone_freq_hz;
    static const char *m_conf_key_acceleration_beep_period_ms;
    static const char *m_conf_key_speed_lift_tone_freq_hz_base;
    static const char *m_conf_key_speed_lift_tone_freq_hz_step;
    static const char *m_conf_key_speed_lift_beep_period_ms_base;
    static const char *m_conf_key_speed_lift_beep_period_ms_step;
    static const char *m_conf_key_speed_sink_tone_freq_hz_base;
    static const char *m_conf_key_speed_sink_tone_freq_hz_step;
    static const char *m_conf_key_acceleration_tone_waveform;
    static const char *m_conf_key_speed_lift_tone_waveform;
    static const char *m_conf_key_speed_sink_tone_waveform;
    static const char *m_conf_key_acceleration_latch_in_multiple;
    static const char *m_conf_key_speed_lift_latch_in_multiple;
    static const char *m_conf_key_speed_sink_latch_in_multiple;

    /* Static member variables */
    static int8_t m_waveform_table[WAVEFORM_MAX][WAVEFORM_TABLE_SIZE];
    static int8_t m_sound_buffer[NS4168_SOUND_BUFFER_SIZE];

    /* Runtime member variables */
    int8_t *m_p_acceleration_waveform_table;
    int8_t *m_p_speed_lift_waveform_table;
    int8_t *m_p_speed_sink_waveform_table;
    SpeedLiftParams_t m_speed_lift_params[VERTICAL_SPEED_MAX+1];
    SpeedSinkParams_t m_speed_sink_params[1-VERTICAL_SPEED_MIN];
    AccelParams_t m_acceleration_params;

    SoundState_t m_sound_state;
    SoundState_t m_last_sound_state;

    bool m_sound_enabled;
    TickType_t m_last_beep_time_ticks;

    int32_t m_vertical_accel_in_multiple;
    int32_t m_vertical_speed_in_multiple;

    int32_t m_acceleration_latch_in_multiple;
    int32_t m_speed_lift_latch_in_multiple;
    int32_t m_speed_sink_latch_in_multiple;

    /* Mutex member variables */
    SemaphoreHandle_t m_sound_mutex;

public:
    Ns4168Sound(I2sMaster *i2s_master, uint32_t sample_rate, uint32_t sample_bits);
    ~Ns4168Sound();

public:
    esp_err_t conf_get_integer(const char *key_str, int32_t *value);
    esp_err_t conf_set_integer(const char *key_str, int32_t value);

public:
    esp_err_t init_device();
    esp_err_t deinit_device();
    void task_cpp_entry();

public:
    void set_volume(int32_t volume);
    void set_disable_sound_timeout(int32_t timeout_ms);
    void set_power_off_timeout(int32_t timeout_ms);

public:
    void set_acceleration_params(int32_t tone_freq_hz, int32_t beep_period_ms);
    void set_speed_lift_params(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms_base, int32_t beep_period_ms_step);
    void set_speed_sink_params(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step);
    void set_acceleration_waveform(Waveform_t tone_waveform);
    void set_speed_lift_waveform(Waveform_t tone_waveform);
    void set_speed_sink_waveform(Waveform_t tone_waveform);
    void set_acceleration_latch_in_multiple(int32_t acceleration_latch_in_multiple);
    void set_speed_lift_latch_in_multiple(int32_t speed_lift_latch_in_multiple);
    void set_speed_sink_latch_in_multiple(int32_t speed_sink_latch_in_multiple);

public:
    void play_acceleration_sound(int32_t vertical_accel);
    void play_speed_lift_sound(int32_t vertical_speed);
    void play_speed_sink_sound(int32_t vertical_speed);
    void play_silence_sound();
};

extern Ns4168Sound *g_pNs4168Sound;

void SoundSetVolume(int32_t volume);
void SoundSetDisableSoundTimeout(int32_t timeout_ms);
void SoundSetPowerOffTimeout(int32_t timeout_ms);

void SoundSetAccelParams(int32_t tone_freq_hz, int32_t beep_period_ms);
void SoundSetSpeedLiftParams(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms_base, int32_t beep_period_ms_step);
void SoundSetSpeedSinkParams(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms);

void SoundSetAccelWaveform(Waveform_t tone_waveform);
void SoundSetSpeedLiftWaveform(Waveform_t tone_waveform);
void SoundSetSpeedSinkWaveform(Waveform_t tone_waveform);

void SoundSetVerticalAccel(float vertical_accel);
void SoundSetVerticalSpeed(float vertical_speed);
