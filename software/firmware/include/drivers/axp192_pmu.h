#pragma once

#include "drivers/i2c_device.h"

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
#define AXP192_REG_ADDR_CHARGE_CTRL             (0x33)

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
    AXP192_REG_VALUE_CHARGE_IN_CUR_100MA = 0x00,
    AXP192_REG_VALUE_CHARGE_IN_CUR_190MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_280MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_360MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_450MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_550MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_630MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_700MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_780MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_880MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_960MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_1000MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_1080MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_1160MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_1240MA,
    AXP192_REG_VALUE_CHARGE_IN_CUR_1320MA,
} Axp192ChargeInCur_t;

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
 * Axp192 spare battery charge control registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define AXP192_REG_ADDR_SPARE_BAT_CHARGE_CTRL   (0x34)

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
* @brief Axp192 PMU class
***********************************************************************************************************************/
class Axp192Pmu : public I2cDevice {
public:
    Axp192Pmu();
    ~Axp192Pmu();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);

public:
    bool is_charging();

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
    esp_err_t set_charge_current(Axp192ChargeInCur_t current_index);
    esp_err_t set_charge_stop_current(Axp192ChargeStopCur_t current_index);

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
};

extern Axp192Pmu *g_p_axp192_pmu;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t EnableVibrationMotor(bool enable);
esp_err_t SetScreenBrightness(uint8_t percent);
esp_err_t SetVolumn(uint8_t percent);
esp_err_t SystemPowerOff();
esp_err_t LightSystemLed(bool light);
esp_err_t EnableBusPower(bool enable);
esp_err_t EnableSpeaker(bool enable);
esp_err_t ResetScreen();

#ifdef __cplusplus
}
#endif