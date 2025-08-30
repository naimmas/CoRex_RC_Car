#include "serial_ifc.h"

#include "ha_uart/ha_uart.h"
#include "ps_logger.h"
#include "su_ring_buffer/su_ring_buffer.h"

static uint8_t g_log_buffer_data[LOGGER_MSG_MAX_LENGTH]; // Buffer for log data

su_rb_t g_log_buffer; // Initialized elsewhere

static volatile uint8_t g_uart_tx_dma_busy = 0;
static volatile size_t  g_uart_tx_dma_len  = 0;

/* Start next DMA transfer if possible */
static void dma_buffer_process(void)
{
    if (g_uart_tx_dma_busy)
    {
        return;
    }

    g_uart_tx_dma_len = su_rb_get_linear_block_read_length(&g_log_buffer);
    if (g_uart_tx_dma_len == 0)
    {
        return;
    }

    g_uart_tx_dma_busy = 1;
    // uart_dma_tx_start_tick = HAL_GetTick();

    uint8_t* pt_ptr = (uint8_t*)su_rb_get_linear_block_read_address(&g_log_buffer);
    if (ha_uart_dma_transmit(UART_DBG_PORT, pt_ptr, g_uart_tx_dma_len) != RET_OK)
    {
        g_uart_tx_dma_busy = 0; // Failed to start DMA
    }
}

static void dma_cb(uart_comm_port_t p_ifc_idx, uart_dma_event_t p_event)
{
    switch (p_event)
    {
        case UART_DMA_EVT_TX_COMPLETE:
            su_rb_skip(&g_log_buffer, g_uart_tx_dma_len); // Mark sent data as read
            g_uart_tx_dma_busy = 0;
            dma_buffer_process();
            break;
        case UART_DMA_EVT_ABORT:
            // Handle TX abort event
            // su_rb_skip(&log_buffer, uart_tx_dma_len); // Skip even if aborted
            g_uart_tx_dma_busy = 0;
            break;
        case UART_DMA_EVT_ERROR:
            // Handle TX error event
            g_uart_tx_dma_busy = 0;
            dma_buffer_process();
            break;
        default:
            break;
    }
    // Optionally, you can call dma_buffer_process() here if needed
}

/* Push log data into buffer and start DMA if idle */
void serial_ifc_send(const uint8_t* ppt_data, size_t p_len)
{
    if (su_rb_get_free(&g_log_buffer) >= p_len)
    {
        su_rb_write(&g_log_buffer, ppt_data, p_len);
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
          su_rb_init(&g_log_buffer, g_log_buffer_data, sizeof(g_log_buffer_data)) ? RET_OK : RET_ERROR;
        ret |= su_rb_is_ready(&g_log_buffer) ? RET_OK : RET_ERROR; // Check if the buffer is ready
    }
    if (ret == RET_OK)
    {
        ha_uart_dma_register_callback(UART_DBG_PORT, dma_cb); // Register DMA callback
    }
    return ret;
}
