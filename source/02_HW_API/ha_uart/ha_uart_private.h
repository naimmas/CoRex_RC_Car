#ifndef HA_UART_PRIVATE_H
#define HA_UART_PRIVATE_H

#include "ha_uart.h"
#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_uart_driver_ifc* uart_driver_ifc;
typedef uart_comm_port_t mp_uart_ifc_idx_t;

typedef struct st_uart_driver
{
    uart_driver_ifc api;
    uint16_t        hw_inst_cnt;
} uart_driver_t;

typedef enum
{
    MP_UART_DMA_TX_EVT_COMPLETE = 0,
    MP_UART_DMA_TX_EVT_ABORT,
    MP_UART_DMA_TX_EVT_ERROR
} mp_uart_dma_tx_event_t;

typedef void (*dma_tx_evt_cb)(mp_uart_ifc_idx_t p_ifc_index, mp_uart_dma_tx_event_t p_event);

struct st_uart_driver_ifc
{
    response_status_t (*init)(void);
    response_status_t (*receive)(mp_uart_ifc_idx_t, uint8_t*, size_t, timeout_t);
    response_status_t (*transmit)(mp_uart_ifc_idx_t, uint8_t*, size_t, timeout_t);
    response_status_t (*dma_transmit_request)(mp_uart_ifc_idx_t, uint8_t*, size_t);
    response_status_t (*dma_transmit_abort)(mp_uart_ifc_idx_t);
    response_status_t (*dma_register_cb)(mp_uart_ifc_idx_t, dma_tx_evt_cb);
};

#endif /* HA_UART_PRIVATE_H */
