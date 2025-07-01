#ifndef DD_BMP388_DEFS_H
#define DD_BMP388_DEFS_H

#include "su_common.h"

/** @name BMP388 register addresses */

/** @brief chip identification code */
#define BMP388_REG_CHIP_ID        (0x00)

/** @brief 3Bit Sensor error conditions */
#define BMP388_REG_ERR            (0x02)

/** @brief 3Bit Sensor status flags */
#define BMP388_REG_SENS_STATUS    (0x03)
#define BMP388_REG_SENS_STATUS_CMD_POS    (0x04)
#define BMP388_REG_SENS_STATUS_CMD_MSK    (0x10)

#define BMP388_REG_SENS_STATUS_PRES_POS    (0x05)
#define BMP388_REG_SENS_STATUS_PRES_MSK    (0x20)

#define BMP388_REG_SENS_STATUS_TEMP_POS    (0x06)
#define BMP388_REG_SENS_STATUS_TEMP_MSK    (0x40)

/** @brief 24Bit pressure data
 * @note data is split into 3 registers.
 */
#define BMP388_REG_DATA_PRES      (0x04)
#define BMP388_REG_DATA_PRES_LEN      (3U)

/** @brief 24Bit temperature data
 * @note data is split into 3 registers.
 */
#define BMP388_REG_DATA_TEMP      (0x07)
#define BMP388_REG_DATA_TEMP_LEN      (3U)

/** @brief 24Bit sensor timestamp
 * @note data is split into 3 registers.
 */
#define BMP388_REG_SENS_TIME      (0x0C)
#define BMP388_REG_SENS_TIME_LEN  (3U)

/** @brief 1Bit sensor POR status */
#define BMP388_REG_EVENT          (0x10)

/** @brief 3Bit Interrupt status register
 * @note This register is cleared on read.
 */
#define BMP388_REG_INT_STATUS     (0x11)
#define BMP388_REG_INT_STATUS_FWM_POS     (0x00)
#define BMP388_REG_INT_STATUS_FWM_MSK     (0x01)

#define BMP388_REG_INT_STATUS_FFULL_POS     (0x01)
#define BMP388_REG_INT_STATUS_FFULL_MSK     (0x02)

#define BMP388_REG_INT_STATUS_DRDY_POS     (0x04)
#define BMP388_REG_INT_STATUS_DRDY_MSK     (0x10)

/** @brief 9Bit FIFO fill level
 * @note data is split into 2 registers.
 */
#define BMP388_REG_FIFO_LENGTH    (0x12)

/** @brief FIFO data output */
#define BMP388_REG_FIFO_DATA      (0x14)

/** @brief 9Bit FIFO watermark
 * @note data is split into 2 registers.
 */
#define BMP388_REG_FIFO_WM        (0x15)

/** @brief 5Bit FIFO frame content configuration */
#define BMP388_REG_FIFO_CONFIG_1  (0x17)
#define BMP388_REG_FIFO_CONFIG_1_MODE_POS  (0x00)
#define BMP388_REG_FIFO_CONFIG_1_MODE_MSK  (0x01)

#define BMP388_REG_FIFO_CONFIG_1_STOP_POS  (0x01)
#define BMP388_REG_FIFO_CONFIG_1_STOP_MSK  (0x02)

#define BMP388_REG_FIFO_CONFIG_1_TIME_POS  (0x02)
#define BMP388_REG_FIFO_CONFIG_1_TIME_MSK  (0x04)

#define BMP388_REG_FIFO_CONFIG_1_PRES_POS  (0x03)
#define BMP388_REG_FIFO_CONFIG_1_PRES_MSK  (0x08)

#define BMP388_REG_FIFO_CONFIG_1_TEMP_POS  (0x04)
#define BMP388_REG_FIFO_CONFIG_1_TEMP_MSK  (0x10)

/** @brief 5Bit FIFO frame content configuration */
#define BMP388_REG_FIFO_CONFIG_2  (0x18)
#define BMP388_REG_FIFO_CONFIG_2_SS_POS  (0x00)
#define BMP388_REG_FIFO_CONFIG_2_SS_MSK  (0x07)

#define BMP388_REG_FIFO_CONFIG_2_DATA_POS  (0x03)
#define BMP388_REG_FIFO_CONFIG_2_DATA_MSK  (0x18)

/** @brief 6Bit Interrupt configuration
 * @note This register affects INT_STATUS reg and INT pin.
 */
#define BMP388_REG_INT_CTRL       (0x19)
#define BMP388_REG_INT_CTRL_TYPE_POS       (0x00)
#define BMP388_REG_INT_CTRL_TYPE_MSK       (0x07)

#define BMP388_REG_INT_CTRL_EN_POS       (0x03)
#define BMP388_REG_INT_CTRL_EN_MSK       (0x58)

/** @brief 3Bit Serial interface settings */
#define BMP388_REG_IF_CONF        (0x1A)
#define BMP388_REG_IF_CONF_SPI_POS        (0x00)
#define BMP388_REG_IF_CONF_SPI_MSK        (0x01)

#define BMP388_REG_IF_CONF_WDT_POS        (0x01)
#define BMP388_REG_IF_CONF_WDT_MSK        (0x06)

/** @brief 4Bit sensor enable and mode settings */
#define BMP388_REG_PWR_CTRL                (0x1B)
#define BMP388_REG_PWR_CTRL_EN_POS       (0x00)
#define BMP388_REG_PWR_CTRL_EN_MSK       (0x03)

#define BMP388_REG_PWR_CTRL_MODE_POS       (0x04)
#define BMP388_REG_PWR_CTRL_MODE_MSK       (0x30)

/** @brief 6Bit oversampling settings for pressure and temperature */
#define BMP388_REG_OSR                     (0X1C)
#define BMP388_REG_OSR_PRES_POS            (0X00)
#define BMP388_REG_OSR_PRES_MSK            (0X07)

#define BMP388_REG_OSR_TEMP_POS            (0X03)
#define BMP388_REG_OSR_TEMP_MSK            (0X38)

/** @brief 5Bit output data rate settings
 * @note Allowd values are 0 to 17
 */
#define BMP388_REG_ODR            (0x1D)
#define BMP388_REG_ODR_POS            (0x00)
#define BMP388_REG_ODR_MSK            (0x1F)

/** @brief 3Bit IIR filter coefficient selection */
#define BMP388_REG_CONFIG         (0x1F)
#define BMP388_REG_CONFIG_POS         (0x01)
#define BMP388_REG_CONFIG_MSK         (0x0E)

/** @brief  168Bit calibration data */
#define BMP388_REG_CALIB_DATA     (0x31)
#define BMP388_REG_CALIB_DATA_LEN (21U)

/** @brief Calibration coefficients calculated from the datasheet
 * @note These coefficients are used for temperature and pressure compensation.
 */
#define BMP388_CALIB_COEFF_T1 (0.0039062500f)
#define BMP388_CALIB_COEFF_T2 (1073741824.0f)
#define BMP388_CALIB_COEFF_T3 (281474976710656.0f)
#define BMP388_CALIB_COEFF_P1 (1048576.0f)
#define BMP388_CALIB_COEFF_P2 (536870912.0f)
#define BMP388_CALIB_COEFF_P3 (4294967296.0f)
#define BMP388_CALIB_COEFF_P4 (137438953472.0f)
#define BMP388_CALIB_COEFF_P5 (0.125000f)
#define BMP388_CALIB_COEFF_P6 (64.0f)
#define BMP388_CALIB_COEFF_P7 (256.0f)
#define BMP388_CALIB_COEFF_P8 (32768.0f)
#define BMP388_CALIB_COEFF_P9 (281474976710656.0f)
#define BMP388_CALIB_COEFF_P10 (281474976710656.0f)
#define BMP388_CALIB_COEFF_P11 (36893488147419100000.0f)

/** @brief Command register */
#define BMP388_REG_CMD            (0x7E)

#define BMP388_CHIP_ID (0x50)
#define BMP388_IIC_ADDR_1 (0x76) // Default I2C address for BMP388
#define BMP388_IIC_ADDR_2 (0x77) // Alternate I2C address for BMP388

#define BMP388_MIN_TEMP (-40.f)
#define BMP388_MAX_TEMP (85.f)

#define BMP388_MIN_PRES (30000.f)
#define BMP388_MAX_PRES (125000.f)

typedef float meas_data_t;

struct st_bmp388_calib_data
{
    meas_data_t nvm_par_t1;
    meas_data_t nvm_par_t2;
    meas_data_t nvm_par_t3;
    meas_data_t nvm_par_p1;
    meas_data_t nvm_par_p2;
    meas_data_t nvm_par_p3;
    meas_data_t nvm_par_p4;
    meas_data_t nvm_par_p5;
    meas_data_t nvm_par_p6;
    meas_data_t nvm_par_p7;
    meas_data_t nvm_par_p8;
    meas_data_t nvm_par_p9;
    meas_data_t nvm_par_p10;
    meas_data_t nvm_par_p11;
    meas_data_t t_lin;
};

struct st_bmp388_raw_data
{
    uint32_t pressure;
    uint32_t temperature;
    uint32_t sensortime;
};
#endif // DD_BMP388_DEFS_H
