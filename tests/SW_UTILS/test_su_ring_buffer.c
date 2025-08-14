#define TEST

#ifdef TEST

#include "unity.h"

#include "su_ring_buffer.h"
#define MAX_BUFF_SZ 15

su_rb_t test_buffer;
uint8_t buffer_data[MAX_BUFF_SZ] = {[0 ... MAX_BUFF_SZ-1] = 0xFF}; // Initialize with 0xFF
uint8_t output_register[MAX_BUFF_SZ] = {[0 ... MAX_BUFF_SZ-1] = 0x00};
volatile size_t usart_tx_dma_current_len;
volatile uint8_t dma_finished = 1;
volatile static size_t remain_len = 0;

void dma_finished_cb(void);
uint8_t usart_start_tx_dma_transfer(void);

void send_dma(size_t len, uint8_t* addrs)
{
    printf("Sending %zu bytes from address %p\n", len, addrs);
    memcpy(output_register, addrs, len);
    for (size_t i = 0; i < usart_tx_dma_current_len; i++)
    {
        printf("Output[%zu]: %02d\n", i, output_register[i]);
    }
    dma_finished_cb();
}

void dma_finished_cb(void)
{
    su_rb_skip(&test_buffer, usart_tx_dma_current_len);/* Skip sent data, mark as read */
    dma_finished = 1;           /* Clear length variable */
    usart_start_tx_dma_transfer();          /* Start sending more data */
}

uint8_t trigger_data_send(const uint8_t* data, size_t len)
{
    size_t free_space = su_rb_get_free(&test_buffer);
    size_t bts = 0;

    if (len > free_space)
    {
        remain_len = len - free_space; // Store remaining length
        bts = free_space;
        printf("Warning: Not enough space in buffer. Free space: %zu, Required: %zu, Remaining: %zu\n", 
               free_space, len, remain_len);
    }
    else
    {
        bts = len;
    }

    if (bts > 0)
    {
        su_rb_sz_t written = su_rb_write(&test_buffer, data, bts);
        if (written != bts)
        {
            printf("Error: Not all data written to buffer. Expected: %zu, Written: %zu\n", bts, written);
            return 0; // Error
        }
        else if (written != len)
        {
            return 0;
        }
        
    }
    return 1;
}

uint8_t usart_start_tx_dma_transfer(void) 
{
    uint8_t started = 0;

    if (dma_finished == 1)
    {
        usart_tx_dma_current_len = su_rb_get_linear_block_read_length(&test_buffer);
        if (usart_tx_dma_current_len > 0)
        {
            dma_finished = 0;        
            send_dma(usart_tx_dma_current_len,  (uint8_t*)su_rb_get_linear_block_read_address(&test_buffer)); /* Send data to USART */
            /* Start transfer */
            started = 1;
        }
    }
    
    return started;

}

void evt_cb(struct lwrb* buff, su_rb_evt_type_t evt, su_rb_sz_t bp)
{
    switch(evt)
    {
        case SU_RB_EVT_READ:
            // Handle read event
            break;
        case SU_RB_EVT_WRITE:
            usart_start_tx_dma_transfer(); // Start USART transfer if needed
            break;
        case SU_RB_EVT_RESET:
            // Handle reset event
            break;
    }

}

void setUp(void)
{
    TEST_ASSERT_TRUE(su_rb_init(&test_buffer, buffer_data, sizeof(buffer_data)));
    TEST_ASSERT_TRUE(su_rb_is_ready(&test_buffer));
    su_rb_set_evt_fn(&test_buffer, evt_cb);
}

void tearDown(void)
{
}

void test_su_ring_buffer_SimpleWriteShouldSuccess(void)
{
    uint8_t data[30];
    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = (uint8_t)i; // Fill with some data
    }
    su_rb_sz_t sz =0;
    memset(output_register, 0, sizeof(output_register)); // Clear output register
    uint8_t res = 0;
    remain_len = sizeof(data);
    do
    {
         res = trigger_data_send(&data[sizeof(data)-remain_len], remain_len);
    }while(res == 0);
}

#endif // TEST
