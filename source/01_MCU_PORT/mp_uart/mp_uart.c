#include "mp_uart.h"

#include "../mp_common.h"
#include "ha_uart/ha_uart_private.h"
#include "main.h"
#include "su_common.h"

typedef struct st_stm32_uart_driver
{
    uart_driver_t               base;
    UART_HandleTypeDef* const * hw_insts;
} stm32_uart_driver_t;

static stm32_uart_driver_t g_driver = { .base = { 0U }, .hw_insts = NULL };

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;

    g_driver.base.hw_inst_cnt = get_uart_ifcs(&g_driver.hw_insts);

    if (g_driver.hw_insts == NULL)
    {
        ret_val = RET_ERROR;
    }
    else if (g_driver.base.hw_inst_cnt == 0)
    {
        ret_val = RET_NOT_SUPPORTED;
    }
    else
    {
        for (uint16_t i = 0; i < g_driver.base.hw_inst_cnt; ++i)
        {
            if (g_driver.hw_insts[i] == NULL)
            {
                ret_val = RET_ERROR;
                break;
            }
        }
    }

    return ret_val;
}

static response_status_t read(uint8_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz,
                              timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_driver.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_driver.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_buffer_sz == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    HAL_StatusTypeDef hal_ret =
      HAL_UART_Receive(g_driver.hw_insts[p_ifc_index], ppt_buffer, p_buffer_sz, p_timeout_ms);

    ret_val = translate_hal_status(hal_ret);

    return ret_val;
}

static response_status_t write(uint8_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz,
                               timeout_t p_timeout_ms)
{

    ASSERT_AND_RETURN(g_driver.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_driver.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_buffer == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_buffer_sz == 0, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    HAL_StatusTypeDef hal_ret =
      HAL_UART_Transmit(g_driver.hw_insts[p_ifc_index], ppt_buffer, p_buffer_sz, p_timeout_ms);

    ret_val = translate_hal_status(hal_ret);

    return ret_val;
}

static struct st_uart_driver_ifc g_interface = {
    .init     = init,
    .receive  = read,
    .transmit = write,
};

uart_driver_t* uart_driver_register(void)
{
    g_driver.base.api = &g_interface;
    return (uart_driver_t*)&g_driver;
}
