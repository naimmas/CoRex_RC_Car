
#include "dd_bmp388.h"

#include "dd_bmp388_defs.h"
#include "ha_iic/ha_iic.h"
#include "ps_timer/ps_timer.h"
#include "string.h"
#include "su_common.h"

#define DEFAULT_IIC_TIMEOUT (100U) // Default I2C timeout in milliseconds
#define DEFAULT_IIC_REG_SZ (1U)

#define BMP3_SET_BITS(bitname, data) ((data) << (bitname##_POS))

#define BMP3_GET_BITS(reg_data, bitname) (((reg_data) & (bitname##_MSK)) >> (bitname##_POS))

IIC_SETUP_PORT_CONNECTION(BMP388_DEV_CNT,
                          IIC_DEFINE_CONNECTION(IIC_PORT1, BMP388_DEV_1, BMP388_IIC_ADDR_1))

typedef struct st_driver
{
    bmp388_dev_t                dev;
    struct st_bmp388_calib_data calib_data;
    struct st_bmp388_raw_data   raw_data;
    uint8_t                     dev_id;
    bool_t                      is_initialized;
} driver_t;

typedef enum
{
    BMP388_CMD_EXT_MODE_EN = 0x34,
    BMP388_CMD_FIFO_FLUSH  = 0xB0,
    BMP388_CMD_SOFT_RESET  = 0xB6,
} bmp388_cmds;

static driver_t g_driver[BMP388_DEV_CNT];

/**
 * @brief This internal function writes data to a specific register of the
 * BMP388 device over I2C.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @param[in] ppt_data Pointer to the data buffer to write.
 * @param[in] p_data_sz Size of the data to write in bytes.
 * @param[in] p_reg_addr The register address to write to in the sensor.
 * @return Result of the execution status.
 */
static response_status_t write_register(bmp388_dev_t* ppt_dev, uint8_t* ppt_data, size_t p_data_sz,
                                        uint8_t p_reg_addr)
{
    response_status_t ret_val        = RET_OK;
    driver_t*         pt_curr_driver = (driver_t*)ppt_dev;

    ret_val = ha_iic_master_mem_write(IIC_GET_DEV_PORT(pt_curr_driver->dev_id),
                                      IIC_GET_DEV_ADDRESS(pt_curr_driver->dev_id),
                                      ppt_data,
                                      p_data_sz,
                                      p_reg_addr,
                                      HW_IIC_MEM_SZ_8BIT,
                                      DEFAULT_IIC_TIMEOUT);

    return ret_val;
}

/**
 * @brief This internal function reads data from a specific register of the
 * BMP388 device over I2C.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @param[out] ppt_data Pointer to the data buffer to read into.
 * @param[in] p_data_sz Size of the data to read in bytes.
 * @param[in] p_reg_addr The register address to read from in the sensor.
 * @return Result of the execution status.
 */
static response_status_t read_register(bmp388_dev_t* ppt_dev, uint8_t* ppt_data, size_t p_data_sz,
                                       uint8_t p_reg_addr)
{
    response_status_t ret_val        = RET_OK;
    driver_t*         pt_curr_driver = (driver_t*)ppt_dev;

    ret_val = ha_iic_master_mem_read(IIC_GET_DEV_PORT(pt_curr_driver->dev_id),
                                     IIC_GET_DEV_ADDRESS(pt_curr_driver->dev_id),
                                     ppt_data,
                                     p_data_sz,
                                     p_reg_addr,
                                     HW_IIC_MEM_SZ_8BIT,
                                     DEFAULT_IIC_TIMEOUT);

    return ret_val;
}

/**
 * @brief This internal function reads the calibration data from the BMP388
 * device and assigns it to `driver_t.calib_data`.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
static response_status_t read_calib_data(bmp388_dev_t* ppt_dev)
{
    response_status_t ret_val                             = RET_OK;
    driver_t*         pt_curr_driver                      = (driver_t*)ppt_dev;
    meas_data_t       temp_val                            = 0.0F;
    uint8_t           reg_data[BMP388_REG_CALIB_DATA_LEN] = { 0U };

    ret_val = read_register(&pt_curr_driver->dev,
                            reg_data,
                            BMP388_REG_CALIB_DATA_LEN,
                            BMP388_REG_CALIB_DATA);

    // NOLINTBEGIN
    if (ret_val == RET_OK)
    {
        temp_val = (meas_data_t)(BYTES_TO_WORD(unsigned, reg_data[0U], reg_data[1U]));
        pt_curr_driver->calib_data.nvm_par_t1 = temp_val / BMP388_CALIB_COEFF_T1;
        temp_val = (meas_data_t)(BYTES_TO_WORD(unsigned, reg_data[2U], reg_data[3U]));
        pt_curr_driver->calib_data.nvm_par_t2 = temp_val / BMP388_CALIB_COEFF_T2;
        temp_val                              = (meas_data_t)((int8_t)reg_data[4U]);
        pt_curr_driver->calib_data.nvm_par_t3 = temp_val / BMP388_CALIB_COEFF_T3;
        temp_val = (meas_data_t)(BYTES_TO_WORD(signed, reg_data[5U], reg_data[6U]));
        pt_curr_driver->calib_data.nvm_par_p1 =
          (temp_val - (meas_data_t)(1 << 14)) / BMP388_CALIB_COEFF_P1;
        temp_val = (meas_data_t)(BYTES_TO_WORD(signed, reg_data[7U], reg_data[8U]));
        pt_curr_driver->calib_data.nvm_par_p2 =
          (temp_val - (meas_data_t)(1 << 14)) / BMP388_CALIB_COEFF_P2;
        temp_val                              = (meas_data_t)((int8_t)reg_data[9U]);
        pt_curr_driver->calib_data.nvm_par_p3 = temp_val / BMP388_CALIB_COEFF_P3;
        temp_val                              = (meas_data_t)((int8_t)reg_data[10U]);
        pt_curr_driver->calib_data.nvm_par_p4 = temp_val / BMP388_CALIB_COEFF_P4;
        temp_val = (meas_data_t)(BYTES_TO_WORD(unsigned, reg_data[11U], reg_data[12U]));
        pt_curr_driver->calib_data.nvm_par_p5 = temp_val / BMP388_CALIB_COEFF_P5;
        temp_val = (meas_data_t)(BYTES_TO_WORD(unsigned, reg_data[13U], reg_data[14U]));
        pt_curr_driver->calib_data.nvm_par_p6 = temp_val / BMP388_CALIB_COEFF_P6;
        temp_val                              = (meas_data_t)((int8_t)reg_data[15U]);
        pt_curr_driver->calib_data.nvm_par_p7 = temp_val / BMP388_CALIB_COEFF_P7;
        temp_val                              = (meas_data_t)((int8_t)reg_data[16U]);
        pt_curr_driver->calib_data.nvm_par_p8 = temp_val / BMP388_CALIB_COEFF_P8;
        temp_val = (meas_data_t)(BYTES_TO_WORD(signed, reg_data[17U], reg_data[18U]));
        pt_curr_driver->calib_data.nvm_par_p9  = temp_val / BMP388_CALIB_COEFF_P9;
        temp_val                               = (meas_data_t)((int8_t)reg_data[19U]);
        pt_curr_driver->calib_data.nvm_par_p10 = temp_val / BMP388_CALIB_COEFF_P10;
        temp_val                               = (meas_data_t)((int8_t)reg_data[20U]);
        pt_curr_driver->calib_data.nvm_par_p11 = temp_val / BMP388_CALIB_COEFF_P11;
        pt_curr_driver->calib_data.t_lin       = 0.0f;
    }
    // NOLINTEND
    else
    {
        // Nothing to do.
    }

    return ret_val;
}

/**
 * @brief This internal function calculate the compensated pressure and updates
 * the `bmp388_dev_t.data.pressure` field.
 * @param[in,out] ppt_dev BMP388 device instance.
 */
static void compensate_pressure(bmp388_dev_t* ppt_dev)
{
    struct st_bmp388_calib_data* pt_calib_data = &((driver_t*)ppt_dev)->calib_data;
    meas_data_t                  raw_pres = (meas_data_t)(((driver_t*)ppt_dev)->raw_data.pressure);

    meas_data_t partial_data1 = (meas_data_t)0;
    meas_data_t partial_data2 = (meas_data_t)0;
    meas_data_t partial_data3 = (meas_data_t)0;
    meas_data_t partial_data4 = (meas_data_t)0;
    meas_data_t partial_out1  = (meas_data_t)0;
    meas_data_t partial_out2  = (meas_data_t)0;

    partial_data1 = pt_calib_data->nvm_par_p6 * pt_calib_data->t_lin;
    partial_data2 = pt_calib_data->nvm_par_p7 * (pt_calib_data->t_lin * pt_calib_data->t_lin);
    partial_data3 = pt_calib_data->nvm_par_p8
                    * (pt_calib_data->t_lin * pt_calib_data->t_lin * pt_calib_data->t_lin);
    partial_out1 = pt_calib_data->nvm_par_p5 + partial_data1 + partial_data2 + partial_data3;

    partial_data1 = pt_calib_data->nvm_par_p2 * pt_calib_data->t_lin;
    partial_data2 = pt_calib_data->nvm_par_p3 * (pt_calib_data->t_lin * pt_calib_data->t_lin);
    partial_data3 = pt_calib_data->nvm_par_p4
                    * (pt_calib_data->t_lin * pt_calib_data->t_lin * pt_calib_data->t_lin);
    partial_out2 =
      raw_pres * (pt_calib_data->nvm_par_p1 + partial_data1 + partial_data2 + partial_data3);

    partial_data1 = raw_pres * raw_pres;
    partial_data2 = pt_calib_data->nvm_par_p9 + pt_calib_data->nvm_par_p10 * pt_calib_data->t_lin;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + (raw_pres * raw_pres * raw_pres) * pt_calib_data->nvm_par_p11;

    ppt_dev->data.pressure = partial_out1 + partial_out2 + partial_data4;
    if (ppt_dev->data.pressure < BMP388_MIN_PRES)
    {
        ppt_dev->data.pressure        = BMP388_MIN_PRES;
        ppt_dev->data.pressure_health = BMP388_HEALTH_CRITICAL;
    }

    else if (ppt_dev->data.pressure > BMP388_MAX_PRES)
    {
        ppt_dev->data.pressure        = BMP388_MAX_PRES;
        ppt_dev->data.pressure_health = BMP388_HEALTH_CRITICAL;
    }
    else
    {
        ppt_dev->data.pressure_health = BMP388_HEALTH_OK;
    }
}

/**
 * @brief This internal function calculate the compensated temperature and
 * updates the `bmp388_dev_t.data.temperature` field.
 * @param[in,out] ppt_dev BMP388 device instance.
 */
static void compensate_temperature(bmp388_dev_t* ppt_dev)
{
    driver_t*   pt_curr_driver = (driver_t*)ppt_dev;
    meas_data_t uncomp_temp    = (meas_data_t)pt_curr_driver->raw_data.temperature;
    meas_data_t partial_data1  = (meas_data_t)0;
    meas_data_t partial_data2  = (meas_data_t)0;

    partial_data1 = (meas_data_t)(uncomp_temp - pt_curr_driver->calib_data.nvm_par_t1);
    partial_data2 = (meas_data_t)(partial_data1 * pt_curr_driver->calib_data.nvm_par_t2);
    pt_curr_driver->calib_data.t_lin =
      partial_data2 + (partial_data1 * partial_data1) * pt_curr_driver->calib_data.nvm_par_t3;

    if (pt_curr_driver->calib_data.t_lin < BMP388_MIN_TEMP)
    {
        pt_curr_driver->calib_data.t_lin            = BMP388_MIN_TEMP;
        pt_curr_driver->dev.data.temperature_health = BMP388_HEALTH_CRITICAL;
    }
    else if (pt_curr_driver->calib_data.t_lin > BMP388_MAX_TEMP)
    {
        pt_curr_driver->calib_data.t_lin            = BMP388_MAX_TEMP;
        pt_curr_driver->dev.data.temperature_health = BMP388_HEALTH_CRITICAL;
    }
    else
    {
        pt_curr_driver->dev.data.temperature_health = BMP388_HEALTH_OK;
    }

    pt_curr_driver->dev.data.temperature = pt_curr_driver->calib_data.t_lin;
}

/**
 * @brief This internal function checks if the BMP388 device is ready by trying
 * to read its chip ID.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return TRUE if the device is ready, FALSE otherwise.
 */
static bool_t is_dev_ready(bmp388_dev_t* ppt_dev)
{
    response_status_t ret_val = RET_OK;

    ret_val = read_register(ppt_dev, &ppt_dev->chip_id, DEFAULT_IIC_REG_SZ, BMP388_REG_CHIP_ID);

    return ((ret_val == RET_OK) && (ppt_dev->chip_id == BMP388_CHIP_ID));
}

/**
 * @brief This internal function retrieves the data state of the sensor.
 * It checks if the temperature and pressure data are ready.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @param[out] ppt_tmp_rdy Pointer to store the temperature readiness status.
 * @param[out] ppt_pres_rdy Pointer to store the pressure readiness status.
 * @return Result of the execution status.
 */
static response_status_t get_data_state(bmp388_dev_t* ppt_dev, bool_t* ppt_tmp_rdy,
                                        bool_t* ppt_pres_rdy)
{
    uint8_t           sens_status = 0U;
    response_status_t api_ret_val =
      read_register(ppt_dev, &sens_status, DEFAULT_IIC_REG_SZ, BMP388_REG_SENS_STATUS);
    if (api_ret_val == RET_OK)
    {
        *ppt_tmp_rdy  = BMP3_GET_BITS(sens_status, BMP388_REG_SENS_STATUS_TEMP);
        *ppt_pres_rdy = BMP3_GET_BITS(sens_status, BMP388_REG_SENS_STATUS_PRES);
    }
    return api_ret_val;
}

/**
 * @brief This internal function reads the sensor time from the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
static bmp388_status_t read_sens_time(bmp388_dev_t* ppt_dev)
{
    response_status_t api_ret_val                             = RET_OK;
    bmp388_status_t   ret_val                                 = BMP388_NO_ERROR;
    uint8_t           time_reg_data[BMP388_REG_SENS_TIME_LEN] = { 0U };

    api_ret_val =
      read_register(ppt_dev, time_reg_data, BMP388_REG_SENS_TIME_LEN, BMP388_REG_SENS_TIME);
    if (api_ret_val == RET_OK)
    {
        ret_val = BMP388_NO_ERROR;
        ppt_dev->data.sensortime =
          BYTES_TO_DWORD(unsigned, time_reg_data[0U], time_reg_data[1U], time_reg_data[2U], 0U);
    }
    else
    {
        ret_val = BMP388_ERROR_API;
    }
    return ret_val;
}

/**
 * @brief This internal function reads the pressure data from the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
static bmp388_status_t read_pressure(bmp388_dev_t* ppt_dev)
{
    response_status_t api_ret_val                             = RET_OK;
    bmp388_status_t   ret_val                                 = BMP388_NO_ERROR;
    bool_t            temp_rdy                                = FALSE;
    bool_t            pres_rdy                                = FALSE;
    driver_t*         pt_curr_driver                          = (driver_t*)ppt_dev;
    uint8_t           pres_reg_data[BMP388_REG_DATA_PRES_LEN] = { 0U };
    // If the interrupt is enabled and thie function is called
    // we assume that the data is ready
    if (ppt_dev->settings.int_settings.int_enable
        & BMP388_INT_ENABLE_DRDY == BMP388_INT_ENABLE_DRDY)
    {
        pres_rdy = TRUE;
    }
    else
    {
        api_ret_val = get_data_state(ppt_dev, &temp_rdy, &pres_rdy);
    }

    if (api_ret_val != RET_OK)
    {
        ret_val = BMP388_ERROR_API;
    }
    else if (pres_rdy == FALSE)
    {
        ret_val = BMP388_WAITING_PRESS;
    }
    else
    {
        api_ret_val =
          read_register(ppt_dev, pres_reg_data, BMP388_REG_DATA_PRES_LEN, BMP388_REG_DATA_PRES);
        if (api_ret_val == RET_OK)
        {
            pt_curr_driver->raw_data.pressure =
              BYTES_TO_DWORD(unsigned, pres_reg_data[0U], pres_reg_data[1U], pres_reg_data[2U], 0U);
            ppt_dev->data.pressure = 0U;
            compensate_pressure(ppt_dev);
            ret_val = BMP388_NO_ERROR;
        }
        else
        {
            ret_val = BMP388_ERROR_API;
        }
    }

    return ret_val;
}

/**
 * @brief This internal function reads the temperature data from the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
static bmp388_status_t read_temp(bmp388_dev_t* ppt_dev)
{
    response_status_t api_ret_val    = RET_OK;
    bmp388_status_t   ret_val        = BMP388_NO_ERROR;
    bool_t            temp_rdy       = FALSE;
    bool_t            pres_rdy       = FALSE;
    driver_t*         pt_curr_driver = (driver_t*)ppt_dev;

    uint8_t temp_reg_data[BMP388_REG_DATA_TEMP_LEN] = { 0U };

    // If the interrupt is enabled and thie function is called
    // we assume that the data is ready
    if (ppt_dev->settings.int_settings.int_enable
        & BMP388_INT_ENABLE_DRDY == BMP388_INT_ENABLE_DRDY)
    {
        pres_rdy = TRUE;
    }
    else
    {
        api_ret_val = get_data_state(ppt_dev, &temp_rdy, &pres_rdy);
    }

    if (api_ret_val != RET_OK)
    {
        ret_val = BMP388_ERROR_API;
    }
    else if (temp_rdy == FALSE)
    {
        ret_val = BMP388_WAITING_TEMP;
    }
    else
    {
        api_ret_val =
          read_register(ppt_dev, temp_reg_data, BMP388_REG_DATA_TEMP_LEN, BMP388_REG_DATA_TEMP);
        if (api_ret_val == RET_OK)
        {
            pt_curr_driver->raw_data.temperature =
              BYTES_TO_DWORD(unsigned, temp_reg_data[0U], temp_reg_data[1U], temp_reg_data[2U], 0U);
            ppt_dev->data.temperature = 0U;
            compensate_temperature(ppt_dev);
            ret_val = BMP388_NO_ERROR;
        }
        else
        {
            ret_val = BMP388_ERROR_API;
        }
    }

    return ret_val;
}

static response_status_t send_cmd(bmp388_dev_t* ppt_dev, bmp388_cmds p_cmd, uint32_t p_timeout_ms)
{
    response_status_t ret_val       = RET_OK;
    uint8_t           reg_val       = 0U;
    bool_t            cmd_rdy       = FALSE;
    uint32_t          max_retry_cnt = p_timeout_ms / 10U; // 10 ms intervals

    while (max_retry_cnt > 0U)
    {
        ret_val = read_register(ppt_dev, &reg_val, DEFAULT_IIC_REG_SZ, BMP388_REG_SENS_STATUS);
        if (ret_val == RET_OK)
        {
            cmd_rdy = BMP3_GET_BITS(reg_val, BMP388_REG_SENS_STATUS_CMD) == 0x01;
            if (cmd_rdy == TRUE)
            {
                ret_val = write_register(ppt_dev, &p_cmd, DEFAULT_IIC_REG_SZ, BMP388_REG_CMD);
                break;
            }
            else
            {
                ret_val = RET_TIMEOUT;
            }
        }
        max_retry_cnt--;
        ps_hard_delay_ms(10U); // Wait for 10 ms before retrying
    }

    return ret_val;
}

static bool_t is_data_rdy(bmp388_dev_t* ppt_dev)
{
    response_status_t api_ret_val = RET_OK;
    uint8_t           reg_val     = 0x00;
    bool_t            data_rdy    = FALSE;

    api_ret_val = read_register(ppt_dev, &reg_val, DEFAULT_IIC_REG_SZ, BMP388_REG_INT_STATUS);

    if (api_ret_val == RET_OK)
    {
        data_rdy = BMP3_GET_BITS(reg_val, BMP388_REG_INT_STATUS_DRDY) == 0x01;
    }

    return data_rdy;
}

/**
 * @brief This function sets the OSR, ODR and IIR filter settings of the sensor
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_set_data_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    uint8_t           settings_data = 0U;

    settings_data =
      BMP3_SET_BITS(BMP388_REG_OSR_PRES, ppt_dev->settings.data_settings.press_oversampling);
    settings_data |=
      BMP3_SET_BITS(BMP388_REG_OSR_TEMP, ppt_dev->settings.data_settings.temp_oversampling);
    ret_val |= write_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_OSR);

    settings_data = BMP3_SET_BITS(BMP388_REG_ODR, ppt_dev->settings.data_settings.output_data_rate);
    ret_val      |= write_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_ODR);

    settings_data = BMP3_SET_BITS(BMP388_REG_CONFIG, ppt_dev->settings.data_settings.iir_filter);
    ret_val      |= write_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_CONFIG);

    return ret_val;
}

/**
 * @brief This function sets the pressure enable, temperature enable and the
 * measurement mode of the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_set_dev_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    uint8_t           settings_data = 0U;

    settings_data =
      BMP3_SET_BITS(BMP388_REG_PWR_CTRL_MODE, ppt_dev->settings.dev_settings.power_mode);
    settings_data |=
      BMP3_SET_BITS(BMP388_REG_PWR_CTRL_EN, ppt_dev->settings.dev_settings.sensor_enable);
    ret_val = write_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_PWR_CTRL);

    return ret_val;
}

/**
 * @brief This function sets the I2C and SPI communication interface settings of
 * the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_set_ifc_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    uint8_t           settings_data = 0U;

    settings_data =
      BMP3_SET_BITS(BMP388_REG_IF_CONF_SPI, ppt_dev->settings.comm_ifc_settings.spi_mode);
    settings_data |=
      BMP3_SET_BITS(BMP388_REG_IF_CONF_WDT, ppt_dev->settings.comm_ifc_settings.iic_wdt);
    ret_val = write_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_IF_CONF);

    return ret_val;
}

/**
 * @brief This function sets the interrupt control settings of the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_set_interrupt_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    uint8_t           settings_data = 0U;

    settings_data =
      BMP3_SET_BITS(BMP388_REG_INT_CTRL_EN, ppt_dev->settings.int_settings.int_enable);
    settings_data |=
      BMP3_SET_BITS(BMP388_REG_INT_CTRL_TYPE, ppt_dev->settings.int_settings.int_type);
    ret_val = write_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_INT_CTRL);

    return ret_val;
}

/**
 * @brief This function gets the OSR, ODR and IIR filter settings of the sensor
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_get_data_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val           = RET_OK;
    uint8_t           settings_data[3U] = { 0U };

    ret_val |= read_register(ppt_dev, &settings_data[0U], DEFAULT_IIC_REG_SZ, BMP388_REG_OSR);
    ret_val |= read_register(ppt_dev, &settings_data[1U], DEFAULT_IIC_REG_SZ, BMP388_REG_ODR);
    ret_val |= read_register(ppt_dev, &settings_data[2U], DEFAULT_IIC_REG_SZ, BMP388_REG_CONFIG);

    if (ret_val == RET_OK)
    {
        ppt_dev->settings.data_settings.press_oversampling =
          BMP3_GET_BITS(settings_data[0U], BMP388_REG_OSR_PRES);
        ppt_dev->settings.data_settings.temp_oversampling =
          BMP3_GET_BITS(settings_data[0U], BMP388_REG_OSR_TEMP);
        ppt_dev->settings.data_settings.output_data_rate =
          BMP3_GET_BITS(settings_data[1U], BMP388_REG_ODR);
        ppt_dev->settings.data_settings.iir_filter =
          BMP3_GET_BITS(settings_data[2U], BMP388_REG_CONFIG);
    }

    return ret_val;
}

/**
 * @brief This function gets the pressure enable, temperature enable and the
 * measurement mode of the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_get_dev_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    uint8_t           settings_data = 0U;

    ret_val = read_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_PWR_CTRL);

    if (ret_val == RET_OK)
    {
        ppt_dev->settings.dev_settings.power_mode =
          BMP3_GET_BITS(settings_data, BMP388_REG_PWR_CTRL_MODE);
        ppt_dev->settings.dev_settings.sensor_enable =
          BMP3_GET_BITS(settings_data, BMP388_REG_PWR_CTRL_EN);
    }

    return ret_val;
}

/**
 * @brief This function gets the I2C and SPI communication interface settings of
 * the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_get_ifc_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    uint8_t settings_data = 0U;

    ret_val = read_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_IF_CONF);

    if (ret_val == RET_OK)
    {
        ppt_dev->settings.comm_ifc_settings.spi_mode =
          BMP3_GET_BITS(settings_data, BMP388_REG_IF_CONF_SPI);
        ppt_dev->settings.comm_ifc_settings.iic_wdt =
          BMP3_GET_BITS(settings_data, BMP388_REG_IF_CONF_WDT);
    }

    return ret_val;
}

/**
 * @brief This function gets the interrupt control settings of the sensor.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_get_interrupt_settings(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    uint8_t           settings_data = 0U;

    ret_val = read_register(ppt_dev, &settings_data, DEFAULT_IIC_REG_SZ, BMP388_REG_INT_CTRL);

    if (ret_val == RET_OK)
    {
        ppt_dev->settings.int_settings.int_enable =
          BMP3_GET_BITS(settings_data, BMP388_REG_INT_CTRL_EN);
        ppt_dev->settings.int_settings.int_type =
          BMP3_GET_BITS(settings_data, BMP388_REG_INT_CTRL_TYPE);
    }

    return ret_val;
}

/**
 * @brief This function retrieves the sensor data based on the requested data
 * type. It reads the sensor time, temperature, and pressure data as needed.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @param[in] p_data_req The data request flags indicating which data to read.
 * @return Result of the execution status.
 * @retval `BMP388_NO_ERROR` if the data is read successfully.
 * @retval `BMP388_ERROR_API` if there is an error in communication with the
 * sensor or device not initialized.
 * @retval `BMP388_WAITING_PRESS` if pressure data is not ready.
 * @retval `BMP388_WAITING_TEMP` if temperature data is not ready.
 */
bmp388_status_t dd_bmp388_get_data(bmp388_dev_t* ppt_dev, bmp388_data_request_t p_data_req)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, BMP388_ERROR_API);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, BMP388_ERROR_API);

    bmp388_status_t   ret_val     = BMP388_NO_ERROR;
    response_status_t api_ret_val = RET_OK;
    uint8_t           reg_val     = 0x00;
    bool_t            data_rdy    = FALSE;

    if (ppt_dev->settings.int_settings.int_enable
        & BMP388_INT_ENABLE_DRDY == BMP388_INT_ENABLE_DRDY)
    {
        data_rdy = is_data_rdy(ppt_dev);
    }
    else
    {
        data_rdy = TRUE;
    }
    if (data_rdy)
    {
        if (p_data_req & BMP388_READ_TIME)
        {
            ret_val = read_sens_time(ppt_dev);
        }

        /**
         * @note If any of the pressure or temperature data is requested,
         * we need to read temperature for compensation and then if
         * pressure is requested, we read pressure data.
         * If only temperature is requested, we don't read pressure data.
         */
        if ((ret_val == BMP388_NO_ERROR) && (p_data_req & BMP388_READ_PRESS_TEMP))
        {
            ret_val = read_temp(ppt_dev);
        }

        if ((ret_val == BMP388_NO_ERROR) && (p_data_req & BMP388_READ_PRESSURE))
        {
            ret_val = read_pressure(ppt_dev);
        }

        /// if temperature data is not requested, we set it to 0
        if (!(p_data_req & BMP388_READ_TEMP))
        {
            ppt_dev->data.temperature = 0U;
        }
    }
    else
    {
        ret_val = BMP388_WAITING_DATA;
    }
    return ret_val;
}

/**
 * @brief This function retrieves the error state of the BMP388 device.
 * @param[in,out] ppt_dev BMP388 device instance.
 * @return The error state of the BMP388 device.
 */
bmp388_status_t dd_bmp388_get_error_state(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, BMP388_ERROR_API);
    ASSERT_AND_RETURN(((driver_t*)(ppt_dev))->is_initialized == FALSE, BMP388_ERROR_API);

    response_status_t api_ret_val = RET_OK;
    bmp388_status_t   ret_val     = BMP388_NO_ERROR;
    uint8_t           reg_data    = 0x00;

    api_ret_val = read_register(ppt_dev, &reg_data, DEFAULT_IIC_REG_SZ, BMP388_REG_ERR);

    if (api_ret_val != RET_OK)
    {
        ret_val = BMP388_ERROR_API;
    }
    else
    {
        ret_val = (reg_data & BMP388_ERROR_CMD) | (reg_data & BMP388_ERROR_FATAL)
                  | (reg_data & BMP388_ERROR_CONFIG);
    }

    return ret_val;
}

bmp388_status_t dd_bmp388_reset(bmp388_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, BMP388_ERROR_API);
    response_status_t api_ret_val = RET_OK;
    bmp388_status_t   ret_val     = BMP388_NO_ERROR;
    uint8_t           reg_val     = 0x00;
    api_ret_val                   = send_cmd(ppt_dev, BMP388_CMD_SOFT_RESET, 100U);
    if (api_ret_val == RET_OK)
    {
        ps_hard_delay_ms(25U); // Wait for 25 ms after reset
        api_ret_val = read_register(ppt_dev, &reg_val, DEFAULT_IIC_REG_SZ, BMP388_REG_EVENT);
        ret_val     = dd_bmp388_get_error_state(ppt_dev);
    }

    if (api_ret_val == RET_OK)
    {
        if (reg_val == 0x01 && ret_val == BMP388_NO_ERROR)
        {
            ret_val = BMP388_NO_ERROR;
        }
        else
        {
            // Do Nothing. Return ret_val value
        }
    }
    else
    {
        ret_val = BMP388_ERROR_API;
    }

    return ret_val;
}

/**
 * @brief This function retrieves the BMP388 device instance based on the device
 * ID.
 * @param[in] p_dev_id The device ID of the BMP388 device.
 * @return Pointer to the BMP388 device instance.
 */
bmp388_dev_t* dd_bmp388_get_dev(bmp388_devices_t p_dev_id)
{
    return &g_driver[p_dev_id].dev;
}

/**
 * @brief This function initializes the BMP388 device driver.
 * @param[in] ppt_dev Address of the BMP388 device instance handler.
 * @note The reason for using a pointer to a pointer is to allow the user to
 * define the instance handler locally and retrive it when necessary. Minimizing
 * global variables usage.
 * @param[in] p_dev_id Device ID of the BMP388 device to initialize.
 * @return Result of the execution status.
 */
response_status_t dd_bmp388_init(bmp388_dev_t** ppt_dev, bmp388_devices_t p_dev_id)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val        = RET_OK;
    driver_t*         pt_curr_driver = &g_driver[p_dev_id];

    ret_val = ha_iic_init();

    if (ret_val == RET_OK)
    {
        memset(pt_curr_driver, 0U, sizeof(driver_t));
        pt_curr_driver->dev_id = p_dev_id;

        if (is_dev_ready(&(pt_curr_driver->dev)))
        {
            pt_curr_driver->is_initialized = TRUE;
            ret_val                        = read_calib_data(&pt_curr_driver->dev);
            ret_val                       |= dd_bmp388_get_data_settings(&pt_curr_driver->dev);
            ret_val                       |= dd_bmp388_get_dev_settings(&pt_curr_driver->dev);
            ret_val                       |= dd_bmp388_get_ifc_settings(&pt_curr_driver->dev);
            ret_val                       |= dd_bmp388_get_interrupt_settings(&pt_curr_driver->dev);
        }
        else
        {
            ret_val = RET_NOT_FOUND;
        }
    }

    else
    {
        // Nothing to do
    }

    if (ret_val == RET_OK)
    {
        pt_curr_driver->is_initialized = TRUE;
        *ppt_dev                       = &(pt_curr_driver->dev);
    }
    else
    {
        pt_curr_driver->is_initialized = FALSE;
        *ppt_dev                       = NULL;
    }

    return ret_val;
}
