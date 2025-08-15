
#ifndef PRIV_DMA_UART_H
#define PRIV_DMA_UART_H

#include "../mp_uart.h"
#include "main.h"

response_status_t dma_tx_start(uint8_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz);
response_status_t dma_tx_abort(uint8_t p_ifc_index);
response_status_t dma_tx_register_callback(uint8_t p_ifc_index, dma_tx_evt_cb p_evt_cb);
bool_t            dma_tx_in_progress(uint8_t p_ifc_index);
void              dma_hw_insts_register(UART_HandleTypeDef* const * hw_insts);

#endif // PRIV_DMA_UART_H
