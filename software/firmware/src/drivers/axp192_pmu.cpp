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

Axp192Pmu::Axp192Pmu() : I2cDevice() {
	AXP192_PMU_LOGI("Create axp192 pmu device.");
}

Axp192Pmu::~Axp192Pmu() {

}

esp_err_t Axp192Pmu::init_device() {
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
    set_charge_current(AXP192_REG_VALUE_CHARGING_INTER_CURRENT_280MA);
    set_charge_stop_current(AXP192_REG_VALUE_CHARGE_STOP_CUR_10PER);

    set_gpio0_level(true);
    set_gpio1_level(true);
    set_gpio2_level(false);
    set_gpio3_level(true);
    set_gpio4_level(true);

    set_gpio0_mode(AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD);
#if CONFIG_BLUETHROAD_TARGET_DEVICE_M5CORE2AWS
	set_gpio1_mode(AXP192_REG_VALUE_GPIO012_LDO_PWM);
#else
    set_gpio1_mode(AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD);
#endif
    set_gpio2_mode(AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD);
    set_gpio3_mode(AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD);
    set_gpio4_mode(AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD);

    return ESP_OK;
}

esp_err_t Axp192Pmu::deinit_device() {
    return ESP_OK;
}

esp_err_t Axp192Pmu::fetch_data(uint8_t *data, uint8_t size) {
    return ESP_OK;
}

esp_err_t Axp192Pmu::process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message) {
    return ESP_OK;
}

esp_err_t Axp192Pmu::enable_dcdc1(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.dcdc1_en = enable;
	result = write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s dcdc1 output.", (enable) ? "Enable" : "Disable");
	
    return ESP_OK;
}

esp_err_t Axp192Pmu::enable_dcdc2(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.dcdc2_en = (enable) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s dcdc2 output.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_dcdc3(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.dcdc3_en = (enable) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s dcdc3 output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_ldo2(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.ldo2_en = (enable) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s ldo2 output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_ldo3(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.ldo3_en = (enable) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power output control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s ldo3 output.", (enable) ? "Enable" : "Disable");
	
	return ESP_OK;
}

esp_err_t Axp192Pmu::enable_external_module(bool enable) {
	Axp192PowerOutputCtrlReg_t power_output_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, &(power_output_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power output control failed.");
		return result;
	}

	power_output_ctrl.ext_en = (enable) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_POWER_OUTPUT_CTRL, power_output_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_DCDC1_VOLT_CTRL, &(dcdc1_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc1 voltage control failed.");
		return result;
	}

	dcdc1_volt_ctrl.output_volt = (millivolt - AXP192_REG_VALUE_DCDC13_VOLT_MIN) / AXP192_REG_VALUE_DCDC13_VOLT_STEP;
	result = write_byte(AXP192_REG_ADDR_DCDC1_VOLT_CTRL, dcdc1_volt_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_DCDC2_VOLT_CTRL, &(dcdc2_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc2 voltage control failed.");
		return result;
	}

	dcdc2_volt_ctrl.output_volt = (millivolt - AXP192_REG_VALUE_DCDC2_VOLT_MIN) / AXP192_REG_VALUE_DCDC2_VOLT_STEP;
	result = write_byte(AXP192_REG_ADDR_DCDC2_VOLT_CTRL, dcdc2_volt_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_DCDC3_VOLT_CTRL, &(dcdc3_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc3 voltage control failed.");
		return result;
	}

	dcdc3_volt_ctrl.output_volt = (millivolt - AXP192_REG_VALUE_DCDC13_VOLT_MIN) / AXP192_REG_VALUE_DCDC13_VOLT_STEP;
	result = write_byte(AXP192_REG_ADDR_DCDC3_VOLT_CTRL, dcdc3_volt_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, &(ldo2_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read ldo2 voltage control failed.");
		return result;
	}

	ldo2_volt_ctrl.ldo2_output_volt = (millivolt - AXP192_REG_VALUE_LDO23_VOLT_MIN) / AXP192_REG_VALUE_LDO23_VOLT_STEP;
	result = write_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, ldo2_volt_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, &(ldo3_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read ldo3 voltage control failed.");
		return result;
	}

	ldo3_volt_ctrl.ldo3_output_volt = (millivolt - AXP192_REG_VALUE_LDO23_VOLT_MIN) / AXP192_REG_VALUE_LDO23_VOLT_STEP;
	result = write_byte(AXP192_REG_ADDR_LDO23_VOLT_CTRL, ldo3_volt_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write ldo3 voltage control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set ldo3 voltage to %d mV.", millivolt);

	return ESP_OK;
}

esp_err_t Axp192Pmu::software_enable_vbus(bool enable) {
	Axp192VbusConnCtrlReg_t vbus_conn_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_VBUS_CONENCT_CTRL, &(vbus_conn_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read vbus connect control failed.");
		return result;
	}

	vbus_conn_ctrl.vbus_en = (enable) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_VBUS_CONENCT_CTRL, vbus_conn_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write vbus connect control failed.");
		return result;
	}

	AXP192_PMU_LOGI("%s vbus connect to ipsout.", (enable) ? "Enable" : "Disable");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_voff_voltage(Axp192VoffVolt_t volt_index) {
	Axp192VoffCtrlReg_t voff_volt_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_VOFF_CTRL, &(voff_volt_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read voff voltage control failed.");
		return result;
	}

	voff_volt_ctrl.voff_volt = (uint8_t)volt_index;
	result = write_byte(AXP192_REG_ADDR_VOFF_CTRL, voff_volt_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_POWER_OFF_CTRL, &(power_off_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read power off control failed.");
		return result;
	}

	power_off_ctrl.power_off = 1;
	result = write_byte(AXP192_REG_ADDR_POWER_OFF_CTRL, power_off_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write power off control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Software power off.");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_charge_voltage(Axp192ChargeTargetVolt_t volt_index) {
	Axp192ChargeCtrl1Reg_t charge_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_CHARGE_CTRL, &(charge_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control failed.");
		return result;
	}

	charge_ctrl.target_volt = (uint8_t)volt_index;
	result = write_byte(AXP192_REG_ADDR_CHARGE_CTRL, charge_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control failed.");
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
	Axp192ChargeCtrl1Reg_t charge_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_CHARGE_CTRL, &(charge_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control failed.");
		return result;
	}

	charge_ctrl.inter_path_current = (uint8_t)current_index;
	result = write_byte(AXP192_REG_ADDR_CHARGE_CTRL, charge_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control failed.");
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
	Axp192ChargeCtrl1Reg_t charge_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_CHARGE_CTRL, &(charge_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read charge control failed.");
		return result;
	}

	charge_ctrl.inter_stop_current = (uint8_t)stop_current_index;
	result = write_byte(AXP192_REG_ADDR_CHARGE_CTRL, charge_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write charge control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set charge stop current index to %d(%s).", stop_current_index, \
	(stop_current_index == AXP192_REG_VALUE_CHARGE_STOP_CUR_10PER) ? "10%" : \
	(stop_current_index == AXP192_REG_VALUE_CHARGE_STOP_CUR_15PER) ? "15%" : "invalid value");

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_dcdc1_mode(Axp192DcdcMode_t mode) {
	Axp192DcdcModeCtrlReg_t dcdc_mode_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, &(dcdc_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc1 mode control failed.");
		return result;
	}

	dcdc_mode_ctrl.dcdc1_mode = mode;
	result = write_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, dcdc_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, &(dcdc_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc2 mode control failed.");
		return result;
	}

	dcdc_mode_ctrl.dcdc2_mode = mode;
	result = write_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, dcdc_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, &(dcdc_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read dcdc3 mode control failed.");
		return result;
	}

	dcdc_mode_ctrl.dcdc3_mode = mode;
	result = write_byte(AXP192_REG_ADDR_DCDC_MODE_CTRL, dcdc_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO0_MODE_CTRL, &(gpio0_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio0 mode control failed.");
		return result;
	}

	gpio0_mode_ctrl.mode = mode;
	result = write_byte(AXP192_REG_ADDR_GPIO0_MODE_CTRL, gpio0_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO1_MODE_CTRL, &(gpio1_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio1 mode control failed.");
		return result;
	}

	gpio1_mode_ctrl.mode = mode;
	result = write_byte(AXP192_REG_ADDR_GPIO1_MODE_CTRL, gpio1_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO2_MODE_CTRL, &(gpio2_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio2 mode control failed.");
		return result;
	}

	gpio2_mode_ctrl.mode = mode;
	result = write_byte(AXP192_REG_ADDR_GPIO2_MODE_CTRL, gpio2_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, &(gpio34_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio34 mode control failed.");
		return result;
	}

	gpio34_mode_ctrl.gpio3_mode = mode;
	result = write_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, gpio34_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, &(gpio34_mode_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio34 mode control failed.");
		return result;
	}

	gpio34_mode_ctrl.gpio4_mode = mode;
	result = write_byte(AXP192_REG_ADDR_GPIO34_MODE_CTRL, gpio34_mode_ctrl.byte);
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

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, &(gpio012_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio0 level control failed.");
		return result;
	}

	gpio012_level_ctrl.gpio0_level = (high) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, gpio012_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio0 level control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio0 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio1_level(bool high) {
	Axp192Gpio012LevelCtrlReg_t gpio012_level_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, &(gpio012_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio1 level control failed.");
		return result;
	}
	
	gpio012_level_ctrl.gpio1_level = (high) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, gpio012_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio1 level control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio1 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio2_level(bool high)	{
	Axp192Gpio012LevelCtrlReg_t gpio012_level_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, &(gpio012_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio2 level control failed.");
		return result;
	}

	gpio012_level_ctrl.gpio2_level = (high) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_GPIO012_LEVEL_CTRL, gpio012_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio2 level control failed.");
		return result;
	}
	
	AXP192_PMU_LOGI("Set gpio2 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio3_level(bool high) {
	Axp192Gpio34LevelCtrlReg_t gpio34_level_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, &(gpio34_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio3 level control failed.");
		return result;
	}

	gpio34_level_ctrl.gpio3_level = (high) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, gpio34_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio3 level control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio3 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
}

esp_err_t Axp192Pmu::set_gpio4_level(bool high) {
	Axp192Gpio34LevelCtrlReg_t gpio34_level_ctrl;

	esp_err_t result = read_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, &(gpio34_level_ctrl.byte));
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Read gpio4 level control failed.");
		return result;
	}

	gpio34_level_ctrl.gpio4_level = (high) ? 1 : 0;
	result = write_byte(AXP192_REG_ADDR_GPIO34_LEVEL_CTRL, gpio34_level_ctrl.byte);
	if (result != ESP_OK) {
		AXP192_PMU_LOGE("Write gpio4 level control failed.");
		return result;
	}

	AXP192_PMU_LOGI("Set gpio4 level to %d.", (high) ? 1 : 0);

	return ESP_OK;
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
