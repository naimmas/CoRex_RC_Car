
#include "dd_qmi86.h"
#include "dd_qmi86_defs.h"
#include "ha_iic/ha_iic.h"
#include "ha_timer/ha_timer.h"

#define DEFAULT_IIC_TIMEOUT (100U) // Default I2C timeout in milliseconds

#define QMI_SET_BITS(bitname, data) ((data) << (bitname##_POS))

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

response_status_t dd_qmi86_set_data_settings(qmi86_dev_t* ppt_dev)
{
  /*TODO: Implement Me!*/
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
        ctrl7_reg_data |= QMI_SET_BITS(QMI86_REG_CTRL7_GSN, p_snooze);
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
        ctrl1_reg_data |= QMI_SET_BITS(QMI86_REG_CTRL1_SENS_DISABLE, all_disabled);
        ctrl7_reg_data |= QMI_SET_BITS(QMI86_REG_CTRL7_AGEN, p_dev_mode);

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
        hndlr->address_auto_increment = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_ADDR_AI);
        hndlr->data_format = QMI_GET_BITS(reg_content, QMI86_REG_CTRL1_BE);
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

response_status_t dd_qmi86_perform_self_test(qmi86_dev_t* ppt_dev)
{
  /*TODO: Implement Me!*/
  (void)ppt_dev;
  return (response_status_t)0;
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

        if (is_dev_ready(pt_curr_dev))
        {
            pt_curr_drv->is_initialized = TRUE;
            ret_val |= read_firmware_version(pt_curr_dev);
            ret_val |= read_chip_id(pt_curr_dev);
            ret_val |= dd_qmi86_get_data_settings(pt_curr_dev);
            ret_val |= dd_qmi86_get_dev_settings(pt_curr_dev);
            ret_val |= dd_qmi86_get_ifc_settings(pt_curr_dev);
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

