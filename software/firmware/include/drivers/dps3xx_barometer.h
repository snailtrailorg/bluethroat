#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "bluethroat_msg_proc.h"
#include "drivers/task_param.h"
#include "utilities/low_pass_filter.h"
#include "drivers/i2c_device.h"
#include "utilities/sme_float.h"

#define DPS3XX_REG_PSR_B2               (0x00)
#define DPS3XX_REG_PSR_B1               (0x01)
#define DPS3XX_REG_PSR_B0               (0x02)

#define DPS3XX_REG_TMP_B2               (0x03)
#define DPS3XX_REG_TMP_B1               (0x04)
#define DPS3XX_REG_TMP_B0               (0x05)

#define DPS3XX_REG_PRS_CFG              (0x06)
#define DPS3XX_REG_TMP_CFG              (0x07)
#define DPS3XX_REG_MEAS_CFG             (0x08)
#define DPS3XX_REG_CFG_REG              (0x09)
#define DPS3XX_REG_INT_STS              (0x0A)
#define DPS3XX_REG_FIFO_STS             (0x0B)
#define DPS3XX_REG_RESET                (0x0C)
#define DPS3XX_REG_PRO_ID               (0x0D)

#define DPS3XX_REG_COEF_C0M             (0x10)
#define DPS3XX_REG_COEF_C0L_C1M         (0x11)
#define DPS3XX_REG_COEF_C1L             (0x12)
#define DPS3XX_REG_COEF_C00U            (0x13)
#define DPS3XX_REG_COEF_C00M            (0x14)
#define DPS3XX_REG_COEF_C00L_C10U       (0x15)
#define DPS3XX_REG_COEF_C10M            (0x16)
#define DPS3XX_REG_COEF_C10L            (0x17)
#define DPS3XX_REG_COEF_C01M            (0x18)
#define DPS3XX_REG_COEF_C01L            (0x19)
#define DPS3XX_REG_COEF_C11M            (0x1A)
#define DPS3XX_REG_COEF_C11L            (0x1B)
#define DPS3XX_REG_COEF_C20M            (0x1C)
#define DPS3XX_REG_COEF_C20L            (0x1D)
#define DPS3XX_REG_COEF_C21M            (0x1E)
#define DPS3XX_REG_COEF_C21L            (0x1F)
#define DPS3XX_REG_COEF_C30M            (0x20)
#define DPS3XX_REG_COEF_C30L            (0x21)

#define DPS3XX_REG_COEF_SRCE            (0x28)

// Pressure measurement rate
#define PRS_CFG_PM_RATE_1               (0x00)
#define PRS_CFG_PM_RATE_2               (0x10)
#define PRS_CFG_PM_RATE_4               (0x20)
#define PRS_CFG_PM_RATE_8               (0x30)
#define PRS_CFG_PM_RATE_16              (0x40)
#define PRS_CFG_PM_RATE_32              (0x50)
#define PRS_CFG_PM_RATE_64              (0x60)
#define PRS_CFG_PM_RATE_128             (0x70)

// Pressure oversampling rate
#define PRS_CFG_PM_PRC_1                (0x00)
#define PRS_CFG_PM_PRC_2                (0x01)
#define PRS_CFG_PM_PRC_4                (0x02)
#define PRS_CFG_PM_PRC_8                (0x03)
#define PRS_CFG_PM_PRC_16               (0x04)
#define PRS_CFG_PM_PRC_32               (0x05)
#define PRS_CFG_PM_PRC_64               (0x06)
#define PRS_CFG_PM_PRC_128              (0x07)

// Temperature measurement source
#define TMP_CFG_TMP_EXT_ASIC            (0x00)
#define TMP_CFG_TMP_EXT_MEMS            (0x80)
#define TMP_CFG_TMP_EXT_MASK            (0x80)

// Temperature measurement rate
#define TMP_CFG_TMP_RATE_1              (0x00)
#define TMP_CFG_TMP_RATE_2              (0x10)
#define TMP_CFG_TMP_RATE_4              (0x20)
#define TMP_CFG_TMP_RATE_8              (0x30)
#define TMP_CFG_TMP_RATE_16             (0x40)
#define TMP_CFG_TMP_RATE_32             (0x50)
#define TMP_CFG_TMP_RATE_64             (0x60)
#define TMP_CFG_TMP_RATE_128            (0x70)

// Temperature oversampling rate
#define TMP_CFG_TMP_PRC_1               (0x00)
#define TMP_CFG_TMP_PRC_2               (0x01)
#define TMP_CFG_TMP_PRC_4               (0x02)
#define TMP_CFG_TMP_PRC_8               (0x03)
#define TMP_CFG_TMP_PRC_16              (0x04)
#define TMP_CFG_TMP_PRC_32              (0x05)
#define TMP_CFG_TMP_PRC_64              (0x06)
#define TMP_CFG_TMP_PRC_128             (0x07)

// Sensor Operating Status
#define MEAS_CFG_COEF_RDY               (0x80)
#define MEAS_CFG_SENSOR_RDY             (0x40)
#define MEAS_CFG_TMP_RDY                (0x20)
#define MEAS_CFG_PRS_RDY                (0x10)
#define MEAS_CFG_PRS_RDY_MASK           (0x10)

// Sensor Operating Mode
#define MEAS_CFG_MEAS_CTRL_STOP         (0x00)
#define MEAS_CFG_MEAS_CTRL_PRS          (0x01)
#define MEAS_CFG_MEAS_CTRL_TMP          (0x02)
#define MEAS_CFG_MEAS_CTRL_BACKGROUND   (0x04)

// Interrupt and FIFO configuration
#define CFG_REG_INT_HL                  (0x80)
#define CFG_REG_INT_FIFO                (0x40)
#define CFG_REG_INT_TMP                 (0x20)
#define CFG_REG_INT_PRS                 (0x10)
#define CFG_REG_T_SHIFT                 (0x08)
#define CFG_REG_P_SHIFT                 (0x04)
#define CFG_REG_FIFO_EN                 (0x02)
#define CFG_REG_SPI_MODE_3WIRE          (0x01)

// Interrupt status
#define INT_STS_FIFO_FULL               (0x04)
#define INT_STS_TMP_READY               (0x02)
#define INT_STS_PRS_READY               (0x01)

// FIFO status
#define FIFO_STS_FIFO_FULL              (0x02)
#define FIFO_STS_FIFO_EMPTY             (0x01)

// Flush FIFO or generate software reset
#define RESET_FIFO_FLUSH_CM             (0x80)
#define RESET_RESET_CMD                 (0x05)

// Temperature Coeicients Source
#define TMP_COEF_SRCE_ASIA              (0x00)
#define TMP_COEF_SRCE_MEMS              (0x80)
#define TMP_COEF_SRCE_MASK              (0x80)

// Chip and revision ID
#define CHIP_AND_REVISION_ID            (0x10)

//Compensation Scale Factors
#define SCALE_FACTOR_PRC_1              (0x00080000)	//524288
#define SCALE_FACTOR_PRC_2              (0x00180000)	//1572864
#define SCALE_FACTOR_PRC_4              (0x00380000)	//3670016
#define SCALE_FACTOR_PRC_8              (0x00780000)	//7864320
#define SCALE_FACTOR_PRC_16             (0x0003E000)	//253952
#define SCALE_FACTOR_PRC_32             (0x0007E000)	//516096
#define SCALE_FACTOR_PRC_64             (0x000FE000)	//1040384
#define SCALE_FACTOR_PRC_128            (0x001FE000)	//2088960

typedef struct {
    float32_t c0;
    float32_t c1;
    float32_t c00;
    float32_t c10;
    float32_t c01;
    float32_t c11;
    float32_t c20;
    float32_t c21;
    float32_t c30;
} Dps3xxCoes_t;

#define AIR_PRESSURE_DEFAULT_VALUE  (101325)

class Dps3xxBarometer : public I2cDevice {
public:
    /* Scaled coefs */
    Dps3xxCoes_t m_coes_sc;
    FirFilter<uint32_t, uint32_t> *m_p_fir_filter;

public:
    Dps3xxBarometer(I2cMaster *p_i2c_master, uint16_t device_addr, const TaskParam_t *p_task_param, QueueHandle_t queue_handle);
    ~Dps3xxBarometer();

public:
    virtual esp_err_t init_device();
    virtual esp_err_t deinit_device();
    virtual esp_err_t fetch_data(uint8_t *data, uint8_t size);
    virtual esp_err_t calculate_data(uint8_t *in_data, uint8_t in_size, BluethroatMsg_t *p_message);
};
