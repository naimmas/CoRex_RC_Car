#ifdef TEST

#include "ha_uart.h"
#include "mock_mp_uart.h"
#include "string.h"
#include "su_common.h"
#include "unity.h"

static response_status_t drv_init(void);
static response_status_t drv_write(uint8_t, uint8_t*, size_t, timeout_t);
static response_status_t drv_read(uint8_t, uint8_t*, size_t, timeout_t);

static response_status_t func_ret_val = RET_OK;
static unsigned char     uart_tx_buf[UART_PORT_CNT][512];
static unsigned char     uart_rx_buf[UART_PORT_CNT][512];

struct st_uart_driver_ifc fake_driver_ifc = { .init = drv_init, .receive = drv_read, .transmit = drv_write };

struct st_uart_driver fake_uart_driver = { .api = &fake_driver_ifc, .hw_inst_cnt = 0 };

static response_status_t drv_init(void)
{
    fake_uart_driver.hw_inst_cnt = UART_PORT_CNT;
    return func_ret_val;
}

static response_status_t drv_write(uint8_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz, timeout_t p_timeout)
{
    memcpy(uart_tx_buf[p_ifc_index], ppt_buffer, p_buffer_sz);
    return func_ret_val;
}

static response_status_t drv_read(uint8_t p_ifc_index, uint8_t* ppt_buffer, size_t p_buffer_sz, timeout_t p_timeout)
{
    memcpy(ppt_buffer, uart_rx_buf[p_ifc_index], p_buffer_sz);
    return func_ret_val;
}

void setUp(void)
{
    func_ret_val = RET_OK;
}

void tearDown(void) {}

/********* Tests for UART driver *********/
void test_uart_when_driver_not_registered_should_return_error(void)
{
    uart_driver_register_ExpectAndReturn(NULL);

    TEST_ASSERT_EQUAL(RET_ERROR, ha_uart_init());
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_uart_receive(UART_PORT1, NULL, 0, 0));
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_uart_transmit(UART_PORT1, NULL, 0, 0));
}

void test_uart_when_driver_registeration_fails_should_return_error(void)
{
    func_ret_val = RET_ERROR;
    uart_driver_register_ExpectAndReturn(&fake_uart_driver);
    TEST_ASSERT_EQUAL(RET_ERROR, ha_uart_init());
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_uart_receive(UART_PORT1, NULL, 0, 0));
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_uart_transmit(UART_PORT1, NULL, 0, 0));

    func_ret_val = RET_NOT_SUPPORTED;
    uart_driver_register_ExpectAndReturn(&fake_uart_driver);
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_uart_init());
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_uart_receive(UART_PORT1, NULL, 0, 0));
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_uart_transmit(UART_PORT1, NULL, 0, 0));
    func_ret_val = RET_OK;
}

void test_uart_init_success(void)
{
    uart_driver_register_ExpectAndReturn(&fake_uart_driver);
    TEST_ASSERT_EQUAL(RET_OK, ha_uart_init());
}

void test_uart_port_not_supported(void)
{

    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_uart_receive(UART_PORT_CNT, NULL, 0, 0));
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_uart_transmit(UART_PORT_CNT, NULL, 0, 0));
}

void test_uart_when_msg_buffer_is_null(void)
{

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_uart_receive(UART_PORT1, NULL, 0, 0));
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_uart_transmit(UART_PORT1, NULL, 0, 0));
}

void test_uart_when_msg_buffer_size_is_zero(void)
{
    uint8_t data_buffer[] = "Hello from test\n";

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_uart_receive(UART_PORT1, data_buffer, 0, 0));
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_uart_transmit(UART_PORT1, data_buffer, 0, 0));
}

void test_uart_write_success(void)
{
    uint8_t data_buffer[] = "Hello from test\n";

    TEST_ASSERT_EQUAL(RET_OK, ha_uart_transmit(UART_PORT1, data_buffer, sizeof(data_buffer), 0));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Hello from test\n", uart_tx_buf[UART_PORT1], sizeof("Hello from test\n"));
}

void test_uart_read_success(void)
{
    uint8_t recv_buff[20] = { 0 };
    memcpy(uart_rx_buf[UART_PORT1], "Hello from test\n1234", sizeof("Hello from test\n1234"));

    TEST_ASSERT_EQUAL(RET_OK, ha_uart_receive(UART_PORT1, recv_buff, 20, 0));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Hello from test\n", recv_buff, sizeof("Hello from test\n") - 1);
}

#endif // TEST