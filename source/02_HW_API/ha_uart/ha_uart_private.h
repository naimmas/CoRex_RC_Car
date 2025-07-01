#ifndef HA_UART_PRIVATE_H
#define HA_UART_PRIVATE_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_uart_driver_ifc* uart_driver_ifc;

typedef struct st_uart_driver
{
    uart_driver_ifc api;
    uint16_t        hw_inst_cnt;
} uart_driver_t;

struct st_uart_driver_ifc
{
    response_status_t (*init)(void);
    response_status_t (*receive)(uint8_t, uint8_t*, size_t, timeout_t);
    response_status_t (*transmit)(uint8_t, uint8_t*, size_t, timeout_t);
};

#endif /* HA_UART_PRIVATE_H */
