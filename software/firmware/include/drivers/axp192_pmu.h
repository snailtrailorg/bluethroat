#pragma once

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

#include "drivers/i2c_device.h"

/***********************************************************************************************************************
 * Axp192 charging current configuration parameters defination
***********************************************************************************************************************/
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_CURRENT_01C
#define I2C_DEVICE_AXP192_DEFAULT_CHARGING_CURRENT (CONFIG_I2C_DEVICE_AXP192_BATTERY_CAPACITY_MAH * 10 / 100)
#elif CONFIG_I2C_DEVICE_AXP192_CHARGING_CURRENT_03C
#define I2C_DEVICE_AXP192_DEFAULT_CHARGING_CURRENT (CONFIG_I2C_DEVICE_AXP192_BATTERY_CAPACITY_MAH * 30 / 100)
#elif CONFIG_I2C_DEVICE_AXP192_CHARGING_CURRENT_05C
#define I2C_DEVICE_AXP192_DEFAULT_CHARGING_CURRENT (CONFIG_I2C_DEVICE_AXP192_BATTERY_CAPACITY_MAH * 50 / 100)
#elif CONFIG_I2C_DEVICE_AXP192_CHARGING_CURRENT_07C
#define I2C_DEVICE_AXP192_DEFAULT_CHARGING_CURRENT (CONFIG_I2C_DEVICE_AXP192_BATTERY_CAPACITY_MAH * 70 / 100)
#elif CONFIG_I2C_DEVICE_AXP192_CHARGING_CURRENT_10C
#define I2C_DEVICE_AXP192_DEFAULT_CHARGING_CURRENT (CONFIG_I2C_DEVICE_AXP192_BATTERY_CAPACITY_MAH * 100 / 100)
#endif

/***********************************************************************************************************************
* Axp192 power status registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_POWER_STATUS            (0x00)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t power_on_src        : 1;
        uint8_t acin_vbus_short     : 1;
        uint8_t charging            : 1;
        uint8_t vbus_gt_vhold       : 1;
        uint8_t vbus_vld            : 1;
        uint8_t vbus_pres           : 1;
        uint8_t acin_vld            : 1;
        uint8_t acin_pres           : 1;
#else
        uint8_t acin_pres           : 1;
        uint8_t acin_vld            : 1;
        uint8_t vbus_pres           : 1;
        uint8_t vbus_vld            : 1;
        uint8_t vbus_gt_vhold       : 1;
        uint8_t charging            : 1;
        uint8_t acin_vbus_short     : 1;
        uint8_t power_on_src        : 1;
#endif
    };
} __attribute__ ((packed)) Axp192PowerStatusReg_t;

#define AXP192_REG_VALUE_POWER_STATUS_CHARGING  (0x04)

/***********************************************************************************************************************
 * Axp192 power mode control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_CHARGING_STATUS         (0x01)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t                     : 1;
        uint8_t power_on_off_type   : 1;
        uint8_t charge_undercurrent : 1;
        uint8_t battery_activating  : 1;
        uint8_t                     : 1;
        uint8_t has_battery         : 1;
        uint8_t battery_charging    : 1;
        uint8_t battery_overheat    : 1;
#else
        uint8_t battery_overheat    : 1;
        uint8_t battery_charging    : 1;
        uint8_t has_battery         : 1;
        uint8_t                     : 1;
        uint8_t battery_activating  : 1;
        uint8_t charge_undercurrent : 1;
        uint8_t power_on_off_type   : 1;
        uint8_t                     : 1;
#endif 
    };
} __attribute__ ((packed)) Axp192ChargingStatusReg_t;

#define AXP192_REG_VALUE_POWER_ON_OFF_TYPE_A    (0x00)
#define AXP192_REG_VALUE_POWER_ON_OFF_TYPE_B    (0x01)

/***********************************************************************************************************************
* Axp192 power output control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_POWER_OUTPUT_CTRL       (0x12)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t dcdc1_en            : 1;
        uint8_t dcdc3_en            : 1;
        uint8_t ldo2_en             : 1;
        uint8_t ldo3_en             : 1;
        uint8_t dcdc2_en            : 1;
        uint8_t                     : 1;
        uint8_t ext_en              : 1;
        uint8_t                     : 1;
#else
        uint8_t                     : 1;
        uint8_t ext_en              : 1;
        uint8_t                     : 1;
        uint8_t dcdc2_en            : 1;
        uint8_t ldo3_en             : 1;
        uint8_t ldo2_en             : 1;
        uint8_t dcdc3_en            : 1;
        uint8_t dcdc1_en            : 1;
#endif
    };
} __attribute__ ((packed)) Axp192PowerOutputCtrlReg_t;

/***********************************************************************************************************************
* Axp192 dcdc2 output voltage control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_DCDC2_VOLT_CTRL         (0x16)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t output_volt         : 6;
        uint8_t                     : 2;
#else
        uint8_t                     : 2;
        uint8_t output_volt         : 6;
#endif
    };
} __attribute__ ((packed)) Axp192Dcdc2VoltCtrlReg_t;

#define AXP192_REG_VALUE_DCDC2_VOLT_MIN         (700)
#define AXP192_REG_VALUE_DCDC2_VOLT_MAX         (2275)
#define AXP192_REG_VALUE_DCDC2_VOLT_STEP        (25)

/***********************************************************************************************************************
* Axp192 dcdc1 & dcdc3 output voltage control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_DCDC1_VOLT_CTRL         (0x26)
#define AXP192_REG_ADDR_DCDC3_VOLT_CTRL         (0x27)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t output_volt         : 7;
        uint8_t                     : 1;
#else
        uint8_t                     : 1;
        uint8_t output_volt         : 7;
#endif
    };
} __attribute__ ((packed)) Axp192Dcdc13VoltCtrlReg_t;

#define AXP192_REG_VALUE_DCDC13_VOLT_MIN        (700)
#define AXP192_REG_VALUE_DCDC13_VOLT_MAX        (3500)
#define AXP192_REG_VALUE_DCDC13_VOLT_STEP       (25)

/***********************************************************************************************************************
* Axp192 ldo2 and ldo3 output voltage control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_LDO23_VOLT_CTRL         (0x28)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t ldo3_output_volt    : 4;
        uint8_t ldo2_output_volt    : 4;
#else
        uint8_t ldo2_output_volt    : 4;
        uint8_t ldo3_output_volt    : 4;
#endif
    };
} __attribute__ ((packed)) Axp192Ldo23VoltCtrlReg_t;

#define AXP192_REG_VALUE_LDO23_VOLT_MIN         (1800)
#define AXP192_REG_VALUE_LDO23_VOLT_MAX         (3300)
#define AXP192_REG_VALUE_LDO23_VOLT_STEP        (100)

/***********************************************************************************************************************
* Axp192 vbus and ipsout connection control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_VBUS_CONENCT_CTRL       (0x30)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t current_limit_100ma : 1;
        uint8_t current_limit_en    : 1;
        uint8_t                     : 1;
        uint8_t vhold_volts         : 3;
        uint8_t vhold_en            : 1;
        uint8_t vbus_en             : 1;
#else
        uint8_t vbus_en             : 1;
        uint8_t vhold_en            : 1;
        uint8_t vhold_volts         : 3;
        uint8_t                     : 1;
        uint8_t current_limit_en    : 1;
        uint8_t current_limit_100ma : 1;
#endif
    };
} __attribute__ ((packed)) Axp192VbusConnCtrlReg_t;

/***********************************************************************************************************************
* Axp192 voff power down voltage control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_VOFF_CTRL               (0x31)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t voff_volt           : 3;
        uint8_t pwron_wakeup_en     : 1;
        uint8_t                     : 4;
#else
        uint8_t                     : 4;
        uint8_t pwron_wakeup_en     : 1;
        uint8_t voff_volt           : 3;
#endif
    };
} __attribute__ ((packed)) Axp192VoffCtrlReg_t;

typedef enum {
    AXP192_REG_VALUE_VOFF_VOLT_2600MV = 0x01,
    AXP192_REG_VALUE_VOFF_VOLT_2700MV,
    AXP192_REG_VALUE_VOFF_VOLT_2800MV,
    AXP192_REG_VALUE_VOFF_VOLT_2900MV,
    AXP192_REG_VALUE_VOFF_VOLT_3000MV,
    AXP192_REG_VALUE_VOFF_VOLT_3100MV,
    AXP192_REG_VALUE_VOFF_VOLT_3200MV,
    AXP192_REG_VALUE_VOFF_VOLT_3300MV,
} Axp192VoffVolt_t;

/***********************************************************************************************************************
* Axp192 voff power down voltage control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_POWER_OFF_CTRL          (0x32)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t noe_delay           : 2;
        uint8_t                     : 1;
        uint8_t chgled_ctrl_en      : 1;
        uint8_t chgled_mode         : 2;
        uint8_t bat_mon_en          : 1;
        uint8_t power_off           : 1;
#else
        uint8_t power_off           : 1;
        uint8_t bat_mon_en          : 1;
        uint8_t chgled_mode         : 2;
        uint8_t chgled_ctrl_en      : 1;
        uint8_t                     : 1;
        uint8_t noe_delay           : 2;
#endif
    };
} __attribute__ ((packed)) Axp192PowerOffCtrlReg_t;

typedef enum {
    AXP192_REG_VALUE_CHGLED_MODE_HIGH_RES = 0x00,
    AXP192_REG_VALUE_CHGLED_MODE_FLASH_1HZ,
    AXP192_REG_VALUE_CHGLED_MODE_FLASH_4HZ,
    AXP192_REG_VALUE_CHGLED_MODE_LOW_LEVEL,
} Axp192ChgledMode_t;

/***********************************************************************************************************************
* Axp192 battery charge control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_CHARGE_CTRL1            (0x33)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t inter_path_current  : 4;
        uint8_t inter_stop_current  : 1;
        uint8_t target_volt         : 2;
        uint8_t charge_en           : 1;
#else
        uint8_t charge_en           : 1;
        uint8_t target_volt         : 2;
        uint8_t inter_stop_current  : 1;
        uint8_t inter_path_current  : 4;
#endif
    };
} __attribute__ ((packed)) Axp192ChargeCtrl1Reg_t;

typedef enum {
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_100MA = 0x00,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_190MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_280MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_360MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_450MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_550MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_630MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_700MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_780MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_880MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_960MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1000MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1080MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1160MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1240MA,
    AXP192_REG_VALUE_CHARGING_INTER_CURRENT_1320MA,
} Axp192ChargingInterCurrent_t;

typedef enum {
    AXP192_REG_VALUE_CHARGE_STOP_CUR_10PER = 0x00,
    AXP192_REG_VALUE_CHARGE_STOP_CUR_15PER,
} Axp192ChargeStopCur_t;

typedef enum {
    AXP192_REG_VALUE_CHARGE_TARGET_4100MV = 0x00,
    AXP192_REG_VALUE_CHARGE_TARGET_4150MV,
    AXP192_REG_VALUE_CHARGE_TARGET_4200MV,
    AXP192_REG_VALUE_CHARGE_TARGET_4360MV,
} Axp192ChargeTargetVolt_t;

/***********************************************************************************************************************
 * Axp192 battery charge control2 registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_CHARGE_CTRL2            (0x34)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t charge_timeout      : 2;
        uint8_t exter_path_en       : 1;
        uint8_t exter_path_current  : 3;
        uint8_t pre_charge_timeout  : 2;
#else
        uint8_t pre_charge_timeout  : 2;
        uint8_t exter_path_current  : 3;
        uint8_t exter_path_en       : 1;
        uint8_t charge_timeout      : 2;
#endif
    };
} __attribute__ ((packed)) Axp192ChargeCtrl2Reg_t;

typedef enum {
    AXP192_REG_VALUE_CHARGE_TIMEOUT_7H = 0x00,
    AXP192_REG_VALUE_CHARGE_TIMEOUT_8H,
    AXP192_REG_VALUE_CHARGE_TIMEOUT_9H,
    AXP192_REG_VALUE_CHARGE_TIMEOUT_10H,
} Axp192ChargeTimeout_t;

typedef enum {
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_300MA = 0x00,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_400MA,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_500MA,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_600MA,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_700MA,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_800MA,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_900MA,
    AXP192_REG_VALUE_EXTER_PATH_CURRENT_1000MA,
} Axp192ExterPathCurrent_t;

typedef enum {
    AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_30MIN = 0x00,
    AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_40MIN,
    AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_50MIN,
    AXP192_REG_VALUE_PRE_CHARGE_TIMEOUT_60MIN,
} Axp192PreChargeTimeout_t;

/***********************************************************************************************************************
 * Axp192 spare battery charge control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_SPARE_BAT_CHARGE_CTRL   (0x35)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t current             : 2;
        uint8_t                     : 3;
        uint8_t target_volt         : 2;
        uint8_t charge_en           : 1;
#else
        uint8_t charge_en           : 1;
        uint8_t target_volt         : 2;
        uint8_t                     : 3;
        uint8_t current             : 2;
#endif
    };
} __attribute__ ((packed)) Axp192SpareBatChargeCtrlReg_t;

/***********************************************************************************************************************
 * Axp192 dcdc mode control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_DCDC_MODE_CTRL          (0x80)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t                     : 1;
        uint8_t dcdc3_mode          : 1;
        uint8_t dcdc2_mode          : 1;
        uint8_t dcdc1_mode          : 1;
        uint8_t                     : 4;
#else
        uint8_t                     : 4;
        uint8_t dcdc1_mode          : 1;
        uint8_t dcdc2_mode          : 1;
        uint8_t dcdc3_mode          : 1;
        uint8_t                     : 1;
#endif
    };
} __attribute__ ((packed)) Axp192DcdcModeCtrlReg_t;

typedef enum {
    AXP192_REG_VALUE_DCDC_MODE_PFM_PWM        = 0x00,
    AXP192_REG_VALUE_DCDC_MODE_PWM            = 0x01,
} Axp192DcdcMode_t;

/***********************************************************************************************************************
 * Axp192 gpio0 mode control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_GPIO0_MODE_CTRL         (0x90)
#define AXP192_REG_ADDR_GPIO1_MODE_CTRL         (0x92)
#define AXP192_REG_ADDR_GPIO2_MODE_CTRL         (0x93)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t mode                : 3;
        uint8_t                     : 5;
#else
        uint8_t                     : 5;
        uint8_t mode                : 3;
#endif
    };
} __attribute__ ((packed)) Axp192Gpio012ModeCtrlReg_t;

typedef enum {
    AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD   = 0x00,
    AXP192_REG_VALUE_GPIO012_GENERAL_INPUT    = 0x01,
    AXP192_REG_VALUE_GPIO012_LDO_PWM          = 0x02,
    AXP192_REG_VALUE_GPIO012_RESERVED         = 0x03,
    AXP192_REG_VALUE_GPIO012_ADC_INPUT        = 0x04,
    AXP192_REG_VALUE_GPIO012_OUTPUT_LOW       = 0x05,
    AXP192_REG_VALUE_GPIO012_FLOAT            = 0x07,
} Axp192Gpio012Mode_t;

/***********************************************************************************************************************
 * Axp192 gpio[2:0] status and level control registers address，structure and related configuration value defination
 * Valid for gpio0, gpio1 and gpio2 set to mode AXP192_REG_VALUE_GPIO012_OUTPUT_NMOS_OD
***********************************************************************************************************************/
#define AXP192_REG_ADDR_GPIO012_LEVEL_CTRL      (0x94)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t gpio0_level         : 1;
        uint8_t gpio1_level         : 1;
        uint8_t gpio2_level         : 1;
        uint8_t                     : 1;
        uint8_t gpio0_status        : 1;
        uint8_t gpio1_status        : 1;
        uint8_t gpio2_status        : 1;
        uint8_t                     : 1;
#else
        uint8_t                     : 1;
        uint8_t gpio2_status        : 1;
        uint8_t gpio1_status        : 1;
        uint8_t gpio0_status        : 1;
        uint8_t                     : 1;
        uint8_t gpio2_level         : 1;
        uint8_t gpio1_level         : 1;
        uint8_t gpio0_level         : 1;
#endif
    };
} __attribute__ ((packed)) Axp192Gpio012LevelCtrlReg_t;

/***********************************************************************************************************************
 * Axp192 gpio[4:3] mode control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_GPIO34_MODE_CTRL        (0x95)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t gpio3_mode          : 2;
        uint8_t gpio4_mode          : 2;
        uint8_t                     : 3;
        uint8_t gpio34_en           : 1;
#else
        uint8_t gpio34_en           : 1;
        uint8_t                     : 3;
        uint8_t gpio4_mode          : 2;
        uint8_t gpio3_mode          : 2;
#endif
    };
} __attribute__ ((packed)) Axp192Gpio34ModeCtrlReg_t;

typedef enum {
    AXP192_REG_VALUE_GPIO34_EXT_CHARGE        = 0x00,
    AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD    = 0x01,
    AXP192_REG_VALUE_GPIO34_GENERAL_INPUT     = 0x02,
    AXP192_REG_VALUE_GPIO34_ADC_INPUT         = 0x03,
} Axp192Gpio34Mode_t;

/***********************************************************************************************************************
 * Axp192 gpio[4:3] status and level control registers address，structure and related configuration value defination
 * Valid for gpio3 and gpio4 set to mode AXP192_REG_VALUE_GPIO34_OUTPUT_NMOS_OD
***********************************************************************************************************************/
#define AXP192_REG_ADDR_GPIO34_LEVEL_CTRL       (0x96)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t gpio3_level         : 1;
        uint8_t gpio4_level         : 1;
        uint8_t                     : 2;
        uint8_t gpio3_status        : 1;
        uint8_t gpio4_status        : 1;
        uint8_t                     : 2;
#else
        uint8_t                     : 2;
        uint8_t gpio4_status        : 1;
        uint8_t gpio3_status        : 1;
        uint8_t                     : 2;
        uint8_t gpio4_level         : 1;
        uint8_t gpio3_level         : 1;
#endif
    };
} __attribute__ ((packed)) Axp192Gpio34LevelCtrlReg_t;

/***********************************************************************************************************************
 * Axp192 battery voltage registers address defination
 * The battery voltage is a 12-bit data combined with two registers, first contains high 8 bits data, 
 * following contains low 4 bits data in bits[3:0].
 * the voltage value is calculated as follows: Vbat = (Vbat_reg * 1.1) / 1000
***********************************************************************************************************************/
#define AXP192_REG_ADDR_BATTERY_VOLTAGE_HIGH    (0x78)
#define AXP192_REG_ADDR_BATTERY_VOLTAGE_LOW     (0x79)

/***********************************************************************************************************************
 * Axp192 PWM frequency and duty cycle registers address defination
 * PWM output frequency = 2.25MHz / (FREQ_CTRL + 1) / DUTY_CTRL1
 * PWM output duty cycle = DUTY_CTRL2 / DUTY_CTRL1
 * FREQ_CTRL, DUTY_CTRL1 and DUTY_CTRL2 are 8-bit registers
***********************************************************************************************************************/
#define AXP192_REG_ADDR_PWM1_FREQ_CTRL          (0x98)
#define AXP192_REG_ADDR_PWM1_DUTY_CTRL1         (0x99)
#define AXP192_REG_ADDR_PWM1_DUTY_CTRL2         (0x9A)

/***********************************************************************************************************************
 * Axp192 software LED configuration parameters defination
***********************************************************************************************************************/
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED

typedef enum {
    SOFTWARE_LED_STATE_OFF = 0,
    SOFTWARE_LED_STATE_FLASH_SLOW,
    SOFTWARE_LED_STATE_FLASH_NORMAL,
    SOFTWARE_LED_STATE_FLASH_FAST,
    SOFTWARE_LED_STATE_ON,
} SoftwareLedState_t;

#define SOFTWARE_LED_FLASH_SLOW_PEROID_TICKS    (pdMS_TO_TICKS(2000))
#define SOFTWARE_LED_FLASH_NORMAL_PEROID_TICKS  (pdMS_TO_TICKS(1000))
#define SOFTWARE_LED_FLASH_FAST_PEROID_TICKS    (pdMS_TO_TICKS(500))

#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
#define PWM_DUTY_CYCLE_TABLE_SIZE               (0x100)
#endif

#endif

/***********************************************************************************************************************
 * Axp192 battery status registers, defined for periodic battery status report
***********************************************************************************************************************/
typedef struct {
    Axp192PowerStatusReg_t power_status;
    Axp192ChargingStatusReg_t charging_status;
    uint8_t bat_volt[2];
}  __attribute__ ((packed)) Axp192PmuStatus_t;

/***********************************************************************************************************************
* @brief Axp192 PMU class
***********************************************************************************************************************/
class Axp192Pmu : public I2cDevice {
public:
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED
    SoftwareLedState_t m_software_led_state;
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED_PWM
    const uint8_t m_duty_cycle_table[PWM_DUTY_CYCLE_TABLE_SIZE] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xF9, 0xF7, 0xF6, 0xF3, 0xF1, 0xEF, 0xEC, 0xE9, 
        0xE7, 0xE3, 0xE0, 0xDD, 0xD9, 0xD6, 0xD2, 0xCE, 0xCA, 0xC5, 0xC1, 0xBD, 0xB8, 0xB4, 0xAF, 0xAA, 
        0xA5, 0xA0, 0x9B, 0x97, 0x91, 0x8C, 0x87, 0x82, 0x7D, 0x78, 0x73, 0x6E, 0x69, 0x64, 0x5F, 0x5A, 
        0x55, 0x51, 0x4C, 0x47, 0x43, 0x3E, 0x3A, 0x36, 0x32, 0x2E, 0x2A, 0x26, 0x23, 0x1F, 0x1C, 0x19, 
        0x16, 0x13, 0x10, 0x0E, 0x0C, 0x0A, 0x08, 0x06, 0x05, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x09, 0x0B, 0x0E, 0x10, 0x13, 0x15, 0x18, 
        0x1B, 0x1E, 0x22, 0x25, 0x29, 0x2D, 0x31, 0x35, 0x39, 0x3D, 0x42, 0x46, 0x4B, 0x50, 0x54, 0x59, 
        0x5E, 0x63, 0x68, 0x6D, 0x72, 0x77, 0x7C, 0x81, 0x86, 0x8B, 0x90, 0x95, 0x9A, 0x9F, 0xA4, 0xA9, 
        0xAE, 0xB3, 0xB7, 0xBC, 0xC0, 0xC5, 0xC9, 0xCD, 0xD1, 0xD5, 0xD8, 0xDC, 0xDF, 0xE3, 0xE6, 0xE9, 
        0xEC, 0xEE, 0xF1, 0xF3, 0xF5, 0xF7, 0xF9, 0xFA, 0xFC, 0xFD, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };
#endif
#endif

public:
    Axp192Pmu();
    ~Axp192Pmu();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);

public:
    esp_err_t enable_dcdc1(bool enable);
    esp_err_t enable_dcdc2(bool enable);
    esp_err_t enable_dcdc3(bool enable);
    esp_err_t enable_ldo2(bool enable);
    esp_err_t enable_ldo3(bool enable);
    esp_err_t enable_external_module(bool enable);

    esp_err_t set_dcdc1_voltage(uint16_t millivolt);
    esp_err_t set_dcdc2_voltage(uint16_t millivolt);
    esp_err_t set_dcdc3_voltage(uint16_t millivolt);
    esp_err_t set_ldo2_voltage(uint16_t millivolt);
    esp_err_t set_ldo3_voltage(uint16_t millivolt);

    esp_err_t software_enable_vbus(bool enable);
    esp_err_t set_voff_voltage(Axp192VoffVolt_t volt_index);
    esp_err_t power_off();

    esp_err_t set_charge_voltage(Axp192ChargeTargetVolt_t volt_index);
    esp_err_t set_charge_current(Axp192ChargingInterCurrent_t current_index);
    esp_err_t set_charge_stop_current(Axp192ChargeStopCur_t current_index);
    esp_err_t set_pre_charge_timeout(Axp192PreChargeTimeout_t timeout_index);
    esp_err_t set_charge_timeout(Axp192ChargeTimeout_t timeout_index);

    esp_err_t set_dcdc1_mode(Axp192DcdcMode_t mode);
    esp_err_t set_dcdc2_mode(Axp192DcdcMode_t mode);
    esp_err_t set_dcdc3_mode(Axp192DcdcMode_t mode);

    esp_err_t set_gpio0_mode(Axp192Gpio012Mode_t mode);
    esp_err_t set_gpio1_mode(Axp192Gpio012Mode_t mode);
    esp_err_t set_gpio2_mode(Axp192Gpio012Mode_t mode);
    esp_err_t set_gpio3_mode(Axp192Gpio34Mode_t mode);
    esp_err_t set_gpio4_mode(Axp192Gpio34Mode_t mode);

    esp_err_t set_gpio0_level(bool high);
    esp_err_t set_gpio1_level(bool high);
    esp_err_t set_gpio2_level(bool high);
    esp_err_t set_gpio3_level(bool high);
    esp_err_t set_gpio4_level(bool high);

    esp_err_t set_pwm1_init_param(uint8_t clock_factor, uint8_t duty_cycle_divisor);
    esp_err_t set_pwm1_duty_cycle(uint8_t duty_cycle);

private:
    Axp192ChargingInterCurrent_t calc_charging_current_index(uint32_t current_ma);
#if CONFIG_I2C_DEVICE_AXP192_CHARGING_SOFTWARE_LED
    esp_err_t software_led_loop();
#endif
};

extern Axp192Pmu *g_p_axp192_pmu;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t VibrateMotor();
esp_err_t SetScreenBrightness(uint8_t percent);
esp_err_t SystemPowerOff();
esp_err_t EnableBusPower(bool enable);
esp_err_t EnableSpeaker(bool enable);
esp_err_t ResetScreen();

#ifdef __cplusplus
}
#endif

/***********************************************************************************************************************
 * If the AXP192 module is not configured, all codes in this file will be ignored. 
 * Use a configuration tool such as menuconfig to configure it.
***********************************************************************************************************************/
#endif
/***********************************************************************************************************************
 * If the AXP192 module is not configured, all codes in this file will be ignored. 
 * Use a configuration tool such as menuconfig to configure it.
***********************************************************************************************************************/
