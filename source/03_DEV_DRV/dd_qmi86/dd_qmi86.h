
#ifndef DD_QMI86_H
#define DD_QMI86_H

#include "su_common.h"

typedef enum en_qmi86_int_output
{
    QMI86_INT_OUTPUT_INT2 = 0x00,
    QMI86_INT_OUTPUT_INT1 = 0x01,
} qmi86_int_output_t;

typedef enum en_qmi86_spi_mode
{
    QMI86_SPI_MODE_4WIRE = 0x00,
    QMI86_SPI_MODE_3WIRE = 0x01,
} qmi86_spi_mode_t;

typedef enum en_qmi86_data_format
{
    QMI86_DATA_FORMAT_LITTLE_ENDIAN = 0x00,
    QMI86_DATA_FORMAT_BIG_ENDIAN    = 0x01,
} qmi86_data_format_t;

typedef enum en_qmi86_acc_fsr
{
    QMI86_ACC_FSR_2G  = 0x00,
    QMI86_ACC_FSR_4G  = 0x01,
    QMI86_ACC_FSR_8G  = 0x02,
    QMI86_ACC_FSR_16G = 0x03
} qmi86_acc_fsr;

typedef enum en_qmi86_gyro_fsr
{
    QMI86_GYRO_FSR_16DPS   = 0x00,
    QMI86_GYRO_FSR_32DPS   = 0x01,
    QMI86_GYRO_FSR_64DPS   = 0x02,
    QMI86_GYRO_FSR_128DPS  = 0x03,
    QMI86_GYRO_FSR_256DPS  = 0x04,
    QMI86_GYRO_FSR_512DPS  = 0x05,
    QMI86_GYRO_FSR_1024DPS = 0x06,
    QMI86_GYRO_FSR_2048DPS = 0x07,
} qmi86_gyro_fsr;

typedef enum en_qmi86_acc_odr
{
    // Normal mode ODR options
    QMI86_ACC_ODR_1000_HZ  = 0x03,
    QMI86_ACC_ODR_500_HZ   = 0x04,
    QMI86_ACC_ODR_250_HZ   = 0x05,
    QMI86_ACC_ODR_125_HZ   = 0x06,
    QMI86_ACC_ODR_62P5_HZ  = 0x07,
    QMI86_ACC_ODR_31P25_HZ = 0x08,

    // Low Power mode ODR options
    QMI86_ACC_ODR_LP_128_HZ = 0x0C,
    QMI86_ACC_ODR_LP_21_HZ  = 0x0D,
    QMI86_ACC_ODR_LP_11_HZ  = 0x0E,
    QMI86_ACC_ODR_LP_3_HZ   = 0x0F,
} qmi86_acc_odr;

typedef enum en_qmi86_gyro_odr
{
    QMI86_GYRO_ODR_896P8_HZ  = 0x04,
    QMI86_GYRO_ODR_448P4_HZ  = 0x05,
    QMI86_GYRO_ODR_224P2_HZ  = 0x06,
    QMI86_GYRO_ODR_112P1_HZ  = 0x07,
    QMI86_GYRO_ODR_56P05_HZ  = 0x08,
    QMI86_GYRO_ODR_28P025_HZ = 0x09,
} qmi86_gyro_odr;

typedef enum en_qmi86_lpf_mode
{
    QMI86_LPF_MODE_2P66_PCT  = 0x00,
    QMI86_LPF_MODE_3P63_PCT  = 0x01,
    QMI86_LPF_MODE_5P39_PCT  = 0x02,
    QMI86_LPF_MODE_13P37_PCT = 0x03
} qmi86_lpf_mode;

typedef enum en_qmi86_sensor_mode
{
    QMI86_SENSOR_DISABLE       = 0x00,
    QMI86_SENSOR_MODE_ACC      = 0x01,
    QMI86_SENSOR_MODE_GYRO     = 0x02,
    QMI86_SENSOR_MODE_ACC_GYRO = QMI86_SENSOR_MODE_ACC | QMI86_SENSOR_MODE_GYRO,
} qmi86_sensor_mode;

typedef enum en_qmi86_gyro_mode
{
    QMI86_GYRO_MODE_FULL   = 0x00,
    QMI86_GYRO_MODE_SNOOZE = 0x01,
} qmi86_gyro_mode;

typedef enum en_qmi86_self_test_result
{
    QMI86_ST_RESULT_OK = 0x00,
    QMI86_ST_RESULT_FAILED,
    QMI86_ST_RESULT_TIMEOUT,
    QMI86_ST_RESULT_API_ERROR,
} qmi86_st_result;

typedef enum en_qmi86_sensors
{
    QMI86_SENSOR_GYRO = 0x00,
    QMI86_SENSOR_ACCEL,
    QMI86_SENSORS_CNT,
} qmi86_sensors_t;

typedef enum en_qmi86_settings_type
{
    QMI86_SETTINGS_INT = 0x00,
    QMI86_SETTINGS_COMM_IFC,
    QMI86_SETTINGS_DATA,
    QMI86_SETTINGS_ALL
} qmi86_settings_t;

union gyro_calib_result
{
    struct
    {
        uint8_t calib_failed             : 1;
        uint8_t gyro_was_enabled         : 1;
        uint8_t startup_failed           : 1;
        uint8_t device_vibrated          : 1;
        uint8_t y_axis_high_limit_failed : 1;
        uint8_t y_axis_low_limit_failed  : 1;
        uint8_t x_axis_high_limit_failed : 1;
        uint8_t x_axis_low_limit_failed  : 1;
    } err_flags;
    uint8_t result_code;
};

struct st_qmi86_int_settings
{
    bool_t             int1_enable;
    bool_t             int2_enable;
    bool_t             drdy_enable;
    qmi86_int_output_t fifo_int_output;
};

struct st_qmi86_ifc_settings
{
    qmi86_spi_mode_t spi_mode;
};

struct st_qmi86_data_settings
{
    qmi86_acc_fsr  acc_fsr;
    qmi86_acc_odr  acc_odr;
    bool_t         acc_lpf_en;
    qmi86_lpf_mode acc_lpf_mode;
    qmi86_gyro_fsr gyro_fsr;
    qmi86_gyro_odr gyro_odr;
    bool_t         gyro_lpf_en;
    qmi86_lpf_mode gyro_lpf_mode;
};

struct st_qmi86_settings
{
    struct st_qmi86_int_settings  interrupt_settings;
    struct st_qmi86_ifc_settings  comm_ifc_settings;
    struct st_qmi86_data_settings data_settings;
};

struct st_qmi86_gyro_calib
{
    int16_t x_gain;
    int16_t y_gain;
    int16_t z_gain;
};

struct st_qmi86_clibration_data
{
    struct st_qmi86_gyro_calib gyro_data;
};
struct st_qmi86_sensor_data
{
    float x;
    float y;
    float z;
};

struct st_qmi86_data
{
    struct st_qmi86_sensor_data gyro;
    struct st_qmi86_sensor_data accel;
};

typedef struct st_qmi86_dev
{
    union
    {
        uint32_t u32;
        uint8_t  u8_arr[4];
    } chip_fw_version;

    union
    {
        uint32_t u32;
        uint8_t  u8_arr[4];
    } chip_id;

    struct st_qmi86_settings        settings;
    struct st_qmi86_clibration_data clib_params;
    struct st_qmi86_data            data;
} qmi86_dev_t;

typedef enum en_qmi86_dev_id
{
    QMI86_DEV_1 = 0x00,
    QMI86_DEV_CNT,
} qmi86_dev_id_t;

response_status_t dd_qmi86_set_data_settings(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_set_interface_settings(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_set_interrupt_settings(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_set_device_mode(qmi86_dev_t* ppt_dev, qmi86_sensor_mode p_dev_mode);
response_status_t dd_qmi86_get_data_settings(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_get_interface_settings(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_get_interrupt_settings(qmi86_dev_t* ppt_dev);
qmi86_sensor_mode dd_qmi86_get_device_mode(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_calibrate_gyro(qmi86_dev_t*             ppt_dev,
                                          union gyro_calib_result* p_result_hndlr);
qmi86_st_result   dd_qmi86_perform_self_test(qmi86_dev_t* ppt_dev, qmi86_sensors_t p_sensor_type);
response_status_t dd_qmi86_reset_device(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_poll_data(qmi86_dev_t* ppt_dev);
response_status_t dd_qmi86_init(qmi86_dev_t** ppt_dev, qmi86_dev_id_t p_dev_id);
qmi86_dev_t*      dd_qmi86_get_dev(qmi86_dev_id_t p_dev_id);

#endif // DD_QMI86_H
