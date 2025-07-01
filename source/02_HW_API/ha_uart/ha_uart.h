#ifndef HA_UART_H
#define HA_UART_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_uart_driver* uart_driver;

typedef enum en_uart_comm_port
{
    UART_PORT1 = 0,
    UART_PORT_CNT,
} uart_comm_port_t;

response_status_t ha_uart_init(void);
response_status_t ha_uart_receive(uart_comm_port_t p_port,
                                  uint8_t* ppt_data_buffer, size_t p_data_size,
                                  timeout_t p_timeout);
response_status_t ha_uart_transmit(uart_comm_port_t p_port,
                                   uint8_t* ppt_data_buffer, size_t p_data_size,
                                   timeout_t p_timeout);

#endif /* HA_UART_H */
