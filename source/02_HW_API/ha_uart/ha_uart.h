#ifndef HA_UART_H
#define HA_UART_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_uart_driver* uart_driver;

typedef enum en_uart_comm_port
{
    UART_DBG_PORT = 0,
    UART_ESP32_PORT,
    UART_PORT_CNT,
} uart_comm_port_t;

typedef enum
{
    UART_DMA_EVT_TX_COMPLETE = 0,
    UART_DMA_EVT_ABORT,
    UART_DMA_EVT_ERROR
} uart_dma_event_t;

typedef void (*uart_dma_evt_cb)(uart_comm_port_t, uart_dma_event_t);

response_status_t ha_uart_init(void);
response_status_t ha_uart_receive(uart_comm_port_t p_port, uint8_t* ppt_data_buffer,
                                  size_t p_data_size, timeout_t p_timeout);
response_status_t ha_uart_transmit(uart_comm_port_t p_port, uint8_t* ppt_data_buffer,
                                   size_t p_data_size, timeout_t p_timeout);
response_status_t ha_uart_dma_transmit(uart_comm_port_t p_port, uint8_t* ppt_data_buffer,
                                       size_t p_data_size);
response_status_t ha_uart_dma_stop(uart_comm_port_t p_port);
response_status_t ha_uart_dma_register_callback(uart_comm_port_t p_port, uart_dma_evt_cb ppt_evt_cb);

#endif /* HA_UART_H */
