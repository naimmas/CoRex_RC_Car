#include "serial_ifc.h"

#include "ha_uart/ha_uart.h"
#include "ps_logger.h"
#include "su_ring_buffer/su_ring_buffer.h"

static uint8_t log_buffer_data[LOGGER_MSG_MAX_LENGTH]; // Buffer for log data

su_rb_t log_buffer; // Initialized elsewhere

static volatile uint8_t uart_tx_dma_busy = 0;
static volatile size_t  uart_tx_dma_len  = 0;

/* Start next DMA transfer if possible */
static void dma_buffer_process(void)
{
    if (uart_tx_dma_busy)
    {
        return;
    }

    uart_tx_dma_len = su_rb_get_linear_block_read_length(&log_buffer);
    if (uart_tx_dma_len == 0)
    {
        return;
    }

    uart_tx_dma_busy = 1;
    // uart_dma_tx_start_tick = HAL_GetTick();

    uint8_t* ptr = (uint8_t*)su_rb_get_linear_block_read_address(&log_buffer);
    if (ha_uart_dma_transmit(UART_DBG_PORT, ptr, uart_tx_dma_len) != RET_OK)
    {
        uart_tx_dma_busy = 0; // Failed to start DMA
    }
}

static void dma_cb(uart_comm_port_t p_ifc_idx, uart_dma_event_t p_event)
{
    switch (p_event)
    {
        case UART_DMA_EVT_TX_COMPLETE:
            su_rb_skip(&log_buffer, uart_tx_dma_len); // Mark sent data as read
            uart_tx_dma_busy = 0;
            dma_buffer_process();
            break;
        case UART_DMA_EVT_ABORT:
            // Handle TX abort event
            // su_rb_skip(&log_buffer, uart_tx_dma_len); // Skip even if aborted
            uart_tx_dma_busy = 0;
            break;
        case UART_DMA_EVT_ERROR:
            // Handle TX error event
            uart_tx_dma_busy = 0;
            dma_buffer_process();
            break;
        default:
            break;
    }
    // Optionally, you can call dma_buffer_process() here if needed
}

/* Push log data into buffer and start DMA if idle */
void serial_ifc_send(const uint8_t* data, size_t len)
{
    if (su_rb_get_free(&log_buffer) >= len)
    {
        su_rb_write(&log_buffer, data, len);
        dma_buffer_process();
    }
    else
    {
        // Handle overflow (drop, overwrite, etc.)
    }
}

response_status_t serial_ifc_init(void)
{

    response_status_t ret = RET_OK;

    ret = ha_uart_init(); // Initialize UART for DMA
    if (ret == RET_OK)
    {
        // Initialize the ring buffer
        ret =
          su_rb_init(&log_buffer, log_buffer_data, sizeof(log_buffer_data)) ? RET_OK : RET_ERROR;
        ret |= su_rb_is_ready(&log_buffer) ? RET_OK : RET_ERROR; // Check if the buffer is ready
    }
    if (ret == RET_OK)
    {
        ha_uart_dma_register_callback(UART_DBG_PORT, dma_cb); // Register DMA callback
    }
    return ret;
}
