/***********************************************************************************************************************
 * If the AXP192 module is not configured, all codes in this file will be ignored. 
 * Use a configuration tool such as menuconfig to configure it.
***********************************************************************************************************************/
#include <sdkconfig.h>
#if CONFIG_I2C_DEVICE_AXP192
/***********************************************************************************************************************
 * If the AXP192 module is not configured, all codes in this file will be ignored. 
 * Use a configuration tool such as menuconfig to configure it.
***********************************************************************************************************************/

#include <esp_err.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>

#include "drivers/axp192_pmu.h"

#define AXP192_PMU_LOGE(format, ...) 				ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGW(format, ...) 				ESP_LOGW(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGI(format, ...) 				ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGD(format, ...) 				ESP_LOGD(TAG, format, ##__VA_ARGS__)
#define AXP192_PMU_LOGV(format, ...) 				ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define AXP192_PMU_BUFFER_LOGE(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_ERROR)
#define AXP192_PMU_BUFFER_LOGW(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_WARN)
#define AXP192_PMU_BUFFER_LOGI(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_INFO)
#define AXP192_PMU_BUFFER_LOGD(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_DEBUG)
#define AXP192_PMU_BUFFER_LOGV(buffer, buff_len) 	ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, buff_len, ESP_LOG_VERBOSE)

#ifdef _DEBUG
#define AXP192_PMU_ASSERT(condition, format, ...)   \
	do                                           \
	{                                            \
		if (!(condition))                        \
		{                                        \
			AXP192_PMU_LOGE(format, ##__VA_ARGS__); \
			assert(0);                           \
		}                                        \
	} while (0)
#else
#define AXP192_PMU_ASSERT(condition, format, ...)
#endif

static const char *TAG = "AXP192_PMU";

Axp192Pmu::Axp192Pmu() : I2cDevice(){
	AXP192_PMU_LOGI("Create axp192 pmu device.");
}

Axp192Pmu::~Axp192Pmu() {

}

esp_err_t Axp192Pmu::init_device() {
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED
	m_software_led_state = SOFTWARE_LED_STATE_OFF;
#endif

    enable_external_module(false);
    software_enable_vbus(false);
    set_voff_voltage(AXP192_REG_VALUE_VOFF_VOLT_3000MV);

    set_dcdc1_mode(AXP192_REG_VALUE_DCDC_MODE_PWM);
    set_dcdc3_mode(AXP192_REG_VALUE_DCDC_MODE_PFM_PWM);

    set_dcdc1_voltage(3300);
    set_dcdc3_voltage(700);

    enable_dcdc1(true);
    enable_dcdc2(false);
    enable_dcdc3(true);

    set_ldo2_voltage(3300);
    set_ldo3_voltage(2500);

    enable_ldo2(true);
    enable_ldo3(false);

    set_charge_voltage(AXP192_REG_VALUE_CHARGE_TARGET_4200MV);
    set_charge_current(calc_charging_current_index(I2C_DEVICE_AXP192_DEFAULT_CHARGING_CURRENT));
    set_charge_stop_current(AXP192_REG_VALUE_CHARGE_STOP_CUR_10PER);
	set_pre_charge_timeout(AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_60MIN);
	set_charge_timeout(AXP192_REG_VALUE_CHARGE_TIMEOUT_10H);

    set_gpio0_level(true);
    set_gpio0_mode(AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD);

#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
	set_pwm1_init_param(0x00, 0xff);
	set_pwm1_duty_cycle(0xff);
	set_gpio1_mode(AXP192_REG_VALUE_GPIO012_LDO_PWM);
#else
	set_gpio1_level(true);
    set_gpio1_mode(AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD);
#endif

    set_gpio2_level(false);
    set_gpio2_mode(AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD);
    set_gpio3_level(true);
    set_gpio3_mode(AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD);
    set_gpio4_level(true);
    set_gpio4_mode(AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD);

	enable_battery_voltage_adc(true);
	enable_battery_current_adc(true);

    return ESP_OK;
}

esp_err_t Axp192Pmu::deinit_device() {
    return ESP_OK;
}

esp_err_t Axp192Pmu::fetch_data(uint8_t *data, uint8_t size) {
	Axp192PmuStatus_t *p_pmu_status = (Axp192PmuStatus_t *)data;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_STATUS, &(p_pmu_status->power_status.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power status failed.");
		return result;
	}

	result = this->read_byte(AXP192_REG_ADDR_CHARGING_STATUS, &(p_pmu_status->charging_status.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charging status failed.");
		return result;
	}

	result = this->read_buffer(AXP192_REG_ADDR_BATTERY_VOLTAGE_H, p_pmu_status->battery_voltage, sizeof(p_pmu_status->battery_voltage));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read battery voltage failed.");
		return result;
	}

	result = this->read_buffer(AXP192_REG_ADDR_CHARGING_CURRENT_H, p_pmu_status->battery_charging_current, sizeof(p_pmu_status->battery_charging_current));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read battery charging current failed.");
		return result;
	}

	result = this->read_buffer(AXP192_REG_ADDR_DISCHARGING_CURRENT_H, p_pmu_status->battery_discharging_current, sizeof(p_pmu_status->battery_discharging_current));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read battery discharging current failed.");
		return result;
	}

	AXP192_PMU_LOGD("Read battery status successful, data are as follows:");
	AXP192_PMU_BUFFER_LOGD(p_pmu_status, sizeof(Axp192PmuStatus_t));

    return ESP_OK;
}

esp_err_t Axp192Pmu::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
	Axp192PmuStatus_t *p_pmu_status = (Axp192PmuStatus_t *)in_data;

#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED
    m_software_led_state = \
		(p_pmu_status->charging_status.battery_activating) ? SOFTWARE_LED_STATE_FLASH_FAST : \
		(p_pmu_status->charging_status.charge_undercurrent) ? SOFTWARE_LED_STATE_FLASH_SLOW : \
		(p_pmu_status->power_status.charging) ? SOFTWARE_LED_STATE_FLASH_NORMAL : \
		(p_pmu_status->power_status.acin_pres) ? SOFTWARE_LED_STATE_ON : \
		SOFTWARE_LED_STATE_OFF;
	
	software_led_loop();
#endif

	static uint8_t counter = 0;
	counter += m_p_task_param->task_interval;
	if (counter >= AXP192_PMU_STATUE_REPORT_INTERVAL) {
		counter = 0;

		uint16_t voltage = p_pmu_status->battery_voltage[0];
		voltage <<= 4; voltage |= (p_pmu_status->battery_voltage[1] & 0x0f);
		voltage *= 11; voltage /= 10;

		uint16_t charging_current = p_pmu_status->battery_charging_current[0];
		charging_current <<= 5; charging_current |= (p_pmu_status->battery_charging_current[1] & 0x1f);
		charging_current >>= 1;

		uint16_t discharging_current = p_pmu_status->battery_discharging_current[0];
		discharging_current <<= 5; discharging_current |= (p_pmu_status->battery_discharging_current[1] & 0x1f);
		discharging_current >>= 1;

		int16_t battery_current = discharging_current;
		battery_current -= charging_current;

		p_message->type = BLUETHROAT_MSG_TYPE_PMU;
		p_message->pmu_data.battery_voltage = voltage;
		p_message->pmu_data.battery_current = battery_current;
		p_message->pmu_data.battery_charging = p_pmu_status->power_status.charging;
		p_message->pmu_data.battery_activiting = p_pmu_status->charging_status.battery_activating;
		p_message->pmu_data.charge_undercurrent = p_pmu_status->charging_status.charge_undercurrent;

		AXP192_PMU_LOGI("Battary status: voltage=%dmV, charging_current=%dmA, discharging_current=%dmA, charging=%s, activating=%s, undercurrent=%s.", \
			p_message->pmu_data.battery_voltage, charging_current, discharging_current, \
			(p_message->pmu_data.battery_charging) ? "true" : "false", \
			(p_message->pmu_data.battery_activiting) ? "true" : "false", \
			(p_message->pmu_data.charge_undercurrent) ? "true" : "false");
	}

    return ESP_OK;
}

#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED
esp_err_t Axp192Pmu::software_led_loop() {
	static SoftwareLedState_t last_state = SOFTWARE_LED_STATE_OFF;
	esp_err_t result = ESP_OK;

	if (m_software_led_state == SOFTWARE_LED_STATE_OFF) {
		if (last_state != SOFTWARE_LED_STATE_OFF) {
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
			result = set_pwm1_duty_cycle(0xFF);
#else
			result = set_gpio1_level(true);
#endif
		}
	} else if (m_software_led_state == SOFTWARE_LED_STATE_ON) {
		if (last_state != SOFTWARE_LED_STATE_ON) {
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
			result = set_pwm1_duty_cycle(0x00);
#else
			result = set_gpio1_level(false);
#endif
		}
	} else {
		TickType_t ticks = xTaskGetTickCount();

#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
		static uint32_t last_index = 0;
		uint32_t index;
# else
		static bool last_level = false;
		uint32_t phrase, peroiod;
		bool level;
#endif

		switch (m_software_led_state) {
		case SOFTWARE_LED_STATE_FLASH_SLOW:
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
			index = (ticks % SOFTWARE_LED_FLASH_SLOW_PEROID_TICKS) * PWM_DUTY_CYCLE_TABLE_SIZE / SOFTWARE_LED_FLASH_SLOW_PEROID_TICKS;
#else
			phrase = ticks % SOFTWARE_LED_FLASH_SLOW_PEROID_TICKS;
			peroiod = SOFTWARE_LED_FLASH_SLOW_PEROID_TICKS;
#endif
			break;
		case SOFTWARE_LED_STATE_FLASH_NORMAL:
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
			index = (ticks % SOFTWARE_LED_FLASH_NORMAL_PEROID_TICKS) * PWM_DUTY_CYCLE_TABLE_SIZE / SOFTWARE_LED_FLASH_NORMAL_PEROID_TICKS;
#else
			phrase = ticks % SOFTWARE_LED_FLASH_NORMAL_PEROID_TICKS;
			peroiod = SOFTWARE_LED_FLASH_NORMAL_PEROID_TICKS;
#endif
			break;
		case SOFTWARE_LED_STATE_FLASH_FAST:
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
			index = (ticks % SOFTWARE_LED_FLASH_FAST_PEROID_TICKS) * PWM_DUTY_CYCLE_TABLE_SIZE / SOFTWARE_LED_FLASH_FAST_PEROID_TICKS;
#else
			phrase = ticks % SOFTWARE_LED_FLASH_FAST_PEROID_TICKS;
			peroiod = SOFTWARE_LED_FLASH_FAST_PEROID_TICKS;
#endif
			break;
		default:
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
			index = 0;
#else
			phrase = 1;
			peroiod = 2;
#endif
		}

#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
		if (index != last_index) {
			result = set_pwm1_duty_cycle(m_duty_cycle_table[index]);
			last_index = index;
		}
#else
		level = (phrase < peroiod * 372 / 1000) ? false : true;
		if (level != last_level) {
			result = set_gpio1_level(level);
			last_level = level;
		}
#endif
	}

	last_state = m_software_led_state;

	return result;
}
#endif

esp_err_t Axp192Pmu::enable_dcdc1(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.dcdc1_en = enable;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s dcdc1 output.", (enable) ? "Enable" : "Disable");
	
    return ESP_OK;
}

esp_err_t Axp192Pmu::enable_dcdc2(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.dcdc2_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s dcdc2 output.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_dcdc3(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.dcdc3_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s dcdc3 output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_ldo2(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.ldo2_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s ldo2 output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_ldo3(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.ldo3_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s ldo3 output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_external_module(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.ext_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s external module output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::set_dcdc1_voltage(uint16_t millivolt) {
	Axp192Dcdc13VoltCtrlReg_t dcdc1_volt_ctrl;

	if (millivolt < AXP192_REG_VALUE_DCDC13_VOLT_MIN || millivolt > AXP192_REG_VALUE_DCDC13_VOLT_MAX) {
		AXP192_PMU_LOGE("Invalid dcdc1 voltage.");
		return ESP_FAIL;
	}

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_DCDC1_VOLT_CTRL, &(dcdc1_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc1 voltage control failed.");
		return result;
	}

	dcdc1_volt_ctrl.output_volt = (millivolt - AXP192_REG_VALUE_DCDC13_VOLT_MIN) / AXP192_REG_VALUE_DCDC13_VOLT_STEP;
	result = this->write_byte(AXP192_REG_ADDR_DCDC1_VOLT_CTRL, dcdc1_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write dcdc1 voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set dcdc1 voltage to %d mV.", millivolt);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_dcdc2_voltage(uint16_t millivolt) {
	Axp192Dcdc2VoltCtrlReg_t dcdc2_volt_ctrl;

	if (millivolt < AXP192_REG_VALUE_DCDC2_VOLT_MIN || millivolt > AXP192_REG_VALUE_DCDC2_VOLT_MAX) {
		AXP192_PMU_LOGE("Invalid dcdc2 voltage.");
		return ESP_FAIL;
	}

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_DCDC2_VOLT_CTRL, &(dcdc2_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc2 voltage control failed.");
		return result;
	}

	dcdc2_volt_ctrl.output_volt = (millivolt - AXP192_REG_VALUE_DCDC2_VOLT_MIN) / AXP192_REG_VALUE_DCDC2_VOLT_STEP;
	result = this->write_byte(AXP192_REG_ADDR_DCDC2_VOLT_CTRL, dcdc2_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write dcdc2 voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set dcdc2 voltage to %d mV.", millivolt);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_dcdc3_voltage(uint16_t millivolt) {
	Axp192Dcdc13VoltCtrlReg_t dcdc3_volt_ctrl;

	if (millivolt < AXP192_REG_VALUE_DCDC13_VOLT_MIN || millivolt > AXP192_REG_VALUE_DCDC13_VOLT_MAX) {
		AXP192_PMU_LOGE("Invalid dcdc3 voltage.");
		return ESP_FAIL;
	}

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_DCDC3_VOLT_CTRL, &(dcdc3_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc3 voltage control failed.");
		return result;
	}

	dcdc3_volt_ctrl.output_volt = (millivolt - AXP192_REG_VALUE_DCDC13_VOLT_MIN) / AXP192_REG_VALUE_DCDC13_VOLT_STEP;
	result = this->write_byte(AXP192_REG_ADDR_DCDC3_VOLT_CTRL, dcdc3_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write dcdc3 voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set dcdc3 voltage to %d mV.", millivolt);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_ldo2_voltage(uint16_t millivolt) {
	Axp192Ldo23VoltCtrlReg_t ldo2_volt_ctrl;

	if (millivolt < AXP192_REG_VALUE_LDO23_VOLT_MIN || millivolt > AXP192_REG_VALUE_LDO23_VOLT_MAX) {
		AXP192_PMU_LOGE("Invalid ldo2 voltage.");
		return ESP_FAIL;
	}

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, &(ldo2_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read ldo2 voltage control failed.");
		return result;
	}

	ldo2_volt_ctrl.ldo2_output_volt = (millivolt - AXP192_REG_VALUE_LDO23_VOLT_MIN) / AXP192_REG_VALUE_LDO23_VOLT_STEP;
	result = this->write_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, ldo2_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write ldo2 voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set ldo2 voltage to %d mV.", millivolt);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_ldo3_voltage(uint16_t millivolt) {
	Axp192Ldo23VoltCtrlReg_t ldo3_volt_ctrl;

	if (millivolt < AXP192_REG_VALUE_LDO23_VOLT_MIN || millivolt > AXP192_REG_VALUE_LDO23_VOLT_MAX) {
		AXP192_PMU_LOGE("Invalid ldo3 voltage.");
		return ESP_FAIL;
	}

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, &(ldo3_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read ldo3 voltage control failed.");
		return result;
	}

	ldo3_volt_ctrl.ldo3_output_volt = (millivolt - AXP192_REG_VALUE_LDO23_VOLT_MIN) / AXP192_REG_VALUE_LDO23_VOLT_STEP;
	result = this->write_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, ldo3_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write ldo3 voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set ldo3 voltage to %d mV.", millivolt);

	return ESP_OK;
}

esp_err_t Axp192Pmu::software_enable_vbus(bool enable) {
	Axp192VbusConnCtrlReg_t vbus_conn_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_VBUS_CONENCT_CTRL, &(vbus_conn_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read vbus connect control failed.");
		return result;
	}

	vbus_conn_ctrl.vbus_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_VBUS_CONENCT_CTRL, vbus_conn_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write vbus connect control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s vbus connect to ipsout.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_voff_voltage(Axp192VoffVolt_t volt_index) {
	Axp192VoffCtrlReg_t voff_volt_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_VOFF_CTRL, &(voff_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read voff voltage control failed.");
		return result;
	}

	voff_volt_ctrl.voff_volt = (uint8_t)volt_index;
	result = this->write_byte(AXP192_REG_ADDR_VOFF_CTRL, voff_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write voff voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set voff voltage index to %d(%s).", volt_index, \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_2600MV) ? "2600mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_2700MV) ? "2700mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_2800MV) ? "2800mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_2900MV) ? "2900mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_3000MV) ? "3000mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_3100MV) ? "3100mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_3200MV) ? "3200mV" : \
	(volt_index == AXP192_REG_VALUE_VOFF_VOLT_3300MV) ? "3300mV" : "invalid value");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::power_off() {
	Axp192PowerOffCtrlReg_t power_off_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_POWER_OFF_CTRL, &(power_off_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power off control failed.");
		return result;
	}

	power_off_ctrl.power_off = 1;
	result = this->write_byte(AXP192_REG_ADDR_POWER_OFF_CTRL, power_off_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power off control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Software power off.");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_charge_voltage(Axp192ChargeTargetVolt_t volt_index) {
	Axp192ChargeCtrl1Reg_t charge_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_CHARGE_CTRL1, &(charge_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control register 1 failed.");
		return result;
	}

	charge_ctrl1.target_volt = (uint8_t)volt_index;
	result = this->write_byte(AXP192_REG_ADDR_CHARGE_CTRL1, charge_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set charge voltage index to %d(%s).", volt_index, \
	(volt_index == AXP192_REG_VALUE_CHARGE_TARGET_4100MV) ? "4100mV" : \
	(volt_index == AXP192_REG_VALUE_CHARGE_TARGET_4150MV) ? "4150mV" : \
	(volt_index == AXP192_REG_VALUE_CHARGE_TARGET_4200MV) ? "4200mV" : \
	(volt_index == AXP192_REG_VALUE_CHARGE_TARGET_4360MV) ? "4360mV" : "invalid value");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_charge_current(Axp192ChargingInterCurrent_t current_index) {
	Axp192ChargeCtrl1Reg_t charge_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_CHARGE_CTRL1, &(charge_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control register 1 failed.");
		return result;
	}

	charge_ctrl1.inter_path_current = (uint8_t)current_index;
	result = this->write_byte(AXP192_REG_ADDR_CHARGE_CTRL1, charge_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set charge current index to %d(%s).", current_index, \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_100MA) ? "100mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_190MA) ? "190mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_280MA) ? "280mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_360MA) ? "360mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_450MA) ? "450mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_550MA) ? "550mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_630MA) ? "630mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_700MA) ? "700mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_780MA) ? "780mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_880MA) ? "880mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_960MA) ? "960mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1000MA) ? "1000mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1080MA) ? "1080mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1160MA) ? "1160mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1240MA) ? "1240mA" : \
	(current_index == AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1320MA) ? "1320mA" : "invalid value");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_charge_stop_current(Axp192ChargeStopCur_t stop_current_index) {
	Axp192ChargeCtrl1Reg_t charge_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_CHARGE_CTRL1, &(charge_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control register 1 failed.");
		return result;
	}

	charge_ctrl1.inter_stop_current = (uint8_t)stop_current_index;
	result = this->write_byte(AXP192_REG_ADDR_CHARGE_CTRL1, charge_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set charge stop current index to %d(%s).", stop_current_index, \
	(stop_current_index == AXP192_REG_VALUE_CHARGE_STOP_CUR_10PER) ? "10%" : \
	(stop_current_index == AXP192_REG_VALUE_CHARGE_STOP_CUR_15PER) ? "15%" : "invalid value");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_pre_charge_timeout(Axp192PreChargeTimeout_t timeout_index) {
	Axp192ChargeCtrl2Reg_t charge_ctrl2;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_CHARGE_CTRL2, &(charge_ctrl2.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control register 2 failed.");
		return result;
	}

	charge_ctrl2.pre_charge_timeout = (uint8_t)timeout_index;
	result = this->write_byte(AXP192_REG_ADDR_CHARGE_CTRL2, charge_ctrl2.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control register 2 failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set pre-charge timeout index to %d(%s).", timeout_index, \
	(timeout_index == AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_30MIN) ? "30 minute" : \
	(timeout_index == AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_40MIN) ? "40 minute" : \
	(timeout_index == AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_50MIN) ? "50 minute" : \
	(timeout_index == AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_60MIN) ? "60 minute" : "invalid value");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_charge_timeout(Axp192ChargeTimeout_t timeout_index){
	Axp192ChargeCtrl2Reg_t charge_ctrl2;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_CHARGE_CTRL2, &(charge_ctrl2.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control register 2 failed.");
		return result;
	}

	charge_ctrl2.charge_timeout = (uint8_t)timeout_index;
	result = this->write_byte(AXP192_REG_ADDR_CHARGE_CTRL2, charge_ctrl2.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control register 2 failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set charge timeout index to %d(%s).", timeout_index, \
	(timeout_index == AXP192_REG_VALUE_CHARGE_TIMEOUT_7H) ? "7 hour" : \
	(timeout_index == AXP192_REG_VALUE_CHARGE_TIMEOUT_8H) ? "8 hour" : \
	(timeout_index == AXP192_REG_VALUE_CHARGE_TIMEOUT_9H) ? "9 hour" : \
	(timeout_index == AXP192_REG_VALUE_CHARGE_TIMEOUT_10H) ? "10 hour" : "invalid value");

	return ESP_OK;

}


esp_err_t Axp192Pmu::set_dcdc1_mode(Axp192DcdcMode_t mode) {
	Axp192DcdcModeCtrlReg_t dcdc_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, &(dcdc_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc1 mode control failed.");
		return result;
	}

	dcdc_mode_ctrl.dcdc1_mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, dcdc_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write dcdc1 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set dcdc1 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_DCDC_MODE_PFM_PWM) ? "PFM/PWM auto switch" : \
	(mode == AXP192_REG_VALUE_DCDC_MODE_PWM) ? "PWM mode" : "invalid mode");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_dcdc2_mode(Axp192DcdcMode_t mode) {
	Axp192DcdcModeCtrlReg_t dcdc_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, &(dcdc_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc2 mode control failed.");
		return result;
	}

	dcdc_mode_ctrl.dcdc2_mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, dcdc_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write dcdc2 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set dcdc2 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_DCDC_MODE_PFM_PWM) ? "PFM/PWM auto switch" : \
	(mode == AXP192_REG_VALUE_DCDC_MODE_PWM) ? "PWM mode" : "invalid mode");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_dcdc3_mode(Axp192DcdcMode_t mode) {
	Axp192DcdcModeCtrlReg_t dcdc_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, &(dcdc_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc3 mode control failed.");
		return result;
	}

	dcdc_mode_ctrl.dcdc3_mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, dcdc_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write dcdc3 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set dcdc3 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_DCDC_MODE_PFM_PWM) ? "PFM/PWM auto switch" : \
	(mode == AXP192_REG_VALUE_DCDC_MODE_PWM) ? "PWM mode" : "invalid mode");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio0_mode(Axp192Gpio012Mode_t mode) {
	Axp192Gpio012ModeCtrlReg_t gpio0_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO0_MODE_CTRL, &(gpio0_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio0 mode control failed.");
		return result;
	}

	gpio0_mode_ctrl.mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_GPIO0_MODE_CTRL, gpio0_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio0 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio0 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD) ? "NMOS open drain output" : \
	(mode == AXP192_REG_VALUE_GPIO012_GENERAL_INPUT) ? "General input" : \
	(mode == AXP192_REG_VALUE_GPIO012_LDO_PWM) ? "Low noise LDO" : \
	(mode == AXP192_REG_VALUE_GPIO012_RESERVED) ? "Reserved" : \
	(mode == AXP192_REG_VALUE_GPIO012_ADC_INPUT) ? "ADC input" : \
	(mode == AXP192_REG_VALUE_GPIO012_OUTPUT_LOW) ? "Output low" : \
	(mode == AXP192_REG_VALUE_GPIO012_FLOAT) ? "Float" : "invalid mode");


	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio1_mode(Axp192Gpio012Mode_t mode) {
	Axp192Gpio012ModeCtrlReg_t gpio1_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO1_MODE_CTRL, &(gpio1_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio1 mode control failed.");
		return result;
	}

	gpio1_mode_ctrl.mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_GPIO1_MODE_CTRL, gpio1_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio1 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio1 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD) ? "NMOS open drain output" : \
	(mode == AXP192_REG_VALUE_GPIO012_GENERAL_INPUT) ? "General input" : \
	(mode == AXP192_REG_VALUE_GPIO012_LDO_PWM) ? "PWM1 output" : \
	(mode == AXP192_REG_VALUE_GPIO012_RESERVED) ? "Reserved" : \
	(mode == AXP192_REG_VALUE_GPIO012_ADC_INPUT) ? "ADC input" : \
	(mode == AXP192_REG_VALUE_GPIO012_OUTPUT_LOW) ? "Output low" : \
	(mode == AXP192_REG_VALUE_GPIO012_FLOAT) ? "Float" : "invalid mode");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio2_mode(Axp192Gpio012Mode_t mode) {
	Axp192Gpio012ModeCtrlReg_t gpio2_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO2_MODE_CTRL, &(gpio2_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio2 mode control failed.");
		return result;
	}

	gpio2_mode_ctrl.mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_GPIO2_MODE_CTRL, gpio2_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio2 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio2 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD) ? "NMOS open drain output" : \
	(mode == AXP192_REG_VALUE_GPIO012_GENERAL_INPUT) ? "General input" : \
	(mode == AXP192_REG_VALUE_GPIO012_LDO_PWM) ? "PWM2 output" : \
	(mode == AXP192_REG_VALUE_GPIO012_RESERVED) ? "Reserved" : \
	(mode == AXP192_REG_VALUE_GPIO012_ADC_INPUT) ? "ADC input" : \
	(mode == AXP192_REG_VALUE_GPIO012_OUTPUT_LOW) ? "Output low" : \
	(mode == AXP192_REG_VALUE_GPIO012_FLOAT) ? "Float" : "invalid mode");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio3_mode(Axp192Gpio34Mode_t mode) {
	Axp192Gpio34ModeCtrlReg_t gpio34_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, &(gpio34_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio34 mode control failed.");
		return result;
	}

	gpio34_mode_ctrl.gpio3_mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, gpio34_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio34 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio3 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_GPIO34_EXT_CHARGE) ? "External charge control" : \
	(mode == AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD) ? "NMOS open drain output" : \
	(mode == AXP192_REG_VALUE_GPIO34_GENERAL_INPUT) ? "General input" : \
	(mode == AXP192_REG_VALUE_GPIO34_ADC_INPUT) ? "ADC input" : "invalid mode");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio4_mode(Axp192Gpio34Mode_t mode) {
	Axp192Gpio34ModeCtrlReg_t gpio34_mode_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, &(gpio34_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio34 mode control failed.");
		return result;
	}

	gpio34_mode_ctrl.gpio4_mode = mode;
	result = this->write_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, gpio34_mode_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio34 mode control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio4 mode to %d(%s).", mode, \
	(mode == AXP192_REG_VALUE_GPIO34_EXT_CHARGE) ? "External charge control" : \
	(mode == AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD) ? "NMOS open drain output" : \
	(mode == AXP192_REG_VALUE_GPIO34_GENERAL_INPUT) ? "General input" : \
	(mode == AXP192_REG_VALUE_GPIO34_ADC_INPUT) ? "ADC input" : "invalid mode");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio0_level(bool high) {
	Axp192Gpio012LevelCtrlReg_t gpio012_level_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, &(gpio012_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio0 level control failed.");
		return result;
	}

	gpio012_level_ctrl.gpio0_level = (high) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, gpio012_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio0 level control failed.");
		return result;
	}

	AXP192_PMU_LOGD("Set gpio0 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio1_level(bool high) {
	Axp192Gpio012LevelCtrlReg_t gpio012_level_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, &(gpio012_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio1 level control failed.");
		return result;
	}
	
	gpio012_level_ctrl.gpio1_level = (high) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, gpio012_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio1 level control failed.");
		return result;
	}

	AXP192_PMU_LOGD("Set gpio1 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio2_level(bool high)	{
	Axp192Gpio012LevelCtrlReg_t gpio012_level_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, &(gpio012_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio2 level control failed.");
		return result;
	}

	gpio012_level_ctrl.gpio2_level = (high) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, gpio012_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio2 level control failed.");
		return result;
	}
	
	AXP192_PMU_LOGD("Set gpio2 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio3_level(bool high) {
	Axp192Gpio34LevelCtrlReg_t gpio34_level_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, &(gpio34_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio3 level control failed.");
		return result;
	}

	gpio34_level_ctrl.gpio3_level = (high) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, gpio34_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio3 level control failed.");
		return result;
	}

	AXP192_PMU_LOGD("Set gpio3 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio4_level(bool high) {
	Axp192Gpio34LevelCtrlReg_t gpio34_level_ctrl;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, &(gpio34_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio4 level control failed.");
		return result;
	}

	gpio34_level_ctrl.gpio4_level = (high) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, gpio34_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio4 level control failed.");
		return result;
	}

	AXP192_PMU_LOGD("Set gpio4 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_pwm1_init_param(uint8_t clock_factor, uint8_t duty_cycle_divisor) {
	esp_err_t result = this->write_byte(AXP192_REG_ADDR_PWM1_FREQ_CTRL, clock_factor);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write pwm1 frequency control failed.");
		return result;
	}

	result = this->write_byte(AXP192_REG_ADDR_PWM1_DUTY_CTRL1, duty_cycle_divisor);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write pwm1 duty cycle control 1 failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set pwm1 frequency register to %d, duty cycle 1 register to %d.", clock_factor, duty_cycle_divisor);

	return ESP_OK;

}


esp_err_t Axp192Pmu::set_pwm1_duty_cycle(uint8_t duty_cycle) {
	esp_err_t result = this->write_byte(AXP192_REG_ADDR_PWM1_DUTY_CTRL2, duty_cycle);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write pwm1 duty cycle control 2 failed.");
		return result;
	}

	AXP192_PMU_LOGD("Set pwm1 duty cycle 2 register to %d.", duty_cycle);

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_battery_voltage_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.bat_volt_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s battery voltage adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_battery_current_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.bat_cur_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s battery current adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_acin_voltage_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.acin_volt_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s acin voltage adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_acin_current_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.acin_cur_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s acin current adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_vbus_voltage_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.vbus_volt_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s vbus voltage adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_vbus_current_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.vbus_cur_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s vbus current adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_aps_voltage_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.aps_volt_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s aps voltage adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_battery_temperature_adc(bool enable) {
	Axp192AdcCtrl1Reg_t adc_ctrl1;

	esp_err_t result = this->read_byte(AXP192_REG_ADDR_ADC_CTRL1, &(adc_ctrl1.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read adc control register 1 failed.");
		return result;
	}

	adc_ctrl1.bat_temp_adc_en = (enable) ? 1 : 0;
	result = this->write_byte(AXP192_REG_ADDR_ADC_CTRL1, adc_ctrl1.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write adc control register 1 failed.");
		return result;
	}

	AXP192_PMU_LOGD("%s battery temperature adc.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

Axp192ChargingInterCurrent_t Axp192Pmu::calc_charging_current_index(uint32_t current_ma) {
	return
	(current_ma <= 145) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_100MA :
	(current_ma <= 235) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_190MA :
	(current_ma <= 320) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_280MA :
	(current_ma <= 405) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_360MA :
	(current_ma <= 500) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_450MA :
	(current_ma <= 590) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_550MA :
	(current_ma <= 665) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_630MA :
	(current_ma <= 740) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_700MA :
	(current_ma <= 830) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_780MA :
	(current_ma <= 920) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_880MA :
	(current_ma <= 980) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_960MA :
	(current_ma <= 1040) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1000MA :
	(current_ma <= 1120) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1080MA :
	(current_ma <= 1200) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1160MA :
	(current_ma <= 1280) ? AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1240MA :
	AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1320MA;
}

Axp192Pmu *g_p_axp192_pmu = NULL;

/***********************************************************************************************************************
* Axp192 virbate motor function group.
***********************************************************************************************************************/
#define VIRBRATE_TIME_IN_MS					(100)
#define VIRBRATE_TIMER_DELETE_DELAY_IN_MS	(50)
static uint32_t vibrate_counter = 0;
static portMUX_TYPE vibrate_counter_lock = SPINLOCK_INITIALIZER;

static void virbrate_timer_callback(TimerHandle_t handle) {
	taskENTER_CRITICAL(&vibrate_counter_lock);
	vibrate_counter--;
	taskEXIT_CRITICAL(&vibrate_counter_lock);

	if (vibrate_counter == 0) {
		if (g_p_axp192_pmu == NULL) {
			AXP192_PMU_LOGE("Axp192 pmu is not initialized.");
		} else {
    		g_p_axp192_pmu->enable_ldo3(false);
		}
	}

    xTimerDelete(handle, pdMS_TO_TICKS(VIRBRATE_TIMER_DELETE_DELAY_IN_MS));
}

esp_err_t VibrateMotor() {
	TimerHandle_t handle = xTimerCreate("vibrate", pdMS_TO_TICKS(VIRBRATE_TIME_IN_MS), pdFALSE, NULL, virbrate_timer_callback);

	if (handle != NULL) {
		if(xTimerStart(handle, 0) == pdPASS) {
			taskENTER_CRITICAL(&vibrate_counter_lock);
			vibrate_counter++;
			taskEXIT_CRITICAL(&vibrate_counter_lock);

			if (g_p_axp192_pmu == NULL) {
				AXP192_PMU_LOGE("Axp192 pmu is not initialized.");
				return ESP_FAIL;
			} else {
				return g_p_axp192_pmu->enable_ldo3(true);
			}
		} else {
			xTimerDelete(handle, pdMS_TO_TICKS(VIRBRATE_TIMER_DELETE_DELAY_IN_MS));
			return ESP_FAIL;
		}
	} else {
		return ESP_FAIL;
	}
}

esp_err_t SetScreenBrightness(uint8_t percent) {
	return ESP_OK;
}

esp_err_t SystemPowerOff() {
	if (g_p_axp192_pmu == NULL) {
		AXP192_PMU_LOGE("Axp192 pmu is not initialized.");
		return ESP_FAIL;
	}

	return g_p_axp192_pmu->power_off();
}

esp_err_t EnableBusPower(bool enable) {
	if (g_p_axp192_pmu == NULL) {
		AXP192_PMU_LOGE("Axp192 pmu is not initialized.");
		return ESP_FAIL;
	}

	return g_p_axp192_pmu->enable_external_module(enable);
}

esp_err_t EnableSpeaker(bool enable) {
	if (g_p_axp192_pmu == NULL) {
		AXP192_PMU_LOGE("Axp192 pmu is not initialized.");
		return ESP_FAIL;
	}

	return g_p_axp192_pmu->set_gpio2_level(enable);
}

esp_err_t ResetScreen() {
	if (g_p_axp192_pmu == NULL) {
		AXP192_PMU_LOGE("Axp192 pmu is not initialized.");
		return ESP_FAIL;
	}

	esp_err_t result = g_p_axp192_pmu->set_gpio4_level(0);
	vTaskDelay(1);
	result |= g_p_axp192_pmu->set_gpio4_level(1);

	return result;
}

/***********************************************************************************************************************
 * If the AXP192 module is not configured, all codes in this file will be ignored. 
 * Use a configuration tool such as menuconfig to configure it.
***********************************************************************************************************************/
#endif
/***********************************************************************************************************************
 * If the AXP192 module is not configured, all codes in this file will be ignored. 
 * Use a configuration tool such as menuconfig to configure it.
***********************************************************************************************************************/
