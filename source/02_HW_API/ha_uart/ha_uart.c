#include "ha_uart.h"

#include "ha_uart/ha_uart_private.h"
#include "mp_uart/mp_uart.h"
#include "stddef.h"
#include "su_common.h"

static uart_driver g_pt_uart_drv    = NULL;
static bool_t      g_uart_drv_ready = FALSE;

/**
 * @brief This function transmits data over a UART communication port.
 *
 * @param[in] p_port UART communication port to use.
 * @param[in] ppt_data_buffer Pointer to the data buffer to send. Should not be
 * NULL.
 * @param[in] p_data_size Data buffer size in bytes. Should not be zero.
 * @param[in] p_timeout Timeout for the operation in milliseconds. Should not be
 * zero.
 * @return Result of the execution status.
 */
response_status_t ha_uart_transmit(uart_comm_port_t p_port, uint8_t* ppt_data_buffer,
                                   size_t p_data_size, timeout_t p_timeout)
{
    ASSERT_AND_RETURN(g_uart_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_uart_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_data_size == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_uart_drv->api->transmit(p_port, ppt_data_buffer, p_data_size, p_timeout);
    return ret_val;
}

/**
 * @brief This function receives data from a UART communication port.
 *
 * @param[in] p_port UART communication port to use.
 * @param[out] ppt_data_buffer Pointer to the data buffer to write received
 * data. Should not be NULL.
 * @param[in] p_data_size The length of the data requested in bytes. Should not
 * be zero.
 * @param[in] p_timeout Timeout for the operation in milliseconds. Should not be
 * zero.
 * @return Result of the execution status.
 */
response_status_t ha_uart_receive(uart_comm_port_t p_port, uint8_t* ppt_data_buffer,
                                  size_t p_data_size, timeout_t p_timeout)
{
    ASSERT_AND_RETURN(g_uart_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_uart_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_data_size == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_uart_drv->api->receive(p_port, ppt_data_buffer, p_data_size, p_timeout);

    return ret_val;
}

response_status_t ha_uart_dma_transmit(uart_comm_port_t p_port, uint8_t* ppt_data_buffer,
                                       size_t p_data_size)
{
    ASSERT_AND_RETURN(g_uart_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_uart_drv->hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_data_size == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_uart_drv->api->dma_transmit_request(p_port, ppt_data_buffer, p_data_size);

    return ret_val;
}

response_status_t ha_uart_dma_stop(uart_comm_port_t p_port)
{
    ASSERT_AND_RETURN(g_uart_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_uart_drv->hw_inst_cnt, RET_NOT_SUPPORTED);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_uart_drv->api->dma_transmit_abort(p_port);

    return ret_val;
}

response_status_t ha_uart_dma_register_callback(uart_comm_port_t p_port, uart_dma_evt_cb ppt_evt_cb)
{
    ASSERT_AND_RETURN(g_uart_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_port >= g_pt_uart_drv->hw_inst_cnt, RET_NOT_SUPPORTED);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_uart_drv->api->dma_register_cb(p_port, (dma_tx_evt_cb)ppt_evt_cb);

    return ret_val;
}

/**
 * @brief This function initializes the UART driver once per power cycle.
 * It has no consequences for multiple calls.
 *
 * @return Result of the operation.
 */
response_status_t ha_uart_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_uart_drv_ready != TRUE)
    {
        g_pt_uart_drv = uart_driver_register();

        if (g_pt_uart_drv == NULL)
        {
            ret_val = RET_ERROR;
        }
        else
        {
            g_pt_uart_drv->hw_inst_cnt = 0;
            ret_val                    = g_pt_uart_drv->api->init();
            if (ret_val == RET_OK)
            {
                g_uart_drv_ready = TRUE;
            }
        }
    }

    return ret_val;
}
