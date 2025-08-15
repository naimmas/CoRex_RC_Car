#include "dd_esp32.h"

#include "ha_timer/ha_timer.h"
#include "ha_uart/ha_uart.h"
#include "string.h"
#include "su_common.h"

#define USER_DATA_SIZE (sizeof(dd_esp32_data_packet_t)/sizeof(uint8_t))

volatile bool_t   g_err_flag     = FALSE;
volatile bool_t   g_free_to_send = TRUE;
uint32_t g_packet_no    = 0;

void dma_evt_cb(uart_comm_port_t p_port, uart_dma_event_t p_event)
{
    // Handle DMA events for ESP32 UART communication
    if (p_port == UART_ESP32_PORT)
    {
        switch (p_event)
        {
            case UART_DMA_EVT_TX_COMPLETE:
                g_free_to_send = TRUE;
                break;
            case UART_DMA_EVT_ABORT:
            case UART_DMA_EVT_ERROR:
                g_err_flag     = TRUE;
                g_free_to_send = TRUE;
                break;
            default:
                break;
        }
    }
}

response_status_t dd_esp32_init(void)
{
    response_status_t ret_val = RET_OK;

    ret_val = ha_uart_init();
    if (ret_val == RET_OK)
    {
        ret_val = ha_uart_dma_register_callback(UART_ESP32_PORT, dma_evt_cb);
    }

    return ret_val;
}

response_status_t dd_esp32_send_data_packet(dd_esp32_data_packet_t* ppt_data_packet)
{
    if (g_free_to_send == FALSE)
    {
        return RET_BUSY;
    }

    if (g_err_flag == TRUE)
    {
        g_err_flag = FALSE;
        return RET_ERROR;
    }

    response_status_t ret_val                             = RET_OK;
    uint32_t          time_stamp                          = ha_timer_get_cpu_time_ms();
    char data_packet_str[2048] = {0};
    size_t wb = snprintf(data_packet_str, 2048, "%d,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%.2f,"
        "%d,"
        "%d\n",
             g_packet_no++, 
             ppt_data_packet->acc[0].f, 
             ppt_data_packet->acc[1].f,
             ppt_data_packet->acc[2].f, 
             ppt_data_packet->gyro[0].f, 
             ppt_data_packet->gyro[1].f,
             ppt_data_packet->gyro[2].f, 
             ppt_data_packet->mag[0].f, 
             ppt_data_packet->mag[1].f,
             ppt_data_packet->mag[2].f,
             ppt_data_packet->baro.f, 
             ppt_data_packet->throttle_stick,
             ppt_data_packet->steering_stick);
    ret_val =
      ha_uart_dma_transmit(UART_ESP32_PORT, (uint8_t*)data_packet_str, wb);

    return ret_val;
}
