#include <esp_log.h>
#include <math.h>
#include <time.h>

#include "drivers/axp192_pmu.h"
#include "drivers/ns4168_sound.h"

#include "bluethroat_config.h"

#define NS4168_SOUND_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define NS4168_SOUND_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define NS4168_SOUND_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define NS4168_SOUND_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define NS4168_SOUND_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define NS4168_SOUND_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define NS4168_SOUND_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define NS4168_SOUND_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define NS4168_SOUND_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define NS4168_SOUND_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define NS4168_SOUND_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			NS4168_SOUND_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define NS4168_SOUND_ASSERT(condition, format, ...)
#endif

static const char *TAG = "NS4168_SOUND";

const char *Ns4168Sound::m_conf_namespace = "sound";
const char *Ns4168Sound::m_conf_key_volume = "volume";
const char *Ns4168Sound::m_conf_key_disable_sound_timeout = "disable_sound_timeout";
const char *Ns4168Sound::m_conf_key_power_off_timeout = "power_off_timeout";
const char *Ns4168Sound::m_conf_key_acceleration_tone_freq_hz = "acceleration_tone_freq";
const char *Ns4168Sound::m_conf_key_acceleration_beep_period_ms = "acceleration_beep_period";
const char *Ns4168Sound::m_conf_key_speed_lift_tone_freq_hz_base = "speed_lift_tone_freq_base";
const char *Ns4168Sound::m_conf_key_speed_lift_tone_freq_hz_step = "speed_lift_tone_freq_step";
const char *Ns4168Sound::m_conf_key_speed_lift_beep_period_ms_base = "speed_lift_beep_period_base";
const char *Ns4168Sound::m_conf_key_speed_lift_beep_period_ms_step = "speed_lift_beep_period_step";
const char *Ns4168Sound::m_conf_key_speed_sink_tone_freq_hz_base = "speed_sink_tone_freq_base";
const char *Ns4168Sound::m_conf_key_speed_sink_tone_freq_hz_step = "speed_sink_tone_freq_step";
const char *Ns4168Sound::m_conf_key_speed_sink_beep_period_ms = "speed_sink_beep_period";
const char *Ns4168Sound::m_conf_key_acceleration_tone_waveform = "acceleration_waveform";
const char *Ns4168Sound::m_conf_key_speed_lift_tone_waveform = "speed_lift_waveform";
const char *Ns4168Sound::m_conf_key_speed_sink_tone_waveform = "speed_sink_waveform";

int8_t Ns4168Sound::m_waveform_table[WAVEFORM_MAX][WAVEFORM_TABLE_SIZE] = {
    {
        /* WAVEFORM_SINE */
        (int8_t)0x00, (int8_t)0x03, (int8_t)0x06, (int8_t)0x09, (int8_t)0x0c, (int8_t)0x0f, (int8_t)0x12, (int8_t)0x15, 
        (int8_t)0x18, (int8_t)0x1c, (int8_t)0x1f, (int8_t)0x22, (int8_t)0x25, (int8_t)0x28, (int8_t)0x2b, (int8_t)0x2e, 
        (int8_t)0x30, (int8_t)0x33, (int8_t)0x36, (int8_t)0x39, (int8_t)0x3c, (int8_t)0x3f, (int8_t)0x41, (int8_t)0x44, 
        (int8_t)0x47, (int8_t)0x49, (int8_t)0x4c, (int8_t)0x4e, (int8_t)0x51, (int8_t)0x53, (int8_t)0x55, (int8_t)0x58, 
        (int8_t)0x5a, (int8_t)0x5c, (int8_t)0x5e, (int8_t)0x60, (int8_t)0x62, (int8_t)0x64, (int8_t)0x66, (int8_t)0x68, 
        (int8_t)0x6a, (int8_t)0x6c, (int8_t)0x6d, (int8_t)0x6f, (int8_t)0x70, (int8_t)0x72, (int8_t)0x73, (int8_t)0x75, 
        (int8_t)0x76, (int8_t)0x77, (int8_t)0x78, (int8_t)0x79, (int8_t)0x7a, (int8_t)0x7b, (int8_t)0x7c, (int8_t)0x7c, 
        (int8_t)0x7d, (int8_t)0x7e, (int8_t)0x7e, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, 
        (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7f, (int8_t)0x7e, (int8_t)0x7e, 
        (int8_t)0x7d, (int8_t)0x7c, (int8_t)0x7c, (int8_t)0x7b, (int8_t)0x7a, (int8_t)0x79, (int8_t)0x78, (int8_t)0x77, 
        (int8_t)0x76, (int8_t)0x75, (int8_t)0x73, (int8_t)0x72, (int8_t)0x70, (int8_t)0x6f, (int8_t)0x6d, (int8_t)0x6c, 
        (int8_t)0x6a, (int8_t)0x68, (int8_t)0x66, (int8_t)0x64, (int8_t)0x62, (int8_t)0x60, (int8_t)0x5e, (int8_t)0x5c, 
        (int8_t)0x5a, (int8_t)0x58, (int8_t)0x55, (int8_t)0x53, (int8_t)0x51, (int8_t)0x4e, (int8_t)0x4c, (int8_t)0x49, 
        (int8_t)0x47, (int8_t)0x44, (int8_t)0x41, (int8_t)0x3f, (int8_t)0x3c, (int8_t)0x39, (int8_t)0x36, (int8_t)0x33, 
        (int8_t)0x30, (int8_t)0x2e, (int8_t)0x2b, (int8_t)0x28, (int8_t)0x25, (int8_t)0x22, (int8_t)0x1f, (int8_t)0x1c, 
        (int8_t)0x18, (int8_t)0x15, (int8_t)0x12, (int8_t)0x0f, (int8_t)0x0c, (int8_t)0x09, (int8_t)0x06, (int8_t)0x03, 
        (int8_t)0x00, (int8_t)0xfd, (int8_t)0xfa, (int8_t)0xf7, (int8_t)0xf4, (int8_t)0xf1, (int8_t)0xee, (int8_t)0xeb, 
        (int8_t)0xe8, (int8_t)0xe4, (int8_t)0xe1, (int8_t)0xde, (int8_t)0xdb, (int8_t)0xd8, (int8_t)0xd5, (int8_t)0xd2, 
        (int8_t)0xd0, (int8_t)0xcd, (int8_t)0xca, (int8_t)0xc7, (int8_t)0xc4, (int8_t)0xc1, (int8_t)0xbf, (int8_t)0xbc, 
        (int8_t)0xb9, (int8_t)0xb7, (int8_t)0xb4, (int8_t)0xb2, (int8_t)0xaf, (int8_t)0xad, (int8_t)0xab, (int8_t)0xa8, 
        (int8_t)0xa6, (int8_t)0xa4, (int8_t)0xa2, (int8_t)0xa0, (int8_t)0x9e, (int8_t)0x9c, (int8_t)0x9a, (int8_t)0x98, 
        (int8_t)0x96, (int8_t)0x94, (int8_t)0x93, (int8_t)0x91, (int8_t)0x90, (int8_t)0x8e, (int8_t)0x8d, (int8_t)0x8b, 
        (int8_t)0x8a, (int8_t)0x89, (int8_t)0x88, (int8_t)0x87, (int8_t)0x86, (int8_t)0x85, (int8_t)0x84, (int8_t)0x84, 
        (int8_t)0x83, (int8_t)0x82, (int8_t)0x82, (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, 
        (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, (int8_t)0x81, (int8_t)0x82, (int8_t)0x82, 
        (int8_t)0x83, (int8_t)0x84, (int8_t)0x84, (int8_t)0x85, (int8_t)0x86, (int8_t)0x87, (int8_t)0x88, (int8_t)0x89, 
        (int8_t)0x8a, (int8_t)0x8b, (int8_t)0x8d, (int8_t)0x8e, (int8_t)0x90, (int8_t)0x91, (int8_t)0x93, (int8_t)0x94, 
        (int8_t)0x96, (int8_t)0x98, (int8_t)0x9a, (int8_t)0x9c, (int8_t)0x9e, (int8_t)0xa0, (int8_t)0xa2, (int8_t)0xa4, 
        (int8_t)0xa6, (int8_t)0xa8, (int8_t)0xab, (int8_t)0xad, (int8_t)0xaf, (int8_t)0xb2, (int8_t)0xb4, (int8_t)0xb7, 
        (int8_t)0xb9, (int8_t)0xbc, (int8_t)0xbf, (int8_t)0xc1, (int8_t)0xc4, (int8_t)0xc7, (int8_t)0xca, (int8_t)0xcd, 
        (int8_t)0xd0, (int8_t)0xd2, (int8_t)0xd5, (int8_t)0xd8, (int8_t)0xdb, (int8_t)0xde, (int8_t)0xe1, (int8_t)0xe4, 
        (int8_t)0xe8, (int8_t)0xeb, (int8_t)0xee, (int8_t)0xf1, (int8_t)0xf4, (int8_t)0xf7, (int8_t)0xfa, (int8_t)0xfd, 
    }, {
        /* WAVEFORM_SQUARE */
        (int8_t)0x00,
    }, {
        /* WAVEFORM_TRIANGLE */
        (int8_t)0x00,
    }, {
        /* WAVEFORM_SAWTOOTH */
        (int8_t)0x00,
    }
};

Ns4168Sound::Ns4168Sound(I2sMaster *i2s_master, uint32_t sample_rate, uint32_t sample_bits) {
    m_p_i2s_master = i2s_master;
    m_sample_rate = sample_rate;
    m_sample_bits = sample_bits;

    m_volume = DEFAULT_VOLUME;
    m_disable_sound_timeout_ticks = pdMS_TO_TICKS(DEFAULT_DISABLE_SOUND_TIMEOUT_MS);
    m_power_off_timeout_ticks = pdMS_TO_TICKS(DEFAULT_POWER_OFF_TIMEOUT_MS);

    m_sound_state = SOUND_IDLE;
    m_last_sound_state = SOUND_IDLE;

    m_sound_enabled = false;
    m_last_beep_time_ticks = 0;

    m_vertical_accel = 0;
    m_vertical_speed = 0;

    m_sound_mutex = xSemaphoreCreateMutex();
    NS4168_SOUND_ASSERT(m_sound_mutex != NULL, "Failed to create sound mutex");

    g_pNs4168Sound = this;
}

Ns4168Sound::~Ns4168Sound() {
    g_pNs4168Sound = NULL;
}

esp_err_t Ns4168Sound::init_device() {
    int32_t volume;
    if (g_pBluethroatConfig->GetInteger(m_conf_namespace, m_conf_key_volume, &volume) != ESP_OK) {
        volume = DEFAULT_VOLUME;
    }
    set_volume(volume);

    int32_t disable_sound_timeout_ms;
    if (g_pBluethroatConfig->GetInteger("sound", "disable_sound_timeout", &disable_sound_timeout_ms) != ESP_OK) {
        disable_sound_timeout_ms = DEFAULT_DISABLE_SOUND_TIMEOUT_MS;
    }
    set_disable_sound_timeout(pdMS_TO_TICKS(disable_sound_timeout_ms));

    int32_t power_off_timeout_ms;
    if (g_pBluethroatConfig->GetInteger("sound", "power_off_timeout", &power_off_timeout_ms) != ESP_OK) {
        power_off_timeout_ms = DEFAULT_POWER_OFF_TIMEOUT_MS;
    }
    set_power_off_timeout(pdMS_TO_TICKS(power_off_timeout_ms));

    if (g_pBluethroatConfig->GetInteger("sound", "acceleration_tone_freq", &m_acceleration_tone_freq_hz) != ESP_OK) {
        m_acceleration_tone_freq_hz = DEFAULT_ACCELERATION_TONE_FREQ_HZ;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "acceleration_tone_waveform", &m_acceleration_tone_waveform) != ESP_OK) {
        m_acceleration_tone_waveform = DEFAULT_ACCELERATION_TONE_WAVEFORM;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "acceleration_beep_period", &m_acceleration_beep_period_ms) != ESP_OK) {
        m_acceleration_beep_period_ms = DEFAULT_ACCELERATION_BEEP_PERIOD_MS;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_lift_tone_freq_base", &m_speed_lift_tone_freq_hz_base) != ESP_OK) {
        m_speed_lift_tone_freq_hz_base = DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_BASE;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_lift_tone_freq_step", &m_speed_lift_tone_freq_hz_step) != ESP_OK) {
        m_speed_lift_tone_freq_hz_step = DEFAULT_SPEED_LIFT_TONE_FREQ_HZ_STEP;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_lift_tone_waveform", &m_speed_lift_tone_waveform) != ESP_OK) {
        m_speed_lift_tone_waveform = DEFAULT_SPEED_LIFT_TONE_WAVEFORM;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_lift_beep_period_base", &m_speed_lift_beep_period_ms_base) != ESP_OK) {
        m_speed_lift_beep_period_ms_base = DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_BASE;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_lift_beep_period_step", &m_speed_lift_beep_period_ms_step) != ESP_OK) {
        m_speed_lift_beep_period_ms_step = DEFAULT_SPEED_LIFT_BEEP_PERIOD_MS_STEP;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_sink_tone_freq_base", &m_speed_sink_tone_freq_hz_base) != ESP_OK) {
        m_speed_sink_tone_freq_hz_base = DEFAULT_SPEED_SINK_TONE_FREQ_HZ_BASE;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_sink_tone_freq_step", &m_speed_sink_tone_freq_hz_step) != ESP_OK) {
        m_speed_sink_tone_freq_hz_step = DEFAULT_SPEED_SINK_TONE_FREQ_HZ_STEP;
    }

    if (g_pBluethroatConfig->GetInteger("sound", "speed_sink_tone_waveform", &m_speed_sink_tone_waveform) != ESP_OK) {
        m_speed_sink_tone_waveform = DEFAULT_SPEED_SINK_TONE_WAVEFORM;
    }

    SoundCalculateAccelParams(this);
    SoundCalculateSpeedLiftParams(this);
    SoundCalculateSpeedSinkParams(this);

    return ESP_OK;
}

esp_err_t Ns4168Sound::deinit_device() {
    return ESP_OK;
}

void SoundCalculateSpeedLiftParams(Ns4168Sound *pSound) {
    if (pSound != NULL) {
        if (xSemaphoreTake(pSound->m_sound_mutex, portMAX_DELAY) == pdTRUE) {
            pSound->m_p_speed_lift_wave_table = 
                (pSound->m_speed_lift_tone_waveform == WAVEFORM_SQUARE) ? pSound->m_square_wave_table : 
                (pSound->m_speed_lift_tone_waveform == WAVEFORM_TRIANGLE) ? pSound->m_triangle_wave_table :
                (pSound->m_speed_lift_tone_waveform == WAVEFORM_SAWTOOTH) ? pSound->m_sawtooth_wave_table :
                pSound->m_sine_wave_table;

            for (int i=0; i<VERTICAL_SPEED_MAX; i++) {
                uint32_t tone_freq = pSound->m_speed_lift_tone_freq_hz_base + pSound->m_speed_lift_tone_freq_hz_step * i;
                uint32_t beep_period = pSound->m_speed_lift_beep_period_ms_base + pSound->m_speed_lift_beep_period_ms_step * i;
                uint32_t beep_period_samples = beep_period * pSound->m_sample_rate / 1000 ;
                uint32_t sound_samples = beep_period_samples * SPEED_LIFT_BEEP_DUTY_RATIO;
                uint32_t grad_samples = pSound->m_gradient_period_ms * pSound->m_sample_rate / 1000;
                uint32_t grad_stop_sample_at_beginning = grad_samples;
                uint32_t grad_start_sample_at_end = sound_samples - grad_samples;

                pSound->m_speed_lift_params[i].tone_freq = tone_freq;
                pSound->m_speed_lift_params[i].beep_period_samples = beep_period_samples;
                pSound->m_speed_lift_params[i].sound_samples = sound_samples;
                pSound->m_speed_lift_params[i].grad_samples = grad_samples;
                pSound->m_speed_lift_params[i].grad_stop_sample_at_beginning = grad_stop_sample_at_beginning;
                pSound->m_speed_lift_params[i].grad_start_sample_at_end = grad_start_sample_at_end;
            }
            xSemaphoreGive(pSound->m_sound_mutex);
        } else {
            NS4168_SOUND_LOGE("Can not take sound mutex, failed to calculate speed lift parameters");
        }
    } else {
        NS4168_SOUND_LOGE("Invalid parameter, failed to calculate speed lift parameters");
    }
}

void SoundCalculateSpeedSinkParams(Ns4168Sound *pSound) {
    if (pSound != NULL) {
        if (xSemaphoreTake(pSound->m_sound_mutex, portMAX_DELAY) == pdTRUE) {
            pSound->m_p_speed_sink_wave_table = 
                (pSound->m_speed_sink_tone_waveform == WAVEFORM_SQUARE) ? pSound->m_square_wave_table : 
                (pSound->m_speed_sink_tone_waveform == WAVEFORM_TRIANGLE) ? pSound->m_triangle_wave_table :
                (pSound->m_speed_sink_tone_waveform == WAVEFORM_SAWTOOTH) ? pSound->m_sawtooth_wave_table :
                pSound->m_sine_wave_table;

            for (int i=0; i<(0-VERTICAL_SPEED_MIN); i++) {
                uint32_t tone_freq = pSound->m_speed_sink_tone_freq_hz_base + pSound->m_speed_sink_tone_freq_hz_step * i;
                uint32_t beep_period_samples = tone_freq * SPEED_SINK_BEEP_PERIOD_MS / 1000 * pSound->m_sample_rate / tone_freq;

                pSound->m_speed_sink_params[i].tone_freq = tone_freq;
                pSound->m_speed_sink_params[i].beep_period_samples = beep_period_samples;
            }
            xSemaphoreGive(pSound->m_sound_mutex);
        } else {
            NS4168_SOUND_LOGE("Can not take sound mutex, failed to calculate speed sink parameters");
        }
    } else {
        NS4168_SOUND_LOGE("Invalid parameter, failed to calculate speed sink parameters");
    }
}

void SoundCalculateAccelParams(Ns4168Sound *pSound) {
    if (pSound != NULL) {
        if (xSemaphoreTake(pSound->m_sound_mutex, portMAX_DELAY) == pdTRUE) {
            pSound->m_p_acceleration_wave_table = 
                (pSound->m_acceleration_tone_waveform == WAVEFORM_SQUARE) ? pSound->m_p_acceleration_wave_table :
                (pSound->m_acceleration_tone_waveform == WAVEFORM_TRIANGLE) ? pSound->m_triangle_wave_table :
                (pSound->m_acceleration_tone_waveform == WAVEFORM_SAWTOOTH) ? pSound->m_sawtooth_wave_table :
                pSound->m_sine_wave_table;

            for (int i=0; i<(0-VERTICAL_SPEED_MIN); i++) {
                uint32_t tone_freq = pSound->m_speed_sink_tone_freq_hz_base + pSound->m_speed_sink_tone_freq_hz_step * i;
                uint32_t beep_period_samples = tone_freq * SPEED_SINK_BEEP_PERIOD_MS / 1000 * pSound->m_sample_rate / tone_freq;

                pSound->m_speed_sink_params[i].tone_freq = tone_freq;
                pSound->m_speed_sink_params[i].beep_period_samples = beep_period_samples;
            }
            xSemaphoreGive(pSound->m_sound_mutex);
        } else {
            NS4168_SOUND_LOGE("Can not take sound mutex, failed to calculate acceleration parameters");
        }
    } else {
        NS4168_SOUND_LOGE("Invalid parameter, failed to calculate acceleration parameters");
    }
}

void set_volume(int32_t volume) {

}

void set_disable_sound_timeout(int32_t timeout_ms) {

}

void set_power_off_timeout(int32_t timeout_ms) {

}

void set_acceleration_params(int32_t tone_freq_hz, int32_t beep_period_ms) {

}

void set_speed_lift_params(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms_base, int32_t beep_period_ms_step) {

}

void set_speed_sink_params(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms) {

}

void set_acceleration_waveform(Waveform_t tone_waveform) {

}

void set_speed_lift_waveform(Waveform_t tone_waveform) {

}

void set_speed_sink_waveform(Waveform_t tone_waveform) {

}


void play_acceleration_sound(int32_t vertical_accel) {

}

void play_speed_lift_sound(int32_t vertical_speed) {

}

void play_speed_sink_sound(int32_t vertical_speed) {

}

void Ns4168Sound::task_cpp_entry() {
    PmuEnableSpeaker(true);
    uint8_t *buffer = (uint8_t *)malloc(NS4168_SOUND_BUFFER_SIZE);
    NS4168_SOUND_ASSERT(buffer != NULL, "Failed to allocate buffer");

    for ( ; ; ) {
/*
        static uint32_t n = 0;
        n++;
        m_vertical_speed = 10 - ((n / 5) % 21);
        NS4168_SOUND_LOGD("n: %ld, m_vertical_accel: %ld", n, m_vertical_speed);
*/
        size_t written;
        esp_err_t result;

        if (m_vertical_speed >= 1) {
            uint32_t tone_freq = m_speed_lift_tone_freq_hz_base + m_speed_lift_tone_freq_hz_step * (m_vertical_speed - 1);
            uint32_t beep_period = m_speed_lift_beep_period_ms_base + m_speed_lift_beep_period_ms_step * (m_vertical_speed - 1);
            uint32_t beep_period_samples = beep_period * m_sample_rate / 1000 ;
            uint32_t sound_samples = beep_period_samples * SPEED_LIFT_BEEP_DUTY_RATIO;
            uint32_t grad_samples = m_gradient_period_ms * m_sample_rate / 1000;
            uint32_t grad_stop_sample_at_beginning = grad_samples;
            uint32_t grad_start_sample_at_end = sound_samples - grad_samples;

            NS4168_SOUND_LOGD("tone_freq: %ld, beep_period: %ld, beep_period_samples: %ld, sound_samples: %ld, grad_samples: %ld, grad_stop_sample_at_beginning: %ld, grad_start_sample_at_end: %ld", tone_freq, beep_period, beep_period_samples, sound_samples, grad_samples, grad_stop_sample_at_beginning, grad_start_sample_at_end);

            int32_t sample;
            uint32_t buffer_offset = 0;
            for (uint32_t i = 0; i < beep_period_samples; i++) {
                if (i < sound_samples) {
                    sample = m_sin_table[(((i * tone_freq) % m_sample_rate) * WAVEFORM_TABLE_SIZE / m_sample_rate) % WAVEFORM_TABLE_SIZE];
                    sample = sample * m_volume / 100;
                    if (grad_samples != 0 && i < grad_stop_sample_at_beginning) {
                        sample = sample * m_sin_table[i * 64 / grad_samples] / 128;
                    } else if (grad_samples != 0 && i >= grad_start_sample_at_end) {
                        sample = sample * m_sin_table[(sound_samples - i) * 64 / grad_samples] / 128;
                    } else {
                        (void)0;
                    }
                } else {
                    sample = 0;
                }

                buffer[buffer_offset++] = sample;
                buffer[buffer_offset++] = sample;
                buffer[buffer_offset++] = sample;
                buffer[buffer_offset++] = sample;

                if (buffer_offset >= NS4168_SOUND_BUFFER_SIZE || i >= (beep_period_samples - 1)) {
                    //if (i >= grad_start_sample_at_end) NS4168_SOUND_BUFFER_LOGD(buffer, buffer_offset);
                    result = m_p_i2s_master->Write(buffer, buffer_offset, &written, portMAX_DELAY);
                    if (result != ESP_OK || written != buffer_offset) {
                        NS4168_SOUND_LOGE("Failed to write sound buffer: %d, %d", result, written);
                    }
                    buffer_offset = 0;
                }
            }
        } else if (m_vertical_speed <= -1) {
            uint32_t tone_freq = SPEED_SINK_TONE_FREQ_HZ_BASE + SPEED_SINK_TONE_FREQ_HZ_STEP * (m_vertical_speed + 1);
            uint32_t beep_period = SPEED_SINK_BEEP_PERIOD_MS;
            uint32_t beep_period_samples = tone_freq * beep_period / 1000 * m_sample_rate / tone_freq;

            NS4168_SOUND_LOGD("tone_freq: %ld, beep_period: %ld, beep_period_samples_old: %ld, beep_period_samples: %ld", tone_freq, beep_period, beep_period_samples_old, beep_period_samples);

            int32_t sample;
            uint32_t buffer_offset = 0;
            for (uint32_t i = 0; i < beep_period_samples; i++) {
                sample = m_sin_table[(((i * tone_freq) % m_sample_rate) * WAVEFORM_TABLE_SIZE / m_sample_rate) % WAVEFORM_TABLE_SIZE];
                sample = sample * m_volume / 100;

                buffer[buffer_offset++] = sample;
                buffer[buffer_offset++] = sample;
                buffer[buffer_offset++] = sample;
                buffer[buffer_offset++] = sample;

                if (buffer_offset >= NS4168_SOUND_BUFFER_SIZE || i >= (beep_period_samples - 1)) {
                    result = m_p_i2s_master->Write(buffer, buffer_offset, &written, portMAX_DELAY);
                    if (result != ESP_OK || written != buffer_offset) {
                        NS4168_SOUND_LOGE("Failed to write sound buffer: %d, %d", result, written);
                    }
                    buffer_offset = 0;
                }
            }
        } else {
            for (uint32_t i = 0; i < NS4168_SOUND_BUFFER_SIZE; i++) {
                buffer[i] = 0;
            }

            result = m_p_i2s_master->Write(buffer, NS4168_SOUND_BUFFER_SIZE, &written, portMAX_DELAY);
            if (result != ESP_OK || written != NS4168_SOUND_BUFFER_SIZE) {
                NS4168_SOUND_LOGE("Failed to write sound buffer: %d, %d", result, written);
            } 
/*
            vTaskDelay(pdMS_TO_TICKS(10));
*/
        }
    }

    free(buffer);
}

Ns4168Sound *g_pNs4168Sound = NULL;

void SoundSetVolume(int32_t volume) {
    NS4168_SOUND_ASSERT(volume >= MIN_VOLUME && volume <= MAX_VOLUME, "Invalid sound volume: %d%%", volume);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_volume(volume);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set volume");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_volume, volume);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set volume");
    }
}

void SoundSetDisableSoundTimeout(int32_t timeout_ms) {
    NS4168_SOUND_ASSERT(timeout_ms >= MIN_DISABLE_SOUND_TIMEOUT_MS && timeout_ms <= MAX_DISABLE_SOUND_TIMEOUT_MS, "Invalid disable sound timeout: %dms", timeout_ms);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_disable_sound_timeout(timeout_ms);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set disable sound timeout");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_disable_sound_timeout, timeout_ms);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set disable sound timeout");
    }
}

void SoundSetPowerOffTimeout(int32_t timeout_ms) {
    NS4168_SOUND_ASSERT(timeout_ms >= MIN_POWER_OFF_TIMEOUT_MS && timeout_ms <= MAX_POWER_OFF_TIMEOUT_MS, "Invalid power off timeout: %dms", timeout_ms);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_power_off_timeout(timeout_ms);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set power off timeout");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_power_off_timeout, timeout_ms);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set power off timeout");
    }
}

void SoundSetAccelParams(int32_t tone_freq_hz, int32_t beep_period_ms) {
    NS4168_SOUND_ASSERT(tone_freq_hz >= MIN_TONE_FREQ_HZ && tone_freq_hz <= MAX_TONE_FREQ_HZ, "Invalid tone frequency: %dHz", tone_freq_hz);
    NS4168_SOUND_ASSERT(beep_period_ms >= MIN_BEEP_PERIOD_MS && beep_period_ms <= MAX_BEEP_PERIOD_MS, "Invalid beep period: %dms", beep_period_ms);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_acceleration_params(tone_freq_hz, beep_period_ms);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set acceleration parameters");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_acceleration_tone_freq_hz, tone_freq_hz);
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_acceleration_beep_period_ms, beep_period_ms);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set acceleration parameters");
    }
}

void SoundSetSpeedLiftParams(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms_base, int32_t beep_period_ms_step) {
    NS4168_SOUND_ASSERT(tone_freq_hz_base >= MIN_TONE_FREQ_HZ && tone_freq_hz_base <= MAX_TONE_FREQ_HZ, "Invalid speed lift tone frequency base: %dHz", tone_freq_hz_base);
    NS4168_SOUND_ASSERT(tone_freq_hz_base + tone_freq_hz_step * VERTICAL_SPEED_MAX >= MIN_TONE_FREQ_HZ && tone_freq_hz_base + tone_freq_hz_step * VERTICAL_SPEED_MAX <= MAX_TONE_FREQ_HZ, "Invalid speed lift tone frequency step: %dHz", tone_freq_hz_step);
    NS4168_SOUND_ASSERT(beep_period_ms_base >= MIN_BEEP_PERIOD_MS && beep_period_ms_base <= MAX_BEEP_PERIOD_MS, "Invalid speed lift beep period base: %dms", beep_period_ms_base);
    NS4168_SOUND_ASSERT(beep_period_ms_base + beep_period_ms_step * VERTICAL_SPEED_MAX >= MIN_BEEP_PERIOD_MS && beep_period_ms_base + beep_period_ms_step * VERTICAL_SPEED_MAX <= MAX_BEEP_PERIOD_MS, "Invalid speed lift beep period step: %dms", beep_period_ms_step);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_speed_lift_params(tone_freq_hz_base, tone_freq_hz_step, beep_period_ms_base, beep_period_ms_step);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set speed lift parameters");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_lift_tone_freq_hz_base, tone_freq_hz_base);
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_lift_tone_freq_hz_step, tone_freq_hz_step);
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_lift_beep_period_ms_base, beep_period_ms_base);
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_lift_beep_period_ms_step, beep_period_ms_step);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set speed lift parameters");
    }
}

void SoundSetSpeedSinkParams(int32_t tone_freq_hz_base, int32_t tone_freq_hz_step, int32_t beep_period_ms) {
    NS4168_SOUND_ASSERT(tone_freq_hz_base >= MIN_TONE_FREQ_HZ && tone_freq_hz_base <= MAX_TONE_FREQ_HZ, "Invalid speed sink tone frequency base: %dHz", tone_freq_hz_base);
    NS4168_SOUND_ASSERT(tone_freq_hz_base + tone_freq_hz_step * (0-VERTICAL_SPEED_MIN) >= MIN_TONE_FREQ_HZ && tone_freq_hz_base + tone_freq_hz_step * (0-VERTICAL_SPEED_MIN) <= MAX_TONE_FREQ_HZ, "Invalid speed sink tone frequency step: %dHz", tone_freq_hz_step);
    NS4168_SOUND_ASSERT(beep_period_ms >= MIN_BEEP_PERIOD_MS && beep_period_ms <= MAX_BEEP_PERIOD_MS, "Invalid speed sink beep period: %dms", beep_period_ms);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_speed_sink_params(tone_freq_hz_base, tone_freq_hz_step, beep_period_ms);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set speed sink parameters");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_sink_tone_freq_hz_base, tone_freq_hz_base);
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_sink_tone_freq_hz_step, tone_freq_hz_step);
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_sink_beep_period_ms, beep_period_ms);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set speed sink parameters");
    }
}

void SoundSetAccelWaveform(Waveform_t tone_waveform) {
    NS4168_SOUND_ASSERT(tone_waveform >= WAVEFORM_SINE && tone_waveform < WAVEFORM_MAX, "Invalid acceleration tone waveform: %d", tone_waveform);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_acceleration_waveform(tone_waveform);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set acceleration waveform");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_acceleration_tone_waveform, tone_waveform);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set acceleration waveform");
    }
}

void SoundSetSpeedLiftWaveform(Waveform_t tone_waveform) {
    NS4168_SOUND_ASSERT(tone_waveform >= WAVEFORM_SINE && tone_waveform < WAVEFORM_MAX, "Invalid speed lift tone waveform: %d", tone_waveform);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_speed_lift_waveform(tone_waveform);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set speed lift waveform");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_lift_tone_waveform, tone_waveform);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set speed lift waveform");
    }
}

void SoundSetSpeedSinkWaveform(Waveform_t tone_waveform) {
    NS4168_SOUND_ASSERT(tone_waveform >= WAVEFORM_SINE && tone_waveform < WAVEFORM_MAX, "Invalid speed sink tone waveform: %d", tone_waveform);

    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->set_speed_sink_waveform(tone_waveform);
    } else {
        NS4168_SOUND_LOGE("Sound instance not initialized, failed to set speed sink waveform");
    }

    if (g_pBluethroatConfig != NULL) {
        g_pBluethroatConfig->SetInteger(g_pNs4168Sound->m_conf_namespace, g_pNs4168Sound->m_conf_key_speed_sink_tone_waveform, tone_waveform);
    } else {
        NS4168_SOUND_LOGE("Bluethroat config instance not initialized, failed to set speed sink waveform");
    }
}

void SoundSetVerticalAccel(float vertical_accel) {
    if (g_pNs4168Sound != NULL) {
        g_pNs4168Sound->m_vertical_accel = (int32_t)round(vertical_accel);

        //NS4168_SOUND_LOGD("Set vertical acceleration: %f, %ld", vertical_accel, g_pNs4168Sound->m_vertical_accel);
    } else {
        NS4168_SOUND_LOGD("Sound instance not initialized, failed to set vertical acceleration");
    }
}

void SoundSetVerticalSpeed(float vertical_speed) {
    if (g_pNs4168Sound != NULL) {
        int32_t n_vertical_speed = (int32_t)round(vertical_speed);
        n_vertical_speed = n_vertical_speed > 10 ? 10 : n_vertical_speed;
        n_vertical_speed = n_vertical_speed < -10 ? -10 : n_vertical_speed;
        
        g_pNs4168Sound->m_vertical_speed = n_vertical_speed;

        //NS4168_SOUND_LOGD("Set vertical speed: %f, %ld", vertical_speed, n_vertical_speed);
    } else {
        NS4168_SOUND_LOGD("Sound instance not initialized, failed to set vertical speed");
    }
}
