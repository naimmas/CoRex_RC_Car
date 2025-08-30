
#include "priv_dma_uart.h"

#include "ha_uart/ha_uart_private.h"
#include "main.h"
#include "mp_common.h"
#include "su_common.h"

typedef struct st_stm32_uart_dma_driver
{
    UART_HandleTypeDef* const * hw_insts;
    volatile uint32_t           dma_in_progress;
    bool_t                      hw_insts_registered;
    dma_tx_evt_cb               user_cb[UART_PORT_CNT];
} stm32_uart_dma_driver_t;

static stm32_uart_dma_driver_t g_uart_dma_drv = { .hw_insts            = NULL,
                                                  .dma_in_progress     = 0x00,
                                                  .hw_insts_registered = FALSE };

static uint32_t get_inst_idx(UART_HandleTypeDef* huart)
{
    for (uint32_t i = 0; i < __builtin_popcount(g_uart_dma_drv.dma_in_progress); i++)
    {
        if (huart == g_uart_dma_drv.hw_insts[i])
        {
            return i;
        }
    }
    return (uint32_t)-1;
}

/* Successful TX complete */
void dma_tx_finished_cb(UART_HandleTypeDef* huart)
{
    uint32_t ifc_index = get_inst_idx(huart);
    if (ifc_index >= UART_PORT_CNT)
    {
        return;
    }
    BIT_CLR(g_uart_dma_drv.dma_in_progress, ifc_index);
    if (g_uart_dma_drv.user_cb[ifc_index] != NULL)
    {
        g_uart_dma_drv.user_cb[ifc_index](ifc_index, MP_UART_DMA_TX_EVT_COMPLETE);
    }
}

/* Abort complete */
void dma_tx_abort_cb(UART_HandleTypeDef* huart)
{
    uint32_t ifc_index = get_inst_idx(huart);
    if (ifc_index >= UART_PORT_CNT)
    {
        return;
    }
    BIT_CLR(g_uart_dma_drv.dma_in_progress, ifc_index);
    if (g_uart_dma_drv.user_cb[ifc_index] != NULL)
    {
        g_uart_dma_drv.user_cb[ifc_index](ifc_index, MP_UART_DMA_TX_EVT_ABORT);
    }
}

/* Error handler */
void dma_tx_error_cb(UART_HandleTypeDef* huart)
{
    uint32_t ifc_index = get_inst_idx(huart);
    if (ifc_index >= UART_PORT_CNT)
    {
        return;
    }
    BIT_CLR(g_uart_dma_drv.dma_in_progress, ifc_index);
    if (g_uart_dma_drv.user_cb[ifc_index] != NULL)
    {
        g_uart_dma_drv.user_cb[ifc_index](ifc_index, MP_UART_DMA_TX_EVT_ERROR);
    }
}

bool_t dma_tx_in_progress(mp_uart_ifc_idx_t p_ifc_index)
{
    ASSERT_AND_RETURN(g_uart_dma_drv.hw_insts_registered == FALSE, FALSE);
    ASSERT_AND_RETURN(p_ifc_index >= UART_PORT_CNT, FALSE);

    return BIT_GET(g_uart_dma_drv.dma_in_progress, p_ifc_index) ? TRUE : FALSE;
}

response_status_t dma_tx_start(mp_uart_ifc_idx_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz)
{
    ASSERT_AND_RETURN(g_uart_dma_drv.hw_insts_registered == FALSE, RET_NOT_INITIALIZED);

    HAL_StatusTypeDef ret_hal = HAL_OK;

    ret_hal = HAL_UART_Transmit_DMA(g_uart_dma_drv.hw_insts[p_ifc_index], ppt_buffer, p_buffer_sz);

    if (ret_hal == HAL_OK)
    {
        BIT_SET(g_uart_dma_drv.dma_in_progress, p_ifc_index);
    }
    else
    {
        BIT_CLR(g_uart_dma_drv.dma_in_progress, p_ifc_index);
    }

    return translate_hal_status(ret_hal);
}

response_status_t dma_tx_abort(mp_uart_ifc_idx_t p_ifc_index)
{
    ASSERT_AND_RETURN(g_uart_dma_drv.hw_insts_registered == FALSE, RET_NOT_INITIALIZED);

    HAL_StatusTypeDef ret_hal = HAL_OK;
    ret_hal                   = HAL_UART_AbortTransmit_IT(g_uart_dma_drv.hw_insts[p_ifc_index]);
    return translate_hal_status(ret_hal);
}

response_status_t dma_tx_register_callback(mp_uart_ifc_idx_t p_ifc_index, dma_tx_evt_cb p_evt_cb)
{
    ASSERT_AND_RETURN(g_uart_dma_drv.hw_insts_registered == FALSE, RET_NOT_INITIALIZED);

    g_uart_dma_drv.user_cb[p_ifc_index] = p_evt_cb;

    return RET_OK;
}

void dma_hw_insts_register(UART_HandleTypeDef* const * hw_insts)
{
    g_uart_dma_drv.hw_insts = hw_insts;
    if (hw_insts != NULL)
    {
        for (uint32_t i = 0; i < UART_PORT_CNT; i++)
        {
            HAL_UART_RegisterCallback(hw_insts[i], HAL_UART_TX_COMPLETE_CB_ID, dma_tx_finished_cb);
            HAL_UART_RegisterCallback(hw_insts[i],
                                      HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID,
                                      dma_tx_abort_cb);
            HAL_UART_RegisterCallback(hw_insts[i], HAL_UART_ERROR_CB_ID, dma_tx_error_cb);
        }
    }
    g_uart_dma_drv.hw_insts_registered = (hw_insts != NULL);
}
