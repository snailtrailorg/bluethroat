#include <stdbool.h>
#include <stdint.h>

#include <esp_err.h>
#include <esp_log.h>

#include "drivers/i2c_device.h"
#include "utilities/low_pass_filter.h"
#include "drivers/dps3xx_barometer.h"

#define DPS3XX_BARO_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define DPS3XX_BARO_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define DPS3XX_BARO_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define DPS3XX_BARO_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define DPS3XX_BARO_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define DPS3XX_BARO_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define DPS3XX_BARO_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define DPS3XX_BARO_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			DPS3XX_BARO_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define DPS3XX_BARO_ASSERT(condition, format, ...)
#endif

static const char *TAG = "DPS3XX_BARO";

Dps3xxBarometer::Dps3xxBarometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle) : 
I2cDevice(p_i2c_master, device_addr, p_task_param, queue_handle) { 
    m_p_fir_filter = new FirFilter<uint32_t, uint32_t>(FILTER_DEPTH_POWER_16, AIR_PRESSURE_DEFAULT_VALUE);
    m_pressure_cfg = {
        .mesurement_rate = DPS3XX_REG_VALUE_PM_RATE_4,
        .oversampling_rate = DPS3XX_REG_VALUE_PM_PRC_64,
        .mesurement_time = pdMS_TO_TICKS(100),
        .scale_factor = float32_t(DPS3XX_SCALE_FACTOR_PRC_64, 0),
    };
    m_temperature_cfg = {
        .mesurement_rate = DPS3XX_REG_VALUE_PM_RATE_4,
        .oversampling_rate = DPS3XX_REG_VALUE_PM_PRC_32,
        .mesurement_time = pdMS_TO_TICKS(50),
        .scale_factor = float32_t(DPS3XX_SCALE_FACTOR_PRC_32, 0),
    };
}

Dps3xxBarometer::~Dps3xxBarometer() {
    delete m_p_fir_filter;
}

esp_err_t Dps3xxBarometer::CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr) {
    Dps3xxIdReg_t id;
    if (p_i2c_master->ReadByte(device_addr, DPS3XX_REG_ADDR_ID , &(id.byte)) == ESP_OK && id.byte == DPS3XX_REG_VALUE_ID) {
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}

esp_err_t Dps3xxBarometer::init_device() {
    // Read chip status and wait for sensor and coefficient data ready
    Dps3xxMeasCfgReg_t meas_cfg = {0};
    for ( ; ; ) {
        this->read_byte(DPS3XX_REG_ADDR_MEAS_CFG, &(meas_cfg.byte));
        if (meas_cfg.sensor_rdy && meas_cfg.coef_rdy) {
            break;
        } else {
            DPS3XX_BARO_LOGD("Waiting for sensor and coefficient data ready");
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    // Read coefficient data
    if (this->get_coefs() != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to read coefficient data");
        return ESP_FAIL;
    }

    // Set pressure measurement rate and oversampling rate
    Dps3xxPrsCfgReg_t prs_cfg = {0};
    prs_cfg.pm_rate = m_pressure_cfg.mesurement_rate;
    prs_cfg.pm_prc = m_pressure_cfg.oversampling_rate;
    if (this->write_byte(DPS3XX_REG_ADDR_PRS_CFG, prs_cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set pressure measurement rate and oversampling rate");
        return ESP_FAIL;
    }

    // Set temperature measurement rate and oversampling rate
    Dps3xxTmpCfgReg_t tmp_cfg = {0};
    tmp_cfg.tmp_rate = m_temperature_cfg.mesurement_rate;
    tmp_cfg.tmp_prc = m_temperature_cfg.oversampling_rate;
    if (this->write_byte(DPS3XX_REG_ADDR_TMP_CFG, tmp_cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set temperature measurement rate and oversampling rate");
        return ESP_FAIL;
    }

    // Set pressure and temperature shift enable
    Dps3xxCfgReg_t cfg = {0};
    cfg.p_shift_en = 1;
    cfg.t_shift_en = 1;
    if (this->write_byte(DPS3XX_REG_ADDR_CFG_REG, cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set pressure and temperature shift enable");
        return ESP_FAIL;
    }

    // Set pressure and temperature measurement mode to stop (idle mode, ready for single shot measurement)
    meas_cfg = {0};
    if (this->write_byte(DPS3XX_REG_ADDR_MEAS_CFG, meas_cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set pressure and temperature measurement rate");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t Dps3xxBarometer::deinit_device() {
    return ESP_OK;
}

esp_err_t Dps3xxBarometer::fetch_data(uint8_t *data, uint8_t size) {
    DPS3XX_BARO_ASSERT(size >= sizeof(Dps3xxData_t), "Buffer size is not enough to contain pressure and temperature structure.");

    uint8_t meas_cfg;
    esp_err_t result;

    if ((result = this->write_byte(DPS3XX_REG_ADDR_MEAS_CFG, DPS3XX_REG_VALUE_MEAS_CTRL_TMP)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to write measurement configuration register");
        return ESP_FAIL;
    } else {
        vTaskDelay(this->m_temperature_cfg.mesurement_time);
    }

    for ( ; ; ) {
        if ((result = this->read_byte(DPS3XX_REG_ADDR_MEAS_CFG, &meas_cfg) != ESP_OK)) {
            DPS3XX_BARO_LOGE("Failed to read measurement configuration register");
            return ESP_FAIL;
        } else if (meas_cfg & DPS3XX_REG_VALUE_TMP_RDY) {
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    if ((result = this->write_byte(DPS3XX_REG_ADDR_MEAS_CFG, DPS3XX_REG_VALUE_MEAS_CTRL_PRS)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to write measurement configuration register");
        return ESP_FAIL;
    } else {
        vTaskDelay(this->m_pressure_cfg.mesurement_time);
    }

    for ( ; ; ) {
        if ((result = this->read_byte(DPS3XX_REG_ADDR_MEAS_CFG, &meas_cfg) != ESP_OK)) {
            DPS3XX_BARO_LOGE("Failed to read measurement configuration register");
            return ESP_FAIL;
        } else if (meas_cfg & DPS3XX_REG_VALUE_PRS_RDY) {
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    return this->read_buffer(DPS3XX_REG_ADDR_PSR_B2, data, sizeof(Dps3xxData_t));
}

esp_err_t Dps3xxBarometer::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    DPS3XX_BARO_ASSERT(in_size >= sizeof(bm8563rtc_time_regs_t), "Buffer size is not enough to contain datetime structure.");
    //bm8563rtc_time_regs_t *regs = (bm8563rtc_time_regs_t *)in_data;

    p_message->type = BLUETHROAT_MSG_BAROMETER;

    return ESP_OK;
}

esp_err_t Dps3xxBarometer::get_coefs() {
    Dps3xxCoefRegs_t coef_regs;
    if (this->read_buffer(DPS3XX_REG_ADDR_COEF, coef_regs.bytes, sizeof(Dps3xxCoefRegs_t)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to read coefficient registers");
        return ESP_FAIL;
    }

    int32_t c0, c1, c00, c10, c01, c11, c20, c21, c30;
    c0  = ((uint32_t)coef_regs.c0h  << 24) | ((uint32_t)coef_regs.c0l  << 20); c0  >>= 20;
    c1  = ((uint32_t)coef_regs.c1h  << 28) | ((uint32_t)coef_regs.c1l  << 20); c1  >>= 20;
    c00 = ((uint32_t)coef_regs.c00h << 24) | ((uint32_t)coef_regs.c00m << 16) | ((uint32_t)coef_regs.c00l << 12); c00 >>= 12;
    c10 = ((uint32_t)coef_regs.c10h << 28) | ((uint32_t)coef_regs.c10m << 20) | ((uint32_t)coef_regs.c10l << 12); c10 >>= 12;
    c01 = ((uint32_t)coef_regs.c01h << 24) | ((uint32_t)coef_regs.c01l << 16); c01 >>= 16;
    c11 = ((uint32_t)coef_regs.c11h << 24) | ((uint32_t)coef_regs.c11l << 16); c11 >>= 16;
    c20 = ((uint32_t)coef_regs.c20h << 24) | ((uint32_t)coef_regs.c20l << 16); c20 >>= 16;
    c21 = ((uint32_t)coef_regs.c21h << 24) | ((uint32_t)coef_regs.c21l << 16); c21 >>= 16;
    c30 = ((uint32_t)coef_regs.c30h << 24) | ((uint32_t)coef_regs.c30l << 16); c30 >>= 16;

    m_coef_data.scaled_c0   = float32_t(c0,  0);
    m_coef_data.scaled_c0  /= float32_t(2, 0);

    m_coef_data.scaled_c1   = float32_t(c1,  0);
    m_coef_data.scaled_c1  /= m_temperature_cfg.scale_factor;

    m_coef_data.scaled_c00  = float32_t(c00, 0);

    m_coef_data.scaled_c10  = float32_t(c10, 0);
    m_coef_data.scaled_c10 /= m_pressure_cfg.scale_factor;

    m_coef_data.scaled_c01  = float32_t(c01, 0);
    m_coef_data.scaled_c01 /= m_temperature_cfg.scale_factor;

    m_coef_data.scaled_c11  = float32_t(c11, 0);
    m_coef_data.scaled_c11 /= m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c01 /= m_temperature_cfg.scale_factor;

    m_coef_data.scaled_c20  = float32_t(c20, 0);
    m_coef_data.scaled_c20 /= m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c20 /= m_pressure_cfg.scale_factor;
    
    m_coef_data.scaled_c21  = float32_t(c21, 0);
    m_coef_data.scaled_c21 /= m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c21 /= m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c21 /= m_temperature_cfg.scale_factor;

    m_coef_data.scaled_c30  = float32_t(c30, 0);
    m_coef_data.scaled_c30 /= m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c30 /= m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c30 /= m_pressure_cfg.scale_factor;

    return ESP_OK;
}
