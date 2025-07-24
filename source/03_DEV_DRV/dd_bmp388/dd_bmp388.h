
#ifndef DD_BMP388_H
#define DD_BMP388_H

#include "su_common.h"

typedef enum en_bmp388_oversampling
{
    BMP388_OVERSAMPLING_NONE = 0x00,
    BMP388_OVERSAMPLING_2X   = 0x01,
    BMP388_OVERSAMPLING_4X   = 0x02,
    BMP388_OVERSAMPLING_8X   = 0x03,
    BMP388_OVERSAMPLING_16X  = 0x04,
    BMP388_OVERSAMPLING_32X  = 0x05,
} bmp388_oversampling_t;

typedef enum en_bmp388_odr
{
    BMP388_ODR_200_HZ    = 0x00,
    BMP388_ODR_100_HZ    = 0x01,
    BMP388_ODR_50_HZ     = 0x02,
    BMP388_ODR_25_HZ     = 0x03,
    BMP388_ODR_12P5_HZ   = 0x04,
    BMP388_ODR_6P25_HZ   = 0x05,
    BMP388_ODR_3P1_HZ    = 0x06,
    BMP388_ODR_1P5_HZ    = 0x07,
    BMP388_ODR_0P78_HZ   = 0x08,
    BMP388_ODR_0P39_HZ   = 0x09,
    BMP388_ODR_0P20_HZ   = 0x0A,
    BMP388_ODR_0P10_HZ   = 0x0B,
    BMP388_ODR_0P05_HZ   = 0x0C,
    BMP388_ODR_0P02_HZ   = 0x0D,
    BMP388_ODR_0P01_HZ   = 0x0E,
    BMP388_ODR_0P006_HZ  = 0x0F,
    BMP388_ODR_0P003_HZ  = 0x10,
    BMP388_ODR_0P0015_HZ = 0x11,
} bmp388_odr_t;

typedef enum en_iir_coeff
{
    BMP388_IIR_DISABLE   = 0x00,
    BMP388_IIR_COEFF_1   = 0x01,
    BMP388_IIR_COEFF_3   = 0x02,
    BMP388_IIR_COEFF_7   = 0x03,
    BMP388_IIR_COEFF_15  = 0x04,
    BMP388_IIR_COEFF_31  = 0x05,
    BMP388_IIR_COEFF_63  = 0x06,
    BMP388_IIR_COEFF_127 = 0x07,
} bmp388_iir_coeff_t;

typedef enum en_bmp388_sensor_enable
{
    BMP388_SENS_DISABLE      = 0x00,
    BMP388_SENS_ENABLE_PRESS = 0x01,
    BMP388_SENS_ENABLE_TEMP  = 0x02,
    BMP388_SENS_ENABLE_ALL   = 0x03,
} bmp388_sensor_enable_t;

typedef enum en_bmp388_power_mode
{
    BMP388_POWER_MODE_SLEEP  = 0x00,
    BMP388_POWER_MODE_FORCED = 0x01,
    BMP388_POWER_MODE_NORMAL = 0x03,
} bmp388_power_mode_t;

typedef enum en_bmp388_ifc_spi_mode
{
    BMP388_IFC_SPI_MODE_3_WIRE = 0x00,
    BMP388_IFC_SPI_MODE_4_WIRE = 0x01,
} bmp388_ifc_spi_mode_t;

typedef enum en_bmp388_iic_wdt
{
    BMP388_IIC_WDT_DISABLE = 0X00,
    BMP388_IIC_WDT_1_25_MS = 0x01,
    BMP388_IIC_WDT_40_MS   = 0x03,
} bmp388_iic_wdt_t;

typedef enum en_bmp388_int_type
{
    BMP388_INT_TYPE_PP_LOW_NON_LATCHED = 0x00,
    BMP388_INT_TYPE_OD_LOW_NON_LATCHED,
    BMP388_INT_TYPE_PP_HIGH_NON_LATCHED,
    BMP388_INT_TYPE_OD_HIGH_NON_LATCHED,
    BMP388_INT_TYPE_PP_LOW_LATCHED,
    BMP388_INT_TYPE_OD_LOW_LATCHED,
    BMP388_INT_TYPE_PP_HIGH_LATCHED,
    BMP388_INT_TYPE_OD_HIGH_LATCHED,
} bmp388_int_type_t;

typedef enum en_bmp388_int_enable
{
    BMP388_INT_DISABLE_ALL       = 0x00,
    BMP388_INT_ENABLE_FWTM       = 0x01,
    BMP388_INT_ENABLE_FFULL      = 0x02,
    BMP388_INT_ENABLE_FWTM_FFULL = 0x03,
    BMP388_INT_ENABLE_DRDY       = 0x08,
    BMP388_INT_ENABLE_FWTM_DRDY  = 0x09,
    BMP388_INT_ENABLE_FFULL_DRDY = 0x0A,
    BMP388_INT_ENABLE_ALL        = 0x0B,
} bmp388_int_enable_t;

typedef enum en_bmp388_data_request
{
    BMP388_READ_PRESSURE   = 0x01,
    BMP388_READ_TEMP       = 0x02,
    BMP388_READ_PRESS_TEMP = (BMP388_READ_PRESSURE | BMP388_READ_TEMP),
    BMP388_READ_TIME       = 0x04,
    BMP388_READ_ALL        = (BMP388_READ_PRESS_TEMP | BMP388_READ_TIME),
} bmp388_data_request_t;

typedef enum en_bmp388_health
{
    BMP388_HEALTH_OK       = 0x00,
    BMP388_HEALTH_WARNING  = 0x01,
    BMP388_HEALTH_CRITICAL = 0x02,
} bmp388_health_t;

typedef enum en_bmp388_status
{
    BMP388_NO_ERROR      = 0x00,
    BMP388_ERROR_FATAL   = 0x01,
    BMP388_ERROR_CMD     = 0x02,
    BMP388_ERROR_CONFIG  = 0x04,
    BMP388_ERROR_API     = 0x10,
    BMP388_WAITING_TEMP  = 0x11,
    BMP388_WAITING_PRESS = 0x12,
    BMP388_WAITING_DATA  = 0x13,
} bmp388_status_t;

struct st_bmp388_data
{
    float           pressure;
    float           temperature;
    uint32_t        sensortime;
    bmp388_health_t pressure_health;
    bmp388_health_t temperature_health;
};

struct st_bmp388_data_settings
{
    bmp388_oversampling_t temp_oversampling;
    bmp388_oversampling_t press_oversampling;
    bmp388_odr_t          output_data_rate;
    bmp388_iir_coeff_t    iir_filter;
};

struct st_bmp388_dev_settings
{
    bmp388_sensor_enable_t sensor_enable;
    bmp388_power_mode_t    power_mode;
};

struct st_bmp388_ifc_settings
{
    bmp388_ifc_spi_mode_t spi_mode;
    bmp388_iic_wdt_t      iic_wdt;
};

struct st_bmp388_interrupt_settings
{
    bmp388_int_type_t   int_type;
    bmp388_int_enable_t int_enable;
};

struct st_bmp388_settings
{
    struct st_bmp388_data_settings      data_settings;
    struct st_bmp388_dev_settings       dev_settings;
    struct st_bmp388_ifc_settings       comm_ifc_settings;
    struct st_bmp388_interrupt_settings int_settings;
};

typedef struct st_bmp388_dev
{
    uint8_t                   chip_id;
    struct st_bmp388_data     data;
    struct st_bmp388_settings settings;
} bmp388_dev_t;

typedef enum en_bmp388_devices
{
    BMP388_DEV_1 = 0x00,
    BMP388_DEV_CNT,
} bmp388_devices_t;

response_status_t dd_bmp388_set_data_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_set_dev_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_set_ifc_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_set_interrupt_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_get_data_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_get_dev_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_get_ifc_settings(bmp388_dev_t* ppt_dev);
response_status_t dd_bmp388_get_interrupt_settings(bmp388_dev_t* ppt_dev);
bmp388_status_t   dd_bmp388_get_data(bmp388_dev_t* ppt_dev, bmp388_data_request_t p_data_req);
bmp388_status_t   dd_bmp388_get_error_state(bmp388_dev_t* ppt_dev);
bmp388_status_t   dd_bmp388_reset(bmp388_dev_t* ppt_dev);
bmp388_dev_t*     dd_bmp388_get_dev(bmp388_devices_t p_dev_id);
response_status_t dd_bmp388_init(bmp388_dev_t** ppt_dev, bmp388_devices_t p_dev_id);

#endif // DD_BMP388_H
