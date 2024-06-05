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

Dps3xxBarometer::Dps3xxBarometer(char *p_instance_name) : I2cDevice(), m_p_instance_name(p_instance_name){
    DPS3XX_BARO_LOGI("Create %s device", m_p_instance_name);
    m_p_shallow_filter = new FirFilter<uint32_t, uint32_t>(FILTER_DEPTH_SHALLOW, AIR_PRESSURE_DEFAULT_VALUE << (31 - AIR_PRESSURE_DEFAULT_VALUE_MSB - FILTER_DEPTH_SHALLOW));
    m_p_deep_filter = new FirFilter<uint32_t, uint32_t>(FILTER_DEPTH_DEEP, AIR_PRESSURE_DEFAULT_VALUE << (31 - AIR_PRESSURE_DEFAULT_VALUE_MSB - FILTER_DEPTH_DEEP));
    m_pressure_cfg = {
        .mesurement_rate = DPS3XX_REG_VALUE_PM_RATE_4,
        .oversampling_rate = DPS3XX_REG_VALUE_PM_PRC_64,
        .mesurement_time = pdMS_TO_TICKS(DPS3XX_MEASUREMENT_TIME_MS_PRC_64 + portTICK_PERIOD_MS - 1),
        .scale_factor = float32_t((int32_t)DPS3XX_SCALE_FACTOR_PRC_64),
    };
    m_temperature_cfg = {
        .mesurement_rate = DPS3XX_REG_VALUE_PM_RATE_4,
        .oversampling_rate = DPS3XX_REG_VALUE_TMP_PRC_32,
        .mesurement_time = pdMS_TO_TICKS(DPS3XX_MEASUREMENT_TIME_MS_PRC_32 + portTICK_PERIOD_MS  - 1),
        .scale_factor = float32_t((int32_t)DPS3XX_SCALE_FACTOR_PRC_32),
    };
}

Dps3xxBarometer::~Dps3xxBarometer() {
    delete m_p_shallow_filter;
    delete m_p_deep_filter;
}

esp_err_t Dps3xxBarometer::CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr) {
    Dps3xxIdReg_t id;
    if (p_i2c_master->ReadByte(device_addr, DPS3XX_REG_ADDR_ID , &(id.byte)) == ESP_OK && id.byte == DPS3XX_REG_VALUE_ID) {
        DPS3XX_BARO_LOGI("DPS3xx device found at 0x%2.2x", device_addr);
        return ESP_OK;
    } else {
        DPS3XX_BARO_LOGE("DPS3xx device not found at 0x%2.2x", device_addr);
        return ESP_FAIL;
    }
}

esp_err_t Dps3xxBarometer::init_device() {
    Dps3xxResetReg_t reset = {0};
    reset.soft_reset = DPS3XX_REG_VALUE_SOFT_RESET;
    reset.fifo_flush = DPS3XX_REG_VALUE_FIFO_FLUSH;
    while (this->write_byte(DPS3XX_REG_ADDR_RESET, reset.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to reset %s device", m_p_instance_name);
        vTaskDelay(pdMS_TO_TICKS(portTICK_PERIOD_MS));
    }

    vTaskDelay(pdMS_TO_TICKS(DPS3XX_RESET_SENSOR_READY_MS + portTICK_PERIOD_MS - 1));

    // Read chip status and wait for sensor and coefficient data ready
    Dps3xxMeasCfgReg_t meas_cfg = {0};
    while (this->read_byte(DPS3XX_REG_ADDR_MEAS_CFG, &(meas_cfg.byte)) != ESP_OK ||  meas_cfg.sensor_rdy == 0 || meas_cfg.coef_rdy == 0) {
        DPS3XX_BARO_LOGI("Waiting for %s sensor and coefficient data ready", m_p_instance_name);
        vTaskDelay(pdMS_TO_TICKS(portTICK_PERIOD_MS));
    }

    // Read coefficient data
    if (this->get_coefs() != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to read %s coefficient data", m_p_instance_name);
        return ESP_FAIL;
    }

    // Set pressure measurement rate and oversampling rate
    Dps3xxPrsCfgReg_t prs_cfg = {0};
    prs_cfg.pm_rate = m_pressure_cfg.mesurement_rate;
    prs_cfg.pm_prc = m_pressure_cfg.oversampling_rate;
    if (this->write_byte(DPS3XX_REG_ADDR_PRS_CFG, prs_cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set %s pressure measurement rate and oversampling rate", m_p_instance_name);
        return ESP_FAIL;
    }

    // Set temperature measurement rate and oversampling rate
    Dps3xxTmpCfgReg_t tmp_cfg = {0};
    if (this->read_byte(DPS3XX_REG_ADDR_COEF_SRC, &(tmp_cfg.byte)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to read %s temperature coefficient source", m_p_instance_name);
        return ESP_FAIL;
    }
    tmp_cfg.tmp_rate = m_temperature_cfg.mesurement_rate;
    tmp_cfg.tmp_prc = m_temperature_cfg.oversampling_rate;
    if (this->write_byte(DPS3XX_REG_ADDR_TMP_CFG, tmp_cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set %s temperature measurement rate and oversampling rate", m_p_instance_name);
        return ESP_FAIL;
    }

    // Set pressure and temperature shift enable
    Dps3xxCfgReg_t cfg = {0};
    cfg.p_shift_en = 1;
    cfg.t_shift_en = 1;
    if (this->write_byte(DPS3XX_REG_ADDR_CFG_REG, cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set %s pressure and temperature shift enable", m_p_instance_name);
        return ESP_FAIL;
    }

    // Set pressure and temperature measurement mode to stop (idle mode, ready for single shot measurement)
    meas_cfg = {0};
    if (this->write_byte(DPS3XX_REG_ADDR_MEAS_CFG, meas_cfg.byte) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to set %s pressure and temperature measurement rate", m_p_instance_name);
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(portTICK_PERIOD_MS));

    return ESP_OK;
}

esp_err_t Dps3xxBarometer::deinit_device() {
    return ESP_OK;
}

esp_err_t Dps3xxBarometer::fetch_data(uint8_t *data, uint8_t size) {
    DPS3XX_BARO_ASSERT(size >= sizeof(Dps3xxData_t), "Buffer size is not enough to contain %s pressure and temperature structure.", m_p_instance_name);
    
    uint8_t meas_cfg;
    esp_err_t result;
    uint8_t retry;

    if ((result = this->write_byte(DPS3XX_REG_ADDR_MEAS_CFG, DPS3XX_REG_VALUE_MEAS_CTRL_TMP)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to write %s measurement configuration register", m_p_instance_name);
        return ESP_FAIL;
    } else {
        vTaskDelay(this->m_temperature_cfg.mesurement_time);
    }

    for (retry=0; retry<5; retry++) {
        if ((result = this->read_byte(DPS3XX_REG_ADDR_MEAS_CFG, &meas_cfg) != ESP_OK)) {
            DPS3XX_BARO_LOGE("Failed to read %s measurement configuration register", m_p_instance_name);
            return ESP_FAIL;
        } else if (meas_cfg & DPS3XX_REG_VALUE_TMP_RDY) {
            break;
        } else {
            DPS3XX_BARO_LOGV("Waiting for %s temperature data ready, meas_cfg: 0x%2.2x", m_p_instance_name, meas_cfg);
            vTaskDelay(pdMS_TO_TICKS(portTICK_PERIOD_MS));
        }
    }

    if (retry == 5) {
        DPS3XX_BARO_LOGE("Failed to get %s temperature data ready flag", m_p_instance_name);
        return ESP_FAIL;
    }

    if ((result = this->write_byte(DPS3XX_REG_ADDR_MEAS_CFG, DPS3XX_REG_VALUE_MEAS_CTRL_PRS)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to write %s measurement configuration register", m_p_instance_name);
        return ESP_FAIL;
    } else {
        vTaskDelay(this->m_pressure_cfg.mesurement_time);
    }

    for (retry=0; retry<5; retry++) {
        if ((result = this->read_byte(DPS3XX_REG_ADDR_MEAS_CFG, &meas_cfg) != ESP_OK)) {
            DPS3XX_BARO_LOGE("Failed to read %s measurement configuration register", m_p_instance_name);
            return ESP_FAIL;
        } else if (meas_cfg & DPS3XX_REG_VALUE_PRS_RDY) {
            break;
        } else {
            DPS3XX_BARO_LOGV("Waiting for %s pressure data ready, meas_cfg: 0x%2.2x", m_p_instance_name, meas_cfg);
            vTaskDelay(pdMS_TO_TICKS(portTICK_PERIOD_MS));
        }
    }

    if (retry == 5) {
        DPS3XX_BARO_LOGE("Failed to get %s pressure data ready flag", m_p_instance_name);
        return ESP_FAIL;
    }

    return this->read_buffer(DPS3XX_REG_ADDR_PSR_B2, data, sizeof(Dps3xxData_t));
}

esp_err_t Dps3xxBarometer::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    DPS3XX_BARO_ASSERT(in_size >= sizeof(Dps3xxData_t), "Buffer size is not enough to contain %s pressure and temperature structure.", m_p_instance_name);
        Dps3xxData_t *regs = (Dps3xxData_t *)in_data;

    int32_t raw_temperature = (int32_t)(((uint32_t)regs->tmp_b2 << 24) | ((uint32_t)regs->tmp_b1 << 16) | ((uint32_t)regs->tmp_b0 << 8)) >> 8;
    int32_t raw_pressure    = (int32_t)(((uint32_t)regs->prs_b2 << 24) | ((uint32_t)regs->prs_b1 << 16) | ((uint32_t)regs->prs_b0 << 8)) >> 8;

    DPS3XX_BARO_LOGD("Instance: %s, raw_temperature: 0x%8.8lx, raw_pressure: 0x%8.8lx", m_p_instance_name, raw_temperature, raw_pressure);

    float32_t temperature = m_coef_data.scaled_c0 + 
                            m_coef_data.scaled_c1 * raw_temperature;
    float32_t pressure    = m_coef_data.scaled_c00 + 
                            m_coef_data.scaled_c10 * raw_pressure + 
                            m_coef_data.scaled_c20 * raw_pressure * raw_pressure +
                            m_coef_data.scaled_c30 * raw_pressure * raw_pressure * raw_pressure +
                            m_coef_data.scaled_c01 * raw_temperature +
                            m_coef_data.scaled_c11 * raw_pressure * raw_temperature;

    DPS3XX_BARO_LOGD("%s %ld %ld %f %f", m_p_instance_name, raw_temperature, raw_pressure,(float)temperature, (float)pressure);

    // Generally, the air pressure value  is 300(@30km) ~ 101325(@0km) Pa, it is a positive value.
    // Since 101325 is 0x18BCD only 17 bits, so the maximum value of the pressure.e is -15.
    // Shift the pressure.m to make exponent tobe ((-15) + FILTER_DEPTH_XXX), then put it into the filter.
    this->m_p_deep_filter->PutSample(pressure.m >> ((AIR_PRESSURE_DEFAULT_VALUE_MSB + FILTER_DEPTH_DEEP - 31) - pressure.e));

    // Deep filter is used to get a stable value of the pressure to calculate the speed of wind by anemometer.
    // For barometer, use a shallow filter to get a relatively stable and low phase shift data.
    if (p_message != NULL) {
        int8_t shallow_offset = (AIR_PRESSURE_DEFAULT_VALUE_MSB + FILTER_DEPTH_SHALLOW - 31) - pressure.e;
        uint32_t prs_shallow_average = m_p_shallow_filter->PutSample(pressure.m >> ((FILTER_DEPTH_SHALLOW - 15) - pressure.e));

        p_message->type = BLUETHROAT_MSG_TYPE_BAROMETER;
        p_message->barometer_data.temperature = (float)temperature;
        // Left shift FILTER_DEPTH_SHALLOW instead of shallow_offset bits to avoid overflow.
        // E.g., if the sample's exponent just change to -16, while the last sevaral samples' exponent is -15, the average's exponent will be -15.
        // In this case, the shallow_offset will be 4, larger than FILTER_DEPTH_SHALLOW by 1, which will cause overflow.
        // In most cases, the pressure value is between 65536(0x10000) and 131071(0x1FFFF), just left shift FILTER_DEPTH_SHALLOW is enough.
        // If don't left shift before construct a float32_t, additional shift operations and MSB detection will cause a lot of load.
        p_message->barometer_data.pressure = (float)float32_t(pressure.s, prs_shallow_average << FILTER_DEPTH_SHALLOW, pressure.e + shallow_offset - FILTER_DEPTH_SHALLOW);

        DPS3XX_BARO_LOGD("Instance: %s send message, temperature: %f, pessure: %f", m_p_instance_name, p_message->barometer_data.temperature, p_message->barometer_data.pressure);
    }

    return ESP_OK;
}

esp_err_t Dps3xxBarometer::get_coefs() {
    Dps3xxCoefRegs_t coef_regs;
    if (this->read_buffer(DPS3XX_REG_ADDR_COEF, coef_regs.bytes, sizeof(Dps3xxCoefRegs_t)) != ESP_OK) {
        DPS3XX_BARO_LOGE("Failed to read %s coefficient registers", m_p_instance_name);
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

    DPS3XX_BARO_LOGI("instance: %s, c0: %ld, c1: %ld, c00: %ld, c10: %ld, c01: %ld, c11: %ld, c20: %ld, c21: %ld, c30: %ld", m_p_instance_name, c0, c1, c00, c10, c01, c11, c20, c21, c30);

    m_coef_data.scaled_c0  = float32_t(c0) / float32_t((int32_t)2);
    m_coef_data.scaled_c1   = float32_t(c1) / m_temperature_cfg.scale_factor;
    m_coef_data.scaled_c00  = float32_t(c00);
    m_coef_data.scaled_c10  = float32_t(c10) / m_pressure_cfg.scale_factor;
    m_coef_data.scaled_c01  = float32_t(c01) / m_temperature_cfg.scale_factor;
    m_coef_data.scaled_c11  = float32_t(c11) / m_pressure_cfg.scale_factor / m_temperature_cfg.scale_factor;
    m_coef_data.scaled_c20  = float32_t(c20) / m_pressure_cfg.scale_factor / m_pressure_cfg.scale_factor;  
    m_coef_data.scaled_c21  = float32_t(c21) / m_pressure_cfg.scale_factor / m_pressure_cfg.scale_factor / m_temperature_cfg.scale_factor;
    m_coef_data.scaled_c30  = float32_t(c30) / m_pressure_cfg.scale_factor / m_pressure_cfg.scale_factor / m_pressure_cfg.scale_factor;

    DPS3XX_BARO_LOGI("Scaled c0(%e) =  s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c0,  m_coef_data.scaled_c0.s,  m_coef_data.scaled_c0.m,  m_coef_data.scaled_c0.e);
    DPS3XX_BARO_LOGI("Scaled c1(%e) =  s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c1,  m_coef_data.scaled_c1.s,  m_coef_data.scaled_c1.m,  m_coef_data.scaled_c1.e);
    DPS3XX_BARO_LOGI("Scaled c00(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c00, m_coef_data.scaled_c00.s, m_coef_data.scaled_c00.m, m_coef_data.scaled_c00.e);
    DPS3XX_BARO_LOGI("Scaled c10(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c10, m_coef_data.scaled_c10.s, m_coef_data.scaled_c10.m, m_coef_data.scaled_c10.e);
    DPS3XX_BARO_LOGI("Scaled c01(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c01, m_coef_data.scaled_c01.s, m_coef_data.scaled_c01.m, m_coef_data.scaled_c01.e);
    DPS3XX_BARO_LOGI("Scaled c11(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c11, m_coef_data.scaled_c11.s, m_coef_data.scaled_c11.m, m_coef_data.scaled_c11.e);
    DPS3XX_BARO_LOGI("Scaled c20(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c20, m_coef_data.scaled_c20.s, m_coef_data.scaled_c20.m, m_coef_data.scaled_c20.e);
    DPS3XX_BARO_LOGI("Scaled c21(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c21, m_coef_data.scaled_c21.s, m_coef_data.scaled_c21.m, m_coef_data.scaled_c21.e);
    DPS3XX_BARO_LOGI("Scaled c30(%e) = s(%ld), m(0x%8.8lx), e(%ld)", (double)(float)m_coef_data.scaled_c30, m_coef_data.scaled_c30.s, m_coef_data.scaled_c30.m, m_coef_data.scaled_c30.e);

    return ESP_OK;
}
