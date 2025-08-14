#include "mp_uart.h"

#include "mp_common.h"
#include "su_common.h"
#include "main.h"
#include "dma/priv_dma_uart.h"

typedef struct st_stm32_uart_driver
{
    uart_driver_t               base;
    UART_HandleTypeDef* const * hw_insts;
} stm32_uart_driver_t;

static stm32_uart_driver_t g_uart_drv = { .base = { 0U }, .hw_insts = NULL };

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;

    g_uart_drv.base.hw_inst_cnt = get_uart_ifcs(&g_uart_drv.hw_insts);

    if (g_uart_drv.hw_insts == NULL)
    {
        ret_val = RET_ERROR;
    }
    else if (g_uart_drv.base.hw_inst_cnt == 0)
    {
        ret_val = RET_NOT_SUPPORTED;
    }
    else
    {
        for (uint16_t i = 0; i < g_uart_drv.base.hw_inst_cnt; ++i)
        {
            if (g_uart_drv.hw_insts[i] == NULL)
            {
                ret_val = RET_ERROR;
                break;
            }
        }
    }
    dma_hw_insts_register(g_uart_drv.hw_insts);
    return ret_val;
}

static response_status_t read(mp_uart_ifc_idx_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz,
                              timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_uart_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_uart_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_buffer_sz == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    HAL_StatusTypeDef hal_ret =
      HAL_UART_Receive(g_uart_drv.hw_insts[p_ifc_index], ppt_buffer, p_buffer_sz, p_timeout_ms);

    ret_val = translate_hal_status(hal_ret);

    return ret_val;
}

static response_status_t write(mp_uart_ifc_idx_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz,
                               timeout_t p_timeout_ms)
{

    ASSERT_AND_RETURN(g_uart_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_uart_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_buffer_sz == 0, RET_PARAM_ERROR);
    
    response_status_t ret_val = RET_OK;

    if (dma_tx_in_progress(p_ifc_index) == FALSE)
    {    
        HAL_StatusTypeDef hal_ret =
        HAL_UART_Transmit(g_uart_drv.hw_insts[p_ifc_index], ppt_buffer, p_buffer_sz, p_timeout_ms);
        
        ret_val = translate_hal_status(hal_ret);
    }
    else
    {
        ret_val = RET_BUSY;
    }

    return ret_val;
}

static struct st_uart_driver_ifc g_interface = {
    .init     = init,
    .receive  = read,
    .transmit = write,
    .dma_register_cb = dma_tx_register_callback,
    .dma_transmit_abort = dma_tx_abort,
    .dma_transmit_request = dma_tx_start
};

uart_driver_t* uart_driver_register(void)
{
    g_uart_drv.base.api = &g_interface;
    return (uart_driver_t*)&g_uart_drv;
}
