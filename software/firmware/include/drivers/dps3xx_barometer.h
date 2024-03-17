#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "bluethroat_msg_proc.h"
#include "drivers/task_param.h"
#include "utilities/low_pass_filter.h"
#include "drivers/i2c_device.h"
#include "utilities/sme_float.h"

/***********************************************************************************************************************
* Dps3xx coefficient scale factor defination
***********************************************************************************************************************/
#define DPS3XX_SCALE_FACTOR_PRC_1               (0x00080000)	//524288
#define DPS3XX_SCALE_FACTOR_PRC_2               (0x00180000)	//1572864
#define DPS3XX_SCALE_FACTOR_PRC_4               (0x00380000)	//3670016
#define DPS3XX_SCALE_FACTOR_PRC_8               (0x00780000)	//7864320
#define DPS3XX_SCALE_FACTOR_PRC_16              (0x0003E000)	//253952
#define DPS3XX_SCALE_FACTOR_PRC_32              (0x0007E000)	//516096
#define DPS3XX_SCALE_FACTOR_PRC_64              (0x000FE000)	//1040384
#define DPS3XX_SCALE_FACTOR_PRC_128             (0x001FE000)	//2088960

/***********************************************************************************************************************
* Dps3xx pressure and temperature data registers address and structure defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_PSR_B2                  (0x00)
#define DPS3XX_REG_ADDR_PSR_B1                  (0x01)
#define DPS3XX_REG_ADDR_PSR_B0                  (0x02)
#define DPS3XX_REG_ADDR_TMP_B2                  (0x03)
#define DPS3XX_REG_ADDR_TMP_B1                  (0x04)
#define DPS3XX_REG_ADDR_TMP_B0                  (0x05)

typedef union {
    uint8_t bytes[6];
    struct {
        uint8_t prs_b2;
        uint8_t prs_b1;
        uint8_t prs_b0;
        uint8_t tmp_b2;
        uint8_t tmp_b1;
        uint8_t tmp_b0;
    };
} __attribute__ ((packed)) Dps3xxData_t;

/***********************************************************************************************************************
* Dps3xx pressure configuration registers address, structure and related configuration value defination 
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_PRS_CFG                 (0x06)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t pm_prc          : 4;
        uint8_t pm_rate         : 3;
        uint8_t                 : 1;
#else
        uint8_t                 : 1;
        uint8_t pm_rate         : 3;
        uint8_t pm_prc          : 4;
#endif
    };
} __attribute__ ((packed)) Dps3xxPrsCfgReg_t;

#define DPS3XX_REG_VALUE_PM_PRC_1               (0x00)
#define DPS3XX_REG_VALUE_PM_PRC_2               (0x01)
#define DPS3XX_REG_VALUE_PM_PRC_4               (0x02)
#define DPS3XX_REG_VALUE_PM_PRC_8               (0x03)
#define DPS3XX_REG_VALUE_PM_PRC_16              (0x04)
#define DPS3XX_REG_VALUE_PM_PRC_32              (0x05)
#define DPS3XX_REG_VALUE_PM_PRC_64              (0x06)
#define DPS3XX_REG_VALUE_PM_PRC_128             (0x07)

#define DPS3XX_REG_VALUE_PM_RATE_1              (0x00)
#define DPS3XX_REG_VALUE_PM_RATE_2              (0x01)
#define DPS3XX_REG_VALUE_PM_RATE_4              (0x02)
#define DPS3XX_REG_VALUE_PM_RATE_8              (0x03)
#define DPS3XX_REG_VALUE_PM_RATE_16             (0x04)
#define DPS3XX_REG_VALUE_PM_RATE_32             (0x05)
#define DPS3XX_REG_VALUE_PM_RATE_64             (0x06)
#define DPS3XX_REG_VALUE_PM_RATE_128            (0x07)

/***********************************************************************************************************************
* Dps3xx temperature configuration registers address, structure and related configuration value defination 
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_TMP_CFG                 (0x07)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t tmp_prc         : 4;
        uint8_t tmp_rate        : 3;
        uint8_t tmp_ext         : 1;
#else
        uint8_t tmp_ext         : 1;
        uint8_t tmp_rate        : 3;
        uint8_t tmp_prc         : 4;
#endif
    };
} __attribute__ ((packed)) Dps3xxTmpCfgReg_t;

#define DPS3XX_REG_VALUE_TMP_PRC_1              (0x00)
#define DPS3XX_REG_VALUE_TMP_PRC_2              (0x01)
#define DPS3XX_REG_VALUE_TMP_PRC_4              (0x02)
#define DPS3XX_REG_VALUE_TMP_PRC_8              (0x03)
#define DPS3XX_REG_VALUE_TMP_PRC_16             (0x04)
#define DPS3XX_REG_VALUE_TMP_PRC_32             (0x05)
#define DPS3XX_REG_VALUE_TMP_PRC_64             (0x06)
#define DPS3XX_REG_VALUE_TMP_PRC_128            (0x07)

#define DPS3XX_REG_VALUE_TMP_RATE_1             (0x00)
#define DPS3XX_REG_VALUE_TMP_RATE_2             (0x01)
#define DPS3XX_REG_VALUE_TMP_RATE_4             (0x02)
#define DPS3XX_REG_VALUE_TMP_RATE_8             (0x03)
#define DPS3XX_REG_VALUE_TMP_RATE_16            (0x04)
#define DPS3XX_REG_VALUE_TMP_RATE_32            (0x05)
#define DPS3XX_REG_VALUE_TMP_RATE_64            (0x06)
#define DPS3XX_REG_VALUE_TMP_RATE_128           (0x07)

#define DPS3XX_REG_VALUE_TMP_EXT_ASIC           (0x00)
#define DPS3XX_REG_VALUE_TMP_EXT_MEMS           (0x01)

/***********************************************************************************************************************
* Dps3xx measurement configuration registers address, structure and related configuration value defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_MEAS_CFG                (0x08)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t meas_ctrl       : 3;
        uint8_t                 : 1;
        uint8_t prs_rdy         : 1;
        uint8_t tmp_rdy         : 1;
        uint8_t sensor_rdy      : 1;
        uint8_t coef_rdy        : 1;
#else
        uint8_t coef_rdy        : 1;
        uint8_t sensor_rdy      : 1;
        uint8_t tmp_rdy         : 1;
        uint8_t prs_rdy         : 1;
        uint8_t                 : 1;
        uint8_t meas_ctrl       : 3;
#endif
    };
} __attribute__ ((packed)) Dps3xxMeasCfgReg_t;

#define DPS3XX_REG_VALUE_MEAS_CTRL_STOP         (0x00)
#define DPS3XX_REG_VALUE_MEAS_CTRL_PRS          (0x01)
#define DPS3XX_REG_VALUE_MEAS_CTRL_TMP          (0x02)
#define DPS3XX_REG_VALUE_MEAS_CTRL_BG_PRS       (0x05)
#define DPS3XX_REG_VALUE_MEAS_CTRL_BG_TMP       (0x06)
#define DPS3XX_REG_VALUE_MEAS_CTRL_BG_ALL       (0x07)

// In order to improve efficiency, some state bits are defined separately here, from the perspective of the entire
// register, rather than from the perspective of a single bit field
#define DPS3XX_REG_VALUE_PRS_RDY                (0x10)
#define DPS3XX_REG_VALUE_TMP_RDY                (0x20)

/***********************************************************************************************************************
* Dps3xx global configuration registers address, structure and related configuration value defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_CFG_REG             (0x09)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t spi_mode        : 1;
        uint8_t fifo_en         : 1;
        uint8_t p_shift_en      : 1;
        uint8_t t_shift_en      : 1;
        uint8_t prs_int_en      : 1;
        uint8_t tmp_int_en      : 1;
        uint8_t fifo_int_en     : 1;
        uint8_t int_hl          : 1;
#else
        uint8_t int_hl          : 1;
        uint8_t fifo_int_en     : 1;
        uint8_t tmp_int_en      : 1;
        uint8_t prs_int_en      : 1;
        uint8_t t_shift_en      : 1;
        uint8_t p_shift_en      : 1;
        uint8_t fifo_en         : 1;
        uint8_t spi_mode        : 1;
#endif
    };
} __attribute__ ((packed)) Dps3xxCfgReg_t;

#define DPS3XX_REG_VALUE_SPI_MODE_4WIRE     (0x00)
#define DPS3XX_REG_VALUE_SPI_MODE_3WIRE     (0x01)

/***********************************************************************************************************************
* Dps3xx interrupt status registers address and structure defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_INT_STS             (0x0A)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t fifo_full       : 1;
        uint8_t tmp_rdy         : 1;
        uint8_t prs_rdy         : 1;
        uint8_t                 : 5;
#else
        uint8_t                 : 5;
        uint8_t prs_rdy         : 1;
        uint8_t tmp_rdy         : 1;
        uint8_t fifo_full       : 1;
#endif
    };
} __attribute__ ((packed)) Dps3xxIntStsReg_t;

/***********************************************************************************************************************
* Dps3xx FIFO status registers address and structure defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_FIFO_STS            (0x0B)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t fifo_full       : 1;
        uint8_t fifo_empty      : 1;
        uint8_t                 : 6;
#else
        uint8_t                 : 6;
        uint8_t fifo_empty      : 1;
        uint8_t fifo_full       : 1;
#endif
    };
} __attribute__ ((packed)) Dps3xxFifoStsReg_t;

/***********************************************************************************************************************
* Dps3xx soft reset and FIFO flush registers address and structure defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_RESET               (0x0C)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t soft_reset      : 4;
        uint8_t                 : 3;
        uint8_t fifo_flush      : 1;
#else
        uint8_t fifo_flush      : 1;
        uint8_t                 : 3;
        uint8_t soft_reset      : 4;
#endif
    };
} __attribute__ ((packed)) Dps3xxResetReg_t;

#define DPS3XX_REG_VALUE_SOFT_RESET         (0x09)
#define DPS3XX_REG_VALUE_FIFO_FLUSH         (0x01)

/***********************************************************************************************************************
* Dps3xx chip and revision ID registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_ID                  (0x0D)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t rev_id          : 4;
        uint8_t prod_id         : 4;
#else
        uint8_t prod_id         : 4;
        uint8_t rev_id          : 4;
#endif
    };
} __attribute__ ((packed)) Dps3xxIdReg_t;

#define DPS3XX_REG_VALUE_ID                 (0x10)

/***********************************************************************************************************************
* Dps3xx coefficient registers address and structure defination, the coefficient data is 18 bytes, from 0x10 to 0x21
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_COEF                (0x10)

typedef union {
    uint8_t bytes[18];
    struct {
        uint8_t c0h;
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t c1h             : 4;
        uint8_t c0l             : 4;
#else
        uint8_t c0l             : 4;
        uint8_t c1h             : 4;
#endif
        uint8_t c1l;
        uint8_t c00h;
        uint8_t c00m;
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t c10h            : 4;
        uint8_t c00l            : 4;
#else
        uint8_t c00l            : 4;
        uint8_t c10h            : 4;
#endif
        uint8_t c10m;
        uint8_t c10l;
        uint8_t c01h;
        uint8_t c01l;
        uint8_t c11h;
        uint8_t c11l;
        uint8_t c20h;
        uint8_t c20l;
        uint8_t c21h;
        uint8_t c21l;
        uint8_t c30h;
        uint8_t c30l;
    };
} __attribute__ ((packed)) Dps3xxCoefRegs_t;

/***********************************************************************************************************************
* Dps3xx temperature coefficient source registers address，structure and related configuration value defination
***********************************************************************************************************************/
#define DPS3XX_REG_ADDR_COEF_SRC            (0x28)

typedef union {
    uint8_t byte;
    struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t coef_src        : 1;
        uint8_t                 : 7;
#else
        uint8_t                 : 7;
        uint8_t coef_src        : 1;
#endif
    };
} __attribute__ ((packed)) Dps3xxCoefSrcReg_t;

#define DPS3XX_REG_VALUE_COEF_SRC_ASIC      (0x00)
#define DPS3XX_REG_VALUE_COEF_SRC_MEMS      (0x01)

/***********************************************************************************************************************
* Dps3xx pressure and temperature measurement configuration structure defination
* The pressure and temperature measurement configuration is used to set the measurement rate, oversampling rate and
* measurement time, and the scale factor for the pressure and temperature data calculation.
***********************************************************************************************************************/
typedef struct {
    uint8_t mesurement_rate;        /* pressure measurement rate, effect in background mode */
    uint8_t oversampling_rate;
    TickType_t mesurement_time;     /* single measurement time, in ticks, call delay to wait for result */
    float32_t scale_factor;
} Dps3xxMeasureConfig_t;

/***********************************************************************************************************************
* Dps3xx scaled coefficient data structure defination
***********************************************************************************************************************/
typedef struct {
    float32_t scaled_c0;
    float32_t scaled_c1;
    float32_t scaled_c00;
    float32_t scaled_c10;
    float32_t scaled_c01;
    float32_t scaled_c11;
    float32_t scaled_c20;
    float32_t scaled_c21;
    float32_t scaled_c30;
} Dps3xxScaledCoefData_t;

/***********************************************************************************************************************
* Default value and depth of FIR filter defination
***********************************************************************************************************************/
#define AIR_PRESSURE_DEFAULT_VALUE      (101325)
#define AIR_PRESSURE_DEFAULT_VALUE_MSB  (16)
#define FILTER_DEPTH_SHALLOW            (FILTER_DEPTH_POWER_08)
#define FILTER_DEPTH_DEEP               (FILTER_DEPTH_POWER_32)

/***********************************************************************************************************************
* @brief Dps3xx barometer class
* This class is used to control the Dps3xx barometer sensor, including the initialization, deinitialization, data
* fetching and calculation.
* The Dps3xx barometer sensor is a digital pressure sensor with a 24-bit ADC and a 24-bit temperature sensor.
* The sensor can be configured to measure the pressure and temperature in different modes, including single measurement
* mode, background measurement mode, and background measurement mode with FIFO and interrupt.
* Since the hardware doesn't connect the interrupt pin, the interrupt and FIFO mode are not used in this class. and the
* background measurement mode is not used to measure the pressure and temperature data.
***********************************************************************************************************************/
class Dps3xxBarometer : public I2cDevice {
public:
    Dps3xxMeasureConfig_t m_pressure_cfg;               /* pressure measurement config */
    Dps3xxMeasureConfig_t m_temperature_cfg;            /* temperature measurement config */
    Dps3xxScaledCoefData_t m_coef_data;                 /* scaled coefficient data */
    FirFilter<uint32_t, uint32_t> *m_p_shallow_filter;  /* FIR shallow filter for pressure data */
    FirFilter<uint32_t, uint32_t> *m_p_deep_filter;     /* FIR deep filter for pressure data */

public:
    Dps3xxBarometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    ~Dps3xxBarometer();

public:
    static esp_err_t CheckDeviceId(I2cMaster *p_i2c_master, uint16_t device_addr);

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t process_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);

private:
    esp_err_t get_coefs();
};
