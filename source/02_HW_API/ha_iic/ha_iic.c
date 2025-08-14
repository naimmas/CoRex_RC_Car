#include "ha_iic.h"

#include "ha_iic_private.h"
#include "mp_iic/mp_iic.h"
#include "stddef.h"
#include "su_common.h"

static iic_driver g_pt_iic_drv          = NULL;
static bool_t     g_iic_drv_ready = FALSE;

/**
 * @brief This function sends data to an I2C device.
 * @param[in] p_port I2C communication port.
 * @param[in] p_slave_addr The address of the I2C device to write to.
 * @param[in] ppt_data_buffer Pointer to the data buffer to send. Should not be
 * NULL.
 * @param[in] p_data_size Data buffer size in bytes. Should not be zero.
 * @param[in] p_timeout_ms Timeout for the operation in milliseconds. Should not
 * be zero.
 * @return Result of the execution status.
 */
response_status_t ha_iic_master_write(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                      const uint8_t* ppt_data_buffer, size_t p_data_size,
                                      timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_iic_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_data_size == 0, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timeout_ms == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val =
      g_pt_iic_drv->api->write(p_port, p_slave_addr, ppt_data_buffer, p_data_size, p_timeout_ms);

    return ret_val;
}

/**
 * @brief This function requests data from an I2C device.
 * @param[in] p_port I2C communication port.
 * @param[in] p_slave_addr The address of the I2C device to read from.
 * @param[out] ppt_data_buffer Pointer to the data buffer to write received
 * data. Should not be NULL.
 * @param[in] p_data_size The length of the data requested in bytes. Should not
 * be zero.
 * @param[in] p_timeout_ms Timeout for the operation in milliseconds. Should not
 * be zero.
 * @return Result of the execution status.
 */
response_status_t ha_iic_master_read(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                     uint8_t* ppt_data_buffer, size_t p_data_size,
                                     timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_iic_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_data_size == 0, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timeout_ms == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val =
      g_pt_iic_drv->api->read(p_port, p_slave_addr, ppt_data_buffer, p_data_size, p_timeout_ms);

    return ret_val;
}

/**
 * @brief This function writes data to a specific memory address of an I2C
 * device.
 * @param[in] p_port I2C communication port.
 * @param[in] p_slave_addr The address of the I2C device to write to.
 * @param[in] ppt_data_buffer Pointer to the data buffer to send. Should not be
 * NULL.
 * @param[in] p_data_size Data buffer size in bytes. Should not be zero.
 * @param[in] p_mem_addr The memory address to write to in the I2C device.
 * @param[in] p_mem_size The size of the memory address, either 8 or 16 bits.
 * @param[in] p_timeout_ms Timeout for the operation in milliseconds. Should not
 * be zero.
 * @return Result of the execution status.
 */
response_status_t ha_iic_master_mem_write(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                          const uint8_t* ppt_data_buffer, size_t p_data_size,
                                          uint16_t p_mem_addr, i2c_mem_size_t p_mem_size,
                                          timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_iic_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_data_size == 0, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timeout_ms == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_iic_drv->api->mem_write(p_port,
                                          p_slave_addr,
                                          p_mem_addr,
                                          p_mem_size,
                                          ppt_data_buffer,
                                          p_data_size,
                                          p_timeout_ms);

    return ret_val;
}

/**
 * @brief This function reads data from a specific memory address of an I2C
 * device.
 * @param[in] p_port I2C communication port.
 * @param[in] p_slave_addr The address of the I2C device to read from.
 * @param[out] ppt_data_buffer Pointer to the data buffer to write received
 * data. Should not be NULL.
 * @param[in] p_data_size The length of the data requested in bytes. Should not
 * be zero.
 * @param[in] p_mem_addr The memory address to read from in the I2C device.
 * @param[in] p_mem_size The size of the memory address, either 8 or 16 bits.
 * @param[in] p_timeout_ms Timeout for the operation in milliseconds. Should not
 * be zero.
 * @return Result of the execution status.
 */
response_status_t ha_iic_master_mem_read(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                         uint8_t* ppt_data_buffer, size_t p_data_size,
                                         uint16_t p_mem_addr, i2c_mem_size_t p_mem_size,
                                         timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_iic_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_mem_size == 0, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timeout_ms == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_iic_drv->api->mem_read(p_port,
                                         p_slave_addr,
                                         p_mem_addr,
                                         p_mem_size,
                                         ppt_data_buffer,
                                         p_data_size,
                                         p_timeout_ms);

    return ret_val;
}

/**
 * @todo This function is not implemented yet.
 * @retval RET_NOT_SUPPORTED
 */
response_status_t ha_iic_bus_recover(iic_comm_port_t p_port)
{
    (void)p_port; // Suppress unused parameter warning
    return RET_NOT_SUPPORTED;
}

/**
 * @brief This function checks if an I2C device is ready.
 * @param[in] p_port I2C communication port.
 * @param[in] p_dev_addr The address of the I2C device to check.
 * @param[in] p_timeout_ms Timeout for the operation in milliseconds. Should not
 * be zero.
 * @return Result of the execution status.
 */
response_status_t ha_iic_dev_check(iic_comm_port_t p_port, uint8_t p_dev_addr,
                                   timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_iic_drv->hw_inst_cnt, RET_NOT_SUPPORTED);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_iic_drv->api->dev_check(p_port, p_dev_addr, p_timeout_ms);

    return ret_val;
}

/**
 * @brief This function initializes the I2C interface.
 * It shall get all hardware instances information from the MCU SDK.
 * @return Result of the execution status.
 */
response_status_t ha_iic_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_iic_drv_ready != TRUE)
    {
        g_pt_iic_drv = iic_driver_register();

        if (g_pt_iic_drv == NULL)
        {
            ret_val = RET_ERROR;
        }
        else
        {
            g_pt_iic_drv->hw_inst_cnt = 0;
            ret_val                  = g_pt_iic_drv->api->init();
            if (ret_val == RET_OK)
            {
                g_iic_drv_ready = TRUE;
            }
        }
    }

    return ret_val;
}
