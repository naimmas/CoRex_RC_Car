
#include "dd_qmi86.h"

#include "dd_qmi86_defs.h"
#include "ha_iic/ha_iic.h"
#include "ha_timer/ha_timer.h"
#include "stdlib.h"
#include "string.h"

#define DEFAULT_IIC_TIMEOUT (100U) // Default I2C timeout in milliseconds

#define QMI_OWR_BITS(reg_data, bitname, data) (((reg_data) & (~bitname##_MSK)) | ((data) << (bitname##_POS)))
#define QMI_GET_BITS(reg_data, bitname) (((reg_data) & (bitname##_MSK)) >> (bitname##_POS))

IIC_SETUP_PORT_CONNECTION(QMI86_DEV_CNT,
                          IIC_DEFINE_CONNECTION(IIC_PORT1, QMI86_DEV_1, QMI86_IIC_ADDR_2))

typedef enum
{
    CTRL_CMD_ACK                     = 0x00,
    CTRL_CMD_RST_FIFO                = 0x04,
    CTRL_CMD_REQ_FIFO                = 0x05,
    CTRL_CMD_WRITE_WOM_SETTING       = 0x08,
    CTRL_CMD_ACCEL_HOST_DELTA_OFFSET = 0x09,
    CTRL_CMD_GYRO_HOST_DELTA_OFFSET  = 0x0A,
    CTRL_CMD_CONFIGURE_TAP           = 0x0C,
    CTRL_CMD_CONFIGURE_PEDOMETER     = 0x0D,
    CTRL_CMD_CONFIGURE_MOTION        = 0x0E,
    CTRL_CMD_RESET_PEDOMETER         = 0x0F,
    CTRL_CMD_COPY_USID               = 0x10,
    CTRL_CMD_SET_RPU                 = 0x11,
    CTRL_CMD_AHB_CLOCK_GATING        = 0x12,
    CTRL_CMD_ON_DEMAND_CALIBRATION   = 0xA2,
    CTRL_CMD_APPLY_GYRO_GAINS        = 0xAA,
} ctrl9_cmds_t;

typedef struct st_driver
{
    qmi86_dev_t       dev;
    qmi86_sensor_mode dev_mode;
    uint8_t           dev_id;
    bool_t            is_initialized;
    uint16_t          acc_sensitivity;
    uint16_t          gyro_sensitivity;
} driver_t;

static driver_t g_qmi_drv[QMI86_DEV_CNT];

/**
 * @brief This internal function writes data to a specific register of the
 * QMI8658 device over I2C.
 * @param[in,out] ppt_dev QMI8658 device instance.
 * @param[in] ppt_data Pointer to the data buffer to write.
 * @param[in] p_data_sz Size of the data to write in bytes.
 * @param[in] p_reg_addr The register address to write to in the sensor.
 * @return Result of the execution status.
 */
static response_status_t write_register(qmi86_dev_t* ppt_dev, uint8_t* ppt_data, size_t p_data_sz,
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
 * QMI8658 device over I2C.
 * @param[in,out] ppt_dev QMI8658 device instance.
 * @param[out] ppt_data Pointer to the data buffer to read into.
 * @param[in] p_data_sz Size of the data to read in bytes.
 * @param[in] p_reg_addr The register address to read from in the sensor.
 * @return Result of the execution status.
 */
static response_status_t read_register(qmi86_dev_t* ppt_dev, uint8_t* ppt_data, size_t p_data_sz,
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

static response_status_t wait_for_statusint_bit(qmi86_dev_t* ppt_dev, uint8_t p_desired_bit_val,
                                                uint8_t p_bit_no)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_desired_bit_val > 0x01, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_bit_no > 0x07, RET_PARAM_ERROR);

    response_status_t ret_val          = RET_OK;
    bool_t            bit_ok           = FALSE;
    uint8_t           reg_data         = 0x00;
    uint8_t           max_retry_cnt    = 50U;
    const uint8_t     delay_per_try_ms = 100U;

    while (ret_val == RET_OK && bit_ok == FALSE)
    {
        ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_STATUSINT);
        bit_ok  = (((reg_data & (1 << p_bit_no)) >> p_bit_no) == p_desired_bit_val);

        max_retry_cnt--;

        if (bit_ok == TRUE)
        {
            break;
        }
        else if (max_retry_cnt == 0)
        {
            ret_val = RET_TIMEOUT;
            break;
        }

        ha_timer_hard_delay_ms(delay_per_try_ms);
    }

    return ret_val;
}

static response_status_t exec_ctrl9_cmd(qmi86_dev_t* ppt_dev, ctrl9_cmds_t p_cmd,
                                        uint16_t p_cmd_time_ms)
{
    response_status_t ret_val  = RET_OK;
    uint8_t           reg_data = 0x00;

    // 2. Write Ctrl9 register 0x0A with the appropriate Command value
    reg_data = p_cmd;
    ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL9);

    // 3. The Device will set STATUSINT.bit7 to 1, and raise INT1(if CTRL1.bit3
    // = 1 & CTRL8.bit7 == 0), once it has executed the appropriate function
    // based on the command value
    if (ret_val == RET_OK)
    {
        ha_timer_hard_delay_ms(p_cmd_time_ms);
        ret_val = wait_for_statusint_bit(ppt_dev, 0x01, QMI86_REG_STATUSINT_CMD_DONE_POS);
    }

    // 4. The host must acknowledge this by writing CTRL_CMD_ACK (0x00) to CTRL9
    // register
    if (ret_val == RET_OK)
    {
        reg_data = CTRL_CMD_ACK;
        ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL9);
    }
    // 4. STATUSINT.bit7 (CmdDone) will be reset to 0 on receiving the
    // CTRL_CMD_ACK command.
    if (ret_val == RET_OK)
    {
        ret_val = wait_for_statusint_bit(ppt_dev, 0x00, QMI86_REG_STATUSINT_CMD_DONE_POS);
    }

    return ret_val;
}

static response_status_t init_dev_regs(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    uint8_t           reg_data = 0x00;
    response_status_t ret_val  = RET_OK;

    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL1_ADDR_AI, 0x01);
    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL1_BE, 0x00);
    ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL1);

    reg_data = 0x00;
    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL8_CTRL9_HANDSHAKE, 0x01);
    ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL9);

    if (ret_val == RET_OK)
    {
        reg_data = 0x01;
        ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CAL1_L);
    }

    if (ret_val == RET_OK)
    {
        ret_val = exec_ctrl9_cmd(ppt_dev, CTRL_CMD_AHB_CLOCK_GATING, 2000U);
    }
    return ret_val;
}

static bool_t is_dev_ready(qmi86_dev_t* ppt_dev)
{
    response_status_t ret_val  = RET_OK;
    uint8_t           who_am_i = 0x00;
    uint8_t           chip_id  = 0x00;

    ret_val  = read_register(ppt_dev, &who_am_i, 1U, QMI86_REG_WHO_AM_I);
    ret_val |= read_register(ppt_dev, &chip_id, 1U, QMI86_REG_REVISION_ID);

    return ((ret_val == RET_OK) && ((chip_id == QMI86_CHIP_ID) && (who_am_i == QMI86_WHO_AM_I)));
}

static response_status_t read_firmware_version(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val  = read_register(ppt_dev, &ppt_dev->chip_fw_version.u8_arr[0], 1, QMI86_REG_DQW_L);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_fw_version.u8_arr[1], 1, QMI86_REG_DQW_H);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_fw_version.u8_arr[2], 1, QMI86_REG_DQX_L);

    return ret_val;
}

static response_status_t read_chip_id(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val  = read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[0], 1, QMI86_REG_DVX_L);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[1], 1, QMI86_REG_DVX_H);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[2], 1, QMI86_REG_DVY_L);
    ret_val |= read_register(ppt_dev, &ppt_dev->chip_id.u8_arr[3], 1, QMI86_REG_DVY_H);

    return ret_val;
}

static qmi86_st_result self_test_gyro(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, QMI86_ST_RESULT_API_ERROR);

    response_status_t api_ret_val = RET_ERROR;
    response_status_t bkp_ret_val = RET_ERROR;
    qmi86_st_result   ret_val     = QMI86_ST_RESULT_FAILED;
    uint8_t           ctrl7_bkp   = 0x00;
    uint8_t           reg_data    = 0x00;
    // The typical time for Self-Test costs about 25 ODRs as odr selected for st
    // is 1Khz then total time is 25Ms
    const uint16_t st_total_time_ms = 500U;
    uint8_t        st_result[6]     = { 0x00 };
    int16_t        st_x_dps         = 0U;
    int16_t        st_y_dps         = 0U;
    int16_t        st_z_dps         = 0U;

    bkp_ret_val = read_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);

    // 1. Disable the sensors (CTRL7 = 0x00).
    if (bkp_ret_val == RET_OK)
    {
        reg_data    = 0x00;
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
    }

    // 2. Set the bit gST to 1. (CTRL3.bit7 = 1’b1).
    api_ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL3);
    if (api_ret_val == RET_OK)
    {
        reg_data    = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL3_GST, 0x01);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL3);
    }

    // 3. Wait for QMI8658A to drive INT2 to High, if INT2 is enabled
    // (CTRL1.bit4 = 1), or STATUSINT.bit0 is set to 1.
    if (api_ret_val == RET_OK)
    {
        reg_data = 0x00;
        ha_timer_hard_delay_ms(st_total_time_ms);
        api_ret_val = wait_for_statusint_bit(ppt_dev, 0x01, QMI86_REG_STATUSINT_AVAIL_POS);
    }

    // 4. Set CTRL3.gST(bit7) to 0, to clear STATUSINT1.bit0 and/or INT2.
    if (api_ret_val == RET_OK)
    {
        reg_data    = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL3_GST, 0x00);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL3);
    }

    // 5. Check for QMI8658A drives INT2 back to Low, and sets STATUSINT1.bit0
    // to 0.
    if (api_ret_val == RET_OK)
    {
        ha_timer_hard_delay_ms(st_total_time_ms);
        api_ret_val = wait_for_statusint_bit(ppt_dev, 0x00, QMI86_REG_STATUSINT_AVAIL_POS);
    }

    // 6. Read the Accel Self-Test result:
    if (api_ret_val == RET_OK)
    {
        api_ret_val = read_register(ppt_dev, st_result, ARRAY_SIZE(st_result), QMI86_REG_DVX_L);
    }

    if (api_ret_val == RET_OK)
    {
        st_x_dps = BYTES_TO_WORD(signed, st_result[0], st_result[1]);
        st_y_dps = BYTES_TO_WORD(signed, st_result[2], st_result[3]);
        st_z_dps = BYTES_TO_WORD(signed, st_result[4], st_result[5]);

        if (abs(st_x_dps) > (300U * (1U << 4U)) && abs(st_y_dps) > (300U * (1U << 4U))
            && abs(st_z_dps) > (300U * (1U << 4U)))
        {
            ret_val = QMI86_ST_RESULT_OK;
        }
        else
        {
            ret_val = QMI86_ST_RESULT_FAILED;
        }
    }

    if (bkp_ret_val == RET_OK)
    {
        write_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
    }

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

static qmi86_st_result self_test_accel(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, QMI86_ST_RESULT_API_ERROR);

    response_status_t api_ret_val = RET_ERROR;
    response_status_t bkp_ret_val = RET_ERROR;
    qmi86_st_result   ret_val     = QMI86_ST_RESULT_FAILED;
    uint8_t           ctrl7_bkp   = 0x00;
    uint8_t           ctrl2_bkp   = 0x00;
    uint8_t           reg_data    = 0x00;
    // The typical time for Self-Test costs about 25 ODRs as odr selected for st
    // is 1Khz then total time is 25Ms
    const uint16_t st_total_time_ms = (QMI86_ST_ODR_CYCLE_CNT * 1U);
    uint8_t        st_result[6]     = { 0x00 };
    int16_t        st_x_mg          = 0U;
    int16_t        st_y_mg          = 0U;
    int16_t        st_z_mg          = 0U;

    bkp_ret_val  = read_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
    bkp_ret_val |= read_register(ppt_dev, &ctrl2_bkp, 1, QMI86_REG_CTRL2);

    // 1. Disable the sensors (CTRL7 = 0x00).
    if (bkp_ret_val == RET_OK)
    {
        reg_data    = 0x00;
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
    }

    // 2. Set proper accelerometer ODR (CTRL2.aODR) and bit CTRL2.aST (bit7) to
    // 1 to trigger the Self-Test.
    api_ret_val = read_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    if (api_ret_val == RET_OK)
    {
        reg_data    = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AODR, QMI86_ACC_ODR_1000_HZ);
        reg_data    = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AST, 0x01);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    }

    // 3. Wait for QMI8658A to drive INT2 to High, if INT2 is enabled
    // (CTRL1.bit4 = 1), or STATUSINT.bit0 is set to 1.
    if (api_ret_val == RET_OK)
    {
        reg_data = 0x00;
        ha_timer_hard_delay_ms(st_total_time_ms);
        api_ret_val = wait_for_statusint_bit(ppt_dev, 0x01, QMI86_REG_STATUSINT_AVAIL_POS);
    }

    // 4. Set CTRL2.aST(bit7) to 0, to clear STATUSINT1.bit0 and/or INT2.
    if (api_ret_val == RET_OK)
    {
        reg_data    = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AST, 0x00);
        api_ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);
    }

    // 5. Check for QMI8658A drives INT2 back to Low, and sets STATUSINT1.bit0
    // to 0.
    if (api_ret_val == RET_OK)
    {
        api_ret_val = wait_for_statusint_bit(ppt_dev, 0x00, QMI86_REG_STATUSINT_AVAIL_POS);
    }

    // 6. Read the Accel Self-Test result:
    if (api_ret_val == RET_OK)
    {
        api_ret_val = read_register(ppt_dev, st_result, ARRAY_SIZE(st_result), QMI86_REG_DVX_L);
    }

    if (api_ret_val == RET_OK)
    {
        st_x_mg = BYTES_TO_WORD(signed, st_result[0], st_result[1]);
        st_y_mg = BYTES_TO_WORD(signed, st_result[2], st_result[3]);
        st_z_mg = BYTES_TO_WORD(signed, st_result[4], st_result[5]);

        if (abs(st_x_mg) > (200 * 2) && abs(st_y_mg) > (200 * 2) && abs(st_z_mg) > (200 * 2))
        {
            ret_val = QMI86_ST_RESULT_OK;
        }
        else
        {
            ret_val = QMI86_ST_RESULT_FAILED;
        }
    }

    if (bkp_ret_val == RET_OK)
    {
        write_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
        write_register(ppt_dev, &ctrl2_bkp, 1, QMI86_REG_CTRL2);
    }

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

response_status_t dd_qmi86_set_data_settings(qmi86_dev_t* ppt_dev)
{
    // TODO: Implement Me!
    //! Refer to 13.5
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val  = RET_OK;
    uint8_t           reg_data = 0x00;

    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AFS, ppt_dev->settings.data_settings.acc_fsr);
    reg_data =
      QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AODR, ppt_dev->settings.data_settings.acc_odr);
    ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL2);

    if (ret_val == RET_OK)
    {
        ((driver_t*)ppt_dev)->acc_sensitivity =
          (1 << (ACC_SCALE_SENSITIVITY_MAX - ppt_dev->settings.data_settings.acc_fsr));

        reg_data =
          QMI_OWR_BITS(reg_data, QMI86_REG_CTRL3_GFS, ppt_dev->settings.data_settings.gyro_fsr);
        reg_data =
          QMI_OWR_BITS(reg_data, QMI86_REG_CTRL3_GODR, ppt_dev->settings.data_settings.gyro_odr);
        ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL3);
    }

    if (ret_val == RET_OK)
    {
        ((driver_t*)ppt_dev)->gyro_sensitivity =
          (1 << (GYRO_SCALE_SENSITIVITY_MAX - ppt_dev->settings.data_settings.gyro_fsr));

        reg_data = QMI_OWR_BITS(reg_data,
                                QMI86_REG_CTRL5_ALPF_EN,
                                ppt_dev->settings.data_settings.acc_lpf_en);
        reg_data = QMI_OWR_BITS(reg_data,
                                QMI86_REG_CTRL5_ALPF_MODE,
                                ppt_dev->settings.data_settings.acc_lpf_mode);
        reg_data = QMI_OWR_BITS(reg_data,
                                QMI86_REG_CTRL5_GLPF_EN,
                                ppt_dev->settings.data_settings.gyro_lpf_en);
        reg_data = QMI_OWR_BITS(reg_data,
                                QMI86_REG_CTRL5_GLPF_MODE,
                                ppt_dev->settings.data_settings.gyro_lpf_mode);
        ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL5);
    }

    return ret_val;
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

response_status_t dd_qmi86_get_data_settings(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t              ret_val        = RET_OK;
    uint8_t                        reg_content[3] = { 0x00 };
    struct st_qmi86_data_settings* hndlr          = &ppt_dev->settings.data_settings;

    ret_val  = read_register(ppt_dev, &reg_content[0], 1, QMI86_REG_CTRL2);
    ret_val |= read_register(ppt_dev, &reg_content[1], 1, QMI86_REG_CTRL3);
    ret_val |= read_register(ppt_dev, &reg_content[2], 1, QMI86_REG_CTRL5);

    if (ret_val == RET_OK)
    {
        hndlr->acc_odr = QMI_GET_BITS(reg_content[0], QMI86_REG_CTRL2_AODR);
        hndlr->acc_fsr = QMI_GET_BITS(reg_content[0], QMI86_REG_CTRL2_AFS);

        hndlr->gyro_odr = QMI_GET_BITS(reg_content[1], QMI86_REG_CTRL3_GODR);
        hndlr->gyro_fsr = QMI_GET_BITS(reg_content[1], QMI86_REG_CTRL3_GFS);

        hndlr->acc_lpf_en   = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_ALPF_EN);
        hndlr->acc_lpf_mode = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_ALPF_MODE);

        hndlr->gyro_lpf_en   = QMI_GET_BITS(reg_content[2], QMI86_REG_CTRL5_GLPF_EN);
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

    response_status_t             ret_val     = RET_OK;
    uint8_t                       reg_content = 0x00;
    struct st_qmi86_ifc_settings* hndlr       = &ppt_dev->settings.comm_ifc_settings;

    ret_val = read_register(ppt_dev, &reg_content, 1, QMI86_REG_CTRL1);

    if (ret_val == RET_OK)
    {
        hndlr->spi_mode = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_SIM);
    }
    else
    {
        // Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_get_interrupt_settings(qmi86_dev_t* ppt_dev)
{
    response_status_t             ret_val           = RET_OK;
    uint8_t                       ctrl1_reg_content = 0x00;
    uint8_t                       ctrl7_reg_content = 0x00;
    struct st_qmi86_int_settings* hndlr             = &ppt_dev->settings.interrupt_settings;

    ret_val  = read_register(ppt_dev, &ctrl1_reg_content, 1, QMI86_REG_CTRL1);
    ret_val |= read_register(ppt_dev, &ctrl7_reg_content, 1, QMI86_REG_CTRL7);

    if (ret_val == RET_OK)
    {
        hndlr->fifo_int_output = QMI_GET_BITS(ctrl1_reg_content, QMI86_REG_CTRL1_FIFO_INT_SEL);
        hndlr->int1_enable     = QMI_GET_BITS(ctrl1_reg_content, QMI86_REG_CTRL1_INT1_EN);
        hndlr->int2_enable     = QMI_GET_BITS(ctrl1_reg_content, QMI86_REG_CTRL1_INT2_EN);
        hndlr->drdy_enable     = !QMI_GET_BITS(ctrl7_reg_content, QMI86_REG_CTRL7_DRDY_DIS);
    }
    else
    {
        // Nothing to do
    }

    return ret_val;
}

response_status_t dd_qmi86_snooze_gyro(qmi86_dev_t* ppt_dev, bool_t p_snooze)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    uint8_t           ctrl7_reg_data = 0x00;
    response_status_t ret_val        = RET_OK;

    ret_val = read_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);

    if (ret_val == RET_OK)
    {
        ctrl7_reg_data = QMI_OWR_BITS(ctrl7_reg_data, QMI86_REG_CTRL7_GSN, p_snooze);
        ret_val        = write_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);
    }
    else
    {
        // Nothing to do
    }

    return ret_val;
}

static response_status_t enable_sensor(qmi86_dev_t* ppt_dev, qmi86_sensor_mode p_sensor_type)
{
    uint8_t ctrl1_reg_data                             = 0x00;
    uint8_t ctrl7_reg_data                             = 0x00;
    uint8_t sys_delays[QMI86_SENSOR_MODE_ACC_GYRO + 1] = {
        QMI86_POWER_OFF_TIME,
        QMI86_ACCEL_POWER_ON_TIME,
        QMI86_GYRO_POWER_ON_TIME,
        // ignore accel turn on time as it's much lower than gyro
        QMI86_GYRO_POWER_ON_TIME
    };
    uint8_t           all_disabled = (p_sensor_type == QMI86_SENSOR_DISABLE);
    response_status_t ret_val      = RET_OK;

    // Save register contents
    ret_val  = read_register(ppt_dev, &ctrl1_reg_data, 1, QMI86_REG_CTRL1);
    ret_val |= read_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);

    if (ret_val == RET_OK)
    {
        ctrl1_reg_data = QMI_OWR_BITS(ctrl1_reg_data, QMI86_REG_CTRL1_SENS_DISABLE, all_disabled);
        ctrl7_reg_data = QMI_OWR_BITS(ctrl7_reg_data, QMI86_REG_CTRL7_AGEN, p_sensor_type);

        ret_val |= write_register(ppt_dev, &ctrl1_reg_data, 1, QMI86_REG_CTRL1);
        ret_val |= write_register(ppt_dev, &ctrl7_reg_data, 1, QMI86_REG_CTRL7);

        // Wait for process
        ha_timer_hard_delay_ms(sys_delays[p_sensor_type]);
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

    ((driver_t*)ppt_dev)->dev_mode = p_dev_mode;

    return RET_OK;
}

qmi86_sensor_mode dd_qmi86_get_device_mode(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, QMI86_SENSOR_DISABLE);

    return ((driver_t*)ppt_dev)->dev_mode;
}

qmi86_dev_t* dd_qmi86_get_dev(qmi86_dev_id_t p_dev_id)
{
    return &g_qmi_drv[p_dev_id].dev;
}

response_status_t dd_qmi86_calibrate_accel(qmi86_dev_t* ppt_dev)
{
    (void)ppt_dev;
    return RET_NOT_SUPPORTED;
}

response_status_t dd_qmi86_calibrate_gyro(qmi86_dev_t*             ppt_dev,
                                          union gyro_calib_result* p_result_hndlr)
{
    uint8_t           ctrl7_bkp     = 0x00;
    uint8_t           reg_data      = 0x00;
    uint8_t           calib_data[6] = { 0x00 };
    response_status_t ret_val       = RET_OK;
    p_result_hndlr->result_code     = 0x00;

    /*
     * First run COD
     * The Calibration-On-Demand supports the on-demand calibration of Gyro X
     * and Y axes. Note that the Z axis of gyroscope is not influenced by COD.
     */

    // 1. Set CTRL7.aEN = 0 and CTRL7.gEN = 0, to disable the accelerometer and
    // gyroscope.
    ret_val = read_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
    if (ret_val == RET_OK)
    {
        reg_data = QMI_OWR_BITS(ctrl7_bkp, QMI86_REG_CTRL7_AGEN, 0x00);
        ret_val  = read_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
    }

    // 2. Issue the CTRL_CMD_ON_DEMAND_CALIBRATION (0xA2) by CTRL9 command.
    // 3. Wait about 1.5 seconds for QMI8658A to finish the CTRL9 command.
    if (ret_val == RET_OK)
    {
        ret_val = exec_ctrl9_cmd(ppt_dev, CTRL_CMD_ON_DEMAND_CALIBRATION, 2000U);
    }

    if (ret_val == RET_OK)
    {
        reg_data = 0x00;
        ret_val  = read_register(ppt_dev, &reg_data, 1, QMI86_REG_COD_STATUS);
    }

    if (ret_val == RET_OK)
    {
        p_result_hndlr->result_code = reg_data;
    }

    if (p_result_hndlr->result_code == 0x00)
    {
        ret_val = read_register(ppt_dev, calib_data, 6U, QMI86_REG_DVX_L);
        if (ret_val == RET_OK)
        {
            ppt_dev->clib_params.gyro_data.x_gain =
              BYTES_TO_WORD(signed, calib_data[0], calib_data[1]);
            ppt_dev->clib_params.gyro_data.y_gain =
              BYTES_TO_WORD(signed, calib_data[2], calib_data[3]);
            ppt_dev->clib_params.gyro_data.z_gain =
              BYTES_TO_WORD(signed, calib_data[4], calib_data[5]);
        }
        else
        {
            ppt_dev->clib_params.gyro_data.x_gain = 0x00;
            ppt_dev->clib_params.gyro_data.y_gain = 0x00;
            ppt_dev->clib_params.gyro_data.z_gain = 0x00;
        }
    }

    return ret_val;
}

qmi86_st_result dd_qmi86_perform_self_test(qmi86_dev_t* ppt_dev, qmi86_sensors_t p_sensor_type)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, QMI86_ST_RESULT_API_ERROR);
    ASSERT_AND_RETURN(p_sensor_type >= QMI86_SENSORS_CNT, QMI86_ST_RESULT_API_ERROR);

    qmi86_st_result (* const self_testers[QMI86_SENSORS_CNT])(
      qmi86_dev_t* ppt_dev) = { self_test_gyro, self_test_accel };
    qmi86_st_result ret_val = QMI86_ST_RESULT_OK;

    ret_val = self_testers[p_sensor_type](ppt_dev);

    return ret_val;
}

response_status_t dd_qmi86_reset_device(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    uint8_t           reg_data = QMI86_RESET_CMD;
    response_status_t ret_val  = RET_OK;
    bool_t            reset_ok = FALSE;

    ret_val = write_register(ppt_dev, &reg_data, 1, QMI86_REG_RESET);

    if (ret_val == RET_OK)
    {
        // Wait until device reset
        ha_timer_hard_delay_ms(QMI86_SYS_POWER_ON_TIME);
        reg_data = 0x00;
        ret_val  = read_register(ppt_dev, &reg_data, 1, QMI86_REG_RESET_RESULT);
        reset_ok = (reg_data == QMI86_RESET_SUCCESSFUL);
    }
    else
    {
        // Nothing to do
    }

    if (ret_val == RET_OK && reset_ok == FALSE)
    {
        ret_val = RET_ERROR;
    }
    else
    {
        // Nothing to do
    }

    return ret_val;
}

static response_status_t set_data_regs_lock(qmi86_dev_t* ppt_dev, bool_t lock_status)
{
    response_status_t ret_val   = RET_OK;
    uint8_t           reg_data  = 0x00;
    uint8_t           ctrl7_bkp = 0x00;

    ret_val = read_register(ppt_dev, &ctrl7_bkp, 1, QMI86_REG_CTRL7);
    if (lock_status == TRUE)
    {
        // 1. Disable internal AHB
        // if (ret_val == RET_OK)
        // {
        //     reg_data = 0x01;
        //     ret_val  = write_register(ppt_dev, &reg_data, 1,
        //     QMI86_REG_CAL1_L);
        // }

        // if (ret_val == RET_OK)
        // {
        //     ret_val = exec_ctrl9_cmd(ppt_dev, CTRL_CMD_AHB_CLOCK_GATING,
        //     2000U);
        // }

        // 2. Enable locking mechanism
        if (ret_val == RET_OK)
        {
            reg_data = QMI_OWR_BITS(ctrl7_bkp, QMI86_REG_CTRL7_SYNC_SAMPLE, 0x01);
            ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
        }
        if (ret_val == RET_OK)
        {
            ret_val = enable_sensor(ppt_dev, ((driver_t*)ppt_dev)->dev_mode);
        }

        // 3. Wait until statusint.avail=1
        if (ret_val == RET_OK)
        {
            ret_val = wait_for_statusint_bit(ppt_dev, 0x01, QMI86_REG_STATUSINT_AVAIL_POS);
        }

        // 4. Wait until statusint.locked=1
        if (ret_val == RET_OK)
        {
            ret_val = wait_for_statusint_bit(ppt_dev, 0x01, QMI86_REG_STATUSINT_LOCKED_POS);
        }

        // return
    }
    else
    {
        // 1. Disable locking mechanism
        if (ret_val == RET_OK)
        {
            reg_data = QMI_OWR_BITS(ctrl7_bkp, QMI86_REG_CTRL7_SYNC_SAMPLE, 0x00);
            ret_val  = write_register(ppt_dev, &reg_data, 1, QMI86_REG_CTRL7);
        }
        if (ret_val == RET_OK)
        {
            ret_val = enable_sensor(ppt_dev, QMI86_SENSOR_DISABLE);
        }

        // // 2. Enable internal AHB
        // if (ret_val == RET_OK)
        // {
        //     reg_data = 0x00;
        //     ret_val  = write_register(ppt_dev, &reg_data, 1,
        //     QMI86_REG_CAL1_L);
        // }

        // if (ret_val == RET_OK)
        // {
        //     ret_val = exec_ctrl9_cmd(ppt_dev, CTRL_CMD_AHB_CLOCK_GATING,
        //     2000U);
        // }
    }

    return ret_val;
}

response_status_t dd_qmi86_poll_data(qmi86_dev_t* ppt_dev)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val          = RET_OK;
    uint8_t           regs_data[12]    = { 0x00 };
    int16_t           gyro_raw_data[3] = { 0x00 };
    int16_t           acc_raw_data[3]  = { 0x00 };

    ret_val = set_data_regs_lock(ppt_dev, TRUE);

    if (ret_val == RET_OK)
    {
        ret_val = read_register(ppt_dev, regs_data, ARRAY_SIZE(regs_data), QMI86_REG_AX_L);
    }

    if (ret_val == RET_OK)
    {
        acc_raw_data[0] = BYTES_TO_WORD(signed, regs_data[0], regs_data[1]);
        acc_raw_data[1] = BYTES_TO_WORD(signed, regs_data[2], regs_data[3]);
        acc_raw_data[2] = BYTES_TO_WORD(signed, regs_data[4], regs_data[5]);

        gyro_raw_data[0] = BYTES_TO_WORD(signed, regs_data[6], regs_data[7]);
        gyro_raw_data[1] = BYTES_TO_WORD(signed, regs_data[8], regs_data[9]);
        gyro_raw_data[2] = BYTES_TO_WORD(signed, regs_data[10], regs_data[11]);

        ppt_dev->data.gyro.x =
          (float)gyro_raw_data[0] / (float)((driver_t*)ppt_dev)->gyro_sensitivity;
        ppt_dev->data.gyro.y =
          (float)gyro_raw_data[1] / (float)((driver_t*)ppt_dev)->gyro_sensitivity;
        ppt_dev->data.gyro.z =
          (float)gyro_raw_data[2] / (float)((driver_t*)ppt_dev)->gyro_sensitivity;

        ppt_dev->data.accel.x =
          (float)acc_raw_data[0] / (float)((driver_t*)ppt_dev)->acc_sensitivity;
        ppt_dev->data.accel.y =
          (float)acc_raw_data[1] / (float)((driver_t*)ppt_dev)->acc_sensitivity;
        ppt_dev->data.accel.z =
          (float)acc_raw_data[2] / (float)((driver_t*)ppt_dev)->acc_sensitivity;
    }

    if (ret_val == RET_OK)
    {
        ret_val = set_data_regs_lock(ppt_dev, FALSE);
    }

    return ret_val;
}

response_status_t dd_qmi86_init(qmi86_dev_t** ppt_dev, qmi86_dev_id_t p_dev_id)
{
    ASSERT_AND_RETURN(ppt_dev == NULL, RET_PARAM_ERROR);

    response_status_t ret_val     = RET_OK;
    driver_t*         pt_curr_drv = &g_qmi_drv[p_dev_id];
    qmi86_dev_t*      pt_curr_dev = &(pt_curr_drv->dev);

    ret_val = ha_iic_init();

    if (ret_val == RET_OK)
    {
        memset(pt_curr_drv, 0U, sizeof(driver_t));
        pt_curr_drv->dev_id = p_dev_id;

        if (is_dev_ready(pt_curr_dev) == TRUE && init_dev_regs(pt_curr_dev) == RET_OK)
        {
            pt_curr_drv->is_initialized = TRUE;
            ret_val                    |= read_firmware_version(pt_curr_dev);
            ret_val                    |= read_chip_id(pt_curr_dev);
            ret_val                    |= dd_qmi86_get_data_settings(pt_curr_dev);
            ret_val                    |= dd_qmi86_get_interface_settings(pt_curr_dev);
            ret_val                    |= dd_qmi86_get_interrupt_settings(pt_curr_dev);
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
        *ppt_dev                    = pt_curr_dev;
    }
    else
    {
        pt_curr_drv->is_initialized = FALSE;
        *ppt_dev                    = NULL;
    }

    return ret_val;
}
