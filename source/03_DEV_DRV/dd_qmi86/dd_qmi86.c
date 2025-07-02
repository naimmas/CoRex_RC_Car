
#include "dd_qmi86.h"
#include "dd_qmi86_defs.h"
#include "ha_iic/ha_iic.h"
#include "ha_timer/ha_timer.h"
#include "string.h"
#include "stdlib.h"

#define DEFAULT_IIC_TIMEOUT (100U) // Default I2C timeout in milliseconds

#define QMI_OWR_BITS(reg_data, bitname, data) (((reg_data) & (~bitname##_MSK)) | ((data) << (bitname##_POS)))
#define QMI_GET_BITS(reg_data, bitname) (((reg_data) & (bitname##_MSK)) >> (bitname##_POS))

IIC_SETUP_PORT_CONNECTION(QMI86_DEV_CNT,
                          IIC_DEFINE_CONNECTION(IIC_PORT1, QMI86_DEV_1,
                                                QMI86_IIC_ADDR_1))

typedef struct st_driver
{
    qmi86_dev_t                dev;
    uint8_t                     dev_id;
    bool_t                      is_initialized;
} driver_t;

static driver_t g_driver[QMI86_DEV_CNT];

/**
 * @brief This internal function writes data to a specific register of the
 * QMI8658 device over I2C.
 * @param[in,out] ppt_dev QMI8658 device instance.
 * @param[in] ppt_data Pointer to the data buffer to write.
 * @param[in] p_data_sz Size of the data to write in bytes.
 * @param[in] p_reg_addr The register address to write to in the sensor.
 * @return Result of the execution status.
 */
static response_status_t write_register(qmi86_dev_t* ppt_dev,
                                        uint8_t* ppt_data, size_t p_data_sz,
                                        uint8_t p_reg_addr)
{
    response_status_t ret_val        = RET_OK;
    driver_t*         pt_curr_driver = (driver_t*)ppt_dev;

    ret_val =
      ha_iic_master_mem_write(IIC_GET_DEV_PORT(pt_curr_driver->dev_id),
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
 * QMI8658 device over I2C.
 * @param[in,out] ppt_dev QMI8658 device instance.
 * @param[out] ppt_data Pointer to the data buffer to read into.
 * @param[in] p_data_sz Size of the data to read in bytes.
 * @param[in] p_reg_addr The register address to read from in the sensor.
 * @return Result of the execution status.
 */
static response_status_t read_register(qmi86_dev_t* ppt_dev, uint8_t* ppt_data,
                                       size_t p_data_sz, uint8_t p_reg_addr)
{
    response_status_t ret_val        = RET_OK;
    driver_t*         pt_curr_driver = (driver_t*)ppt_dev;

    ret_val =
      ha_iic_master_mem_read(IIC_GET_DEV_PORT(pt_curr_driver->dev_id),
                             IIC_GET_DEV_ADDRESS(pt_curr_driver->dev_id),
                             ppt_data,
                             p_data_sz,
                             p_reg_addr,
                             HW_IIC_MEM_SZ_8BIT,
                             DEFAULT_IIC_TIMEOUT);

    return ret_val;
}

static bool_t is_dev_ready(qmi86_dev_t* ppt_dev)
{
    response_status_t ret_val = RET_OK;
    uint8_t who_am_i = 0x00;
    uint8_t chip_id = 0x00;

    ret_val = read_register(ppt_dev,
                            &who_am_i,
                            1U,
                            QMI86_REG_WHO_AM_I);
    ret_val |= read_register(ppt_dev,
                            &chip_id,
                            1U,
                            QMI86_REG_REVISION_ID);

    return ((ret_val == RET_OK) && ((chip_id == QMI86_CHIP_ID) && (who_am_i == QMI86_WHO_AM_I)));
}

static response_status_t read_firmware_version(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    
    response_status_t ret_val = RET_OK;

    ret_val = read_register(ppt_dev, &ppt_dev->chip_fw_version.u8_arr[0], 1, QMI86_REG_DQW_L);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_fw_version.u8_arr[1], 1, QMI86_REG_DQW_H);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_fw_version.u8_arr[2], 1, QMI86_REG_DQX_L);

    return ret_val;
}

static response_status_t read_chip_id(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    
    response_status_t ret_val = RET_OK;

    ret_val = read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[0], 1, QMI86_REG_DVX_L);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[1], 1, QMI86_REG_DVX_H);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[2], 1, QMI86_REG_DVY_L);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[3], 1, QMI86_REG_DVY_H);

    return ret_val;    
}

static response_status_t wait_for_statusint_0bit(qmi86_dev_t* ppt_dev, uint8_t desired_bit_val)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(desired_bit_val > 0x01, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;
    bool_t bit_ok = FALSE;
    uint8_t reg_data = 0x00;
    uint8_t max_retry_cnt = 50U;
    const uint8_t delay_per_try_ms = 10U;

    while(ret_val == RET_OK && bit_ok == FALSE)
    {
        ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_STATUSINT);
        bit_ok = (QMI_GET_BITS(reg_data, QMI86_REG_STATUSINT_AVAIL) == desired_bit_val);

        max_retry_cnt--;
        if (max_retry_cnt == 0)
        {
            ret_val = RET_TIMEOUT;
            break;
        }

        ha_timer_hard_delay_ms(delay_per_try_ms);
    }

    return ret_val;
}

response_status_t dd_qmi86_set_data_settings(qmi86_dev_t* ppt_dev)
{
  //TODO: Implement Me!
  //! Refer to 13.5

  (void)ppt_dev;
  return (response_status_t)0;
}

response_status_t dd_qmi86_set_interface_settings(qmi86_dev_t* ppt_dev)
{
  /*TODO: Implement Me!*/
  (void)ppt_dev;
  return (response_status_t)0;
}

response_status_t dd_qmi86_set_interrupt_settings(qmi86_dev_t* ppt_dev)
{
  /*TODO: Implement Me!*/
  (void)ppt_dev;
  return (response_status_t)0;
}

response_status_t dd_qmi86_snooze_gyro(qmi86_dev_t* ppt_dev, bool_t p_snooze)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    
    uint8_t ctrl7_reg_data = 0x00;
    response_status_t ret_val = RET_OK;
    
    ret_val = read_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);
    
    if (ret_val == RET_OK)
    {
        ctrl7_reg_data = QMI_OWR_BITS(ctrl7_reg_data, QMI86_REG_CTRL7_GSN, p_snooze);
        ret_val = write_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);
    }
    else
    {
        // Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_set_device_mode(qmi86_dev_t* ppt_dev, qmi86_sensor_mode p_dev_mode)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_dev_mode > QMI86_SENSOR_MODE_ACC_GYRO, RET_PARAM_ERROR);

    uint8_t ctrl1_reg_data = 0x00;
    uint8_t ctrl7_reg_data = 0x00;
    uint8_t sys_delays[QMI86_SENSOR_MODE_ACC_GYRO + 1] = {QMI86_POWER_OFF_TIME, 
                            QMI86_ACCEL_POWER_ON_TIME,      
                            QMI86_GYRO_POWER_ON_TIME,
                            // ignore accel turn on time as it's much lower than gyro
                            QMI86_GYRO_POWER_ON_TIME};
    uint8_t all_disabled = (p_dev_mode == QMI86_SENSOR_DISABLE);
    response_status_t ret_val = RET_OK;

    // Save register contents
    ret_val = read_register(ppt_dev, &ctrl1_reg_data, 1, QMI86_REG_CTRL1);
    ret_val |= read_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);

    if (ret_val == RET_OK)
    {
        ctrl1_reg_data = QMI_OWR_BITS(ctrl1_reg_data, QMI86_REG_CTRL1_SENS_DISABLE, all_disabled);
        ctrl7_reg_data = QMI_OWR_BITS(ctrl7_reg_data, QMI86_REG_CTRL7_AGEN, p_dev_mode);

        ret_val |= write_register(ppt_dev, &ctrl1_reg_data, 1, QMI86_REG_CTRL1);
        ret_val |= write_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);

        // Wait for process
        ha_timer_hard_delay_ms(sys_delays[p_dev_mode]);
    }
    else 
    {
        // Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_get_data_settings(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    
    response_status_t ret_val = RET_OK;
    uint8_t reg_content[3] = { 0x00 };
    struct st_qmi86_data_settings* hndlr = &ppt_dev->settings.data_settings;

    ret_val = read_register(ppt_dev, &reg_content[0], 1, QMI86_REG_CTRL2);
    ret_val |= read_register(ppt_dev, &reg_content[1], 1, QMI86_REG_CTRL3);
    ret_val |= read_register(ppt_dev, &reg_content[2], 1, QMI86_REG_CTRL5);

    if (ret_val == RET_OK)
    {
        hndlr->acc_odr = QMI_GET_BITS(reg_content[0], QMI86_REG_CTRL2_AODR);
        hndlr->acc_fsr = QMI_GET_BITS(reg_content[0], QMI86_REG_CTRL2_AFS);
        
        hndlr->gyro_odr = QMI_GET_BITS(reg_content[1], QMI86_REG_CTRL3_GODR);
        hndlr->gyro_fsr = QMI_GET_BITS(reg_content[1], QMI86_REG_CTRL3_GFS);

        hndlr->acc_lpf_en = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_ALPF_EN);
        hndlr->acc_lpf_mode = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_ALPF_MODE);
        
        hndlr->gyro_lpf_en = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_GLPF_EN);
        hndlr->gyro_lpf_mode = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_GLPF_MODE);
    }
    else
    {
        // Nothing to do
    }

    return ret_val;

}

response_status_t dd_qmi86_get_interface_settings(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;
    uint8_t reg_content = 0x00;
    struct st_qmi86_ifc_settings* hndlr = &ppt_dev->settings.comm_ifc_settings;

    ret_val = read_register(ppt_dev, &reg_content, 1, QMI86_REG_CTRL1);

    if(ret_val == RET_OK)
    {
        hndlr->spi_mode = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_SIM);
    }
    else
    {
        //Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_get_interrupt_settings(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;
    uint8_t reg_content = 0x00;
    struct st_qmi86_int_settings * hndlr = &ppt_dev->settings.interrupt_settings;

    ret_val = read_register(ppt_dev, &reg_content, 1, QMI86_REG_CTRL1);

    if(ret_val == RET_OK)
    {
        hndlr->fifo_int_output = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_FIFO_INT_SEL);
        hndlr->int1_enable = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_INT1_EN);
        hndlr->int2_enable = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_INT2_EN);
        hndlr->drdy_enable = QMI_GET_BITS(reg_content, QMI86_REG_CTRL7_DRDY_DIS);
    }
    else 
    {
        // Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_get_device_mode(qmi86_dev_t* ppt_dev, qmi86_sensor_mode* p_dev_mode)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    
    uint8_t ctrl1_reg_data = 0x00;
    uint8_t ctrl7_reg_data = 0x00;
    response_status_t ret_val = RET_OK;

    // Save register contents
    ret_val = read_register(ppt_dev, &ctrl1_reg_data, 1, QMI86_REG_CTRL1);
    ret_val |= read_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);

    if (ret_val == RET_OK)
    {
        if (QMI_GET_BITS(ctrl1_reg_data, QMI86_REG_CTRL1_SENS_DISABLE) == 0x01)
        {
            *p_dev_mode = QMI86_SENSOR_DISABLE;
        }
        else
        {
            // Maps enabling bits to enum
            *p_dev_mode = QMI_GET_BITS(ctrl7_reg_data, QMI86_REG_CTRL7_AGEN);
        }
    }
    else
    {
        //Nothing to do
    }

    return ret_val;
}

qmi86_dev_t* dd_qmi86_get_dev(qmi86_dev_id_t p_dev_id)
{
    return &g_driver[p_dev_id].dev;
}

qmi86_st_result dd_qmi86_gyro_self_test(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, QMI86_ST_RESULT_API_ERROR);

    response_status_t api_ret_val = RET_OK;
    qmi86_st_result ret_val = QMI86_ST_RESULT_OK;
    uint8_t ctrl7_bkp = 0x00;
    uint8_t reg_data = 0x00;
    // The typical time for Self-Test costs about 25 ODRs as odr selected for st is 1Khz then total time is 25Ms
    const uint16_t st_total_time_ms = 300U;
    uint8_t st_result[6] = { 0x00 };
    int16_t st_x_dps = 0U;
    int16_t st_y_dps = 0U;
    int16_t st_z_dps = 0U;

    api_ret_val = read_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
    //1. Disable the sensors (CTRL7 = 0x00).
    if (api_ret_val == RET_OK)
    {
        reg_data = 0x00;
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
    }

    //2. Set the bit gST to 1. (CTRL3.bit7 = 1’b1).
    api_ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL3);
    if (api_ret_val == RET_OK)
    {
        reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL3_GST, 0x01);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL3);
    }

    //3. Wait for QMI8658A to drive INT2 to High, if INT2 is enabled (CTRL1.bit4 = 1), or STATUSINT.bit0 is set to 1.
    if (api_ret_val == RET_OK)
    {
        reg_data = 0x00;
        ha_timer_hard_delay_ms(st_total_time_ms);
        api_ret_val = wait_for_statusint_0bit(ppt_dev, 0x01);
    }

    //4. Set CTRL2.aST(bit7) to 0, to clear STATUSINT1.bit0 and/or INT2.
    if(api_ret_val ==RET_OK)
    {
        reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AST, 0x00);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    }

    //5. Check for QMI8658A drives INT2 back to Low, and sets STATUSINT1.bit0 to 0.
    if(api_ret_val == RET_OK)
    {
        api_ret_val = wait_for_statusint_0bit(ppt_dev, 0x00);
    }

    //6. Read the Accel Self-Test result:
    if(api_ret_val ==RET_OK)
    {
        api_ret_val = read_register(ppt_dev, st_result, ARRAY_SIZE(st_result), QMI86_REG_DVX_L);
    }

    if(api_ret_val == RET_OK)
    {
        st_x_dps =(int16_t)BYTES_TO_WORD(st_result[0], st_result[1]);
        st_y_dps =(int16_t)BYTES_TO_WORD(st_result[2], st_result[3]);
        st_z_dps =(int16_t)BYTES_TO_WORD(st_result[4], st_result[5]);
        
        if (abs(st_x_dps) > (300U * (1U << 4U)) &&
            abs(st_y_dps) > (300U * (1U << 4U)) &&
            abs(st_z_dps) > (300U * (1U << 4U)))
            {
                ret_val = QMI86_ST_RESULT_OK;
            }
            else
            {
                ret_val = QMI86_ST_RESULT_FAILED_GYRO;
            }
    }

    write_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);

    if (api_ret_val == RET_TIMEOUT)
    {
        ret_val = QMI86_ST_RESULT_TIMEOUT;
    }
    else if (api_ret_val != RET_OK)
    {
        ret_val = QMI86_ST_RESULT_API_ERROR;
    }

    return ret_val;
}

qmi86_st_result dd_qmi86_accel_self_test(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, QMI86_ST_RESULT_API_ERROR);

    response_status_t api_ret_val = RET_OK;
    qmi86_st_result ret_val = QMI86_ST_RESULT_OK;
    uint8_t ctrl7_bkp = 0x00;
    uint8_t reg_data = 0x00;
    // The typical time for Self-Test costs about 25 ODRs as odr selected for st is 1Khz then total time is 25Ms
    const uint16_t st_total_time_ms = (QMI86_ST_ODR_CYCLE_CNT * 1U);
    uint8_t st_result[6] = { 0x00 };
    int16_t st_x_mg = 0U;
    int16_t st_y_mg = 0U;
    int16_t st_z_mg = 0U;

    api_ret_val = read_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
    //1. Disable the sensors (CTRL7 = 0x00).
    if (api_ret_val == RET_OK)
    {
        reg_data = 0x00;
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
    }

    //2. Set proper accelerometer ODR (CTRL2.aODR) and bit CTRL2.aST (bit7) to 1 to trigger the Self-Test.
    api_ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    if (api_ret_val == RET_OK)
    {
        reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AODR, QMI86_ACC_ODR_1000_HZ);
        reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AST, 0x01);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    }

    //3. Wait for QMI8658A to drive INT2 to High, if INT2 is enabled (CTRL1.bit4 = 1), or STATUSINT.bit0 is set to 1.
    if (api_ret_val == RET_OK)
    {
        reg_data = 0x00;
        ha_timer_hard_delay_ms(st_total_time_ms);
        api_ret_val = wait_for_statusint_0bit(ppt_dev, 0x01);
    }

    //4. Set CTRL2.aST(bit7) to 0, to clear STATUSINT1.bit0 and/or INT2.
    if(api_ret_val ==RET_OK)
    {
        reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AST, 0x00);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    }

    //5. Check for QMI8658A drives INT2 back to Low, and sets STATUSINT1.bit0 to 0.
    if(api_ret_val == RET_OK)
    {
        api_ret_val = wait_for_statusint_0bit(ppt_dev, 0x00);
    }

    //6. Read the Accel Self-Test result:
    if(api_ret_val ==RET_OK)
    {
        api_ret_val = read_register(ppt_dev, st_result, ARRAY_SIZE(st_result), QMI86_REG_DVX_L);
    }

    if(api_ret_val == RET_OK)
    {
        st_x_mg =(int16_t)BYTES_TO_WORD(st_result[0], st_result[1]);
        st_y_mg =(int16_t)BYTES_TO_WORD(st_result[2], st_result[3]);
        st_z_mg =(int16_t)BYTES_TO_WORD(st_result[4], st_result[5]);
        
        if (abs(st_x_mg) > (200U * 2U) &&
            abs(st_y_mg) > (200U * 2U) &&
            abs(st_z_mg) > (200U * 2U))
            {
                ret_val = QMI86_ST_RESULT_OK;
            }
            else
            {
                ret_val = QMI86_ST_RESULT_FAILED_ACC;
            }
    }

    write_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);

    if (api_ret_val == RET_TIMEOUT)
    {
        ret_val = QMI86_ST_RESULT_TIMEOUT;
    }
    else if (api_ret_val != RET_OK)
    {
        ret_val = QMI86_ST_RESULT_API_ERROR;
    }

    return ret_val;
}

response_status_t dd_qmi86_perform_calibration(qmi86_dev_t* ppt_dev)
{
  /*TODO: Implement Me!*/
  (void)ppt_dev;
  return (response_status_t)0;
}

response_status_t dd_qmi86_reset_device(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    uint8_t reg_data = QMI86_RESET_CMD;
    response_status_t ret_val = RET_OK;
    bool_t reset_ok = FALSE;

    ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_RESET);

    if (ret_val == RET_OK)
    {
        //Wait until device reset
        ha_timer_hard_delay_ms(QMI86_SYS_POWER_ON_TIME);
        reg_data = 0x00;
        ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_RESET_RESULT);
        reset_ok = (reg_data == QMI86_RESET_SUCCESSFUL);
    }
    else
    {
        //Nothing to do
    }

    if (ret_val == RET_OK && reset_ok == FALSE)
    {
        ret_val = RET_ERROR;
    }
    else
    {
        //Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_poll_data(qmi86_dev_t* ppt_dev, bool_t from_isr)
{
  /*TODO: Implement Me!*/
  (void)ppt_dev;
  (void)from_isr;
  return (response_status_t)0;
}

response_status_t init_comm_ifc(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    uint8_t reg_data = 0x00;
    response_status_t ret_val = RET_OK;

    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL1_ADDR_AI, 0x01);
    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL1_BE, 0x01);

    ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL1);

    return ret_val;
}

response_status_t dd_qmi86_init(qmi86_dev_t** ppt_dev, qmi86_dev_id_t p_dev_id)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val        = RET_OK;
    driver_t*         pt_curr_drv = &g_driver[p_dev_id];
    qmi86_dev_t*      pt_curr_dev = &(pt_curr_drv->dev);

    ret_val = ha_iic_init();

    if (ret_val == RET_OK)
    {
        memset(pt_curr_drv, 0U, sizeof(driver_t));
        pt_curr_drv->dev_id = p_dev_id;

        if (is_dev_ready(pt_curr_dev) == TRUE && init_comm_ifc(pt_curr_dev) == RET_OK)
        {
            pt_curr_drv->is_initialized = TRUE;
            ret_val |= read_firmware_version(pt_curr_dev);
            ret_val |= read_chip_id(pt_curr_dev);
            ret_val |= dd_qmi86_get_data_settings(pt_curr_dev);
            ret_val |= dd_qmi86_get_interface_settings(pt_curr_dev);
            ret_val |= dd_qmi86_get_interrupt_settings(pt_curr_dev);
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
        pt_curr_drv->is_initialized = TRUE;
        *ppt_dev                       = pt_curr_dev;
    }
    else
    {
        pt_curr_drv->is_initialized = FALSE;
        *ppt_dev                       = NULL;
    }

    return ret_val;
}
