#ifdef TEST

#include "mock_main.h"
#include "mock_stm32f4xx_hal_uart.h"
#include "priv_dma_uart.h"
#include "mp_uart.h"
#include "su_common.h"
#include "unity.h"

#undef ARRAY_SIZE
#define ARRAY_SIZE(x) x##_len

typedef struct
{
    UART_HandleTypeDef* expected_huart;
    uint8_t*            expected_data;
    uint16_t            expected_size;
    uint32_t            expected_timeout;
    HAL_StatusTypeDef   return_val;
} UartReceiveExpectation;

static UartReceiveExpectation uart_expect;

UART_HandleTypeDef huart1 = { 0 };
UART_HandleTypeDef huart2 = { 0 };

uart_driver_t* g_uart_driver = NULL;

static UART_HandleTypeDef* const * l_uart_ifcs = NULL;
static size_t                      l_uart_ifcs_len = 0U;
static UART_HandleTypeDef*         expected_uart = NULL;

size_t get_uart_ifcs_stub(UART_HandleTypeDef* const ** const uart_ifcs_buffer)
{
    *uart_ifcs_buffer = l_uart_ifcs;
    // Return the number of UART interfaces
    return ARRAY_SIZE(l_uart_ifcs);
}

HAL_StatusTypeDef HAL_UART_Receive_fake(UART_HandleTypeDef* huart, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    TEST_ASSERT_EQUAL_PTR(uart_expect.expected_huart, huart);
    TEST_ASSERT_EQUAL_PTR(uart_expect.expected_data, pData);
    TEST_ASSERT_EQUAL(uart_expect.expected_size, Size);
    TEST_ASSERT_EQUAL(uart_expect.expected_timeout, Timeout);
    return uart_expect.return_val;
}

HAL_StatusTypeDef HAL_UART_Transmit_fake(UART_HandleTypeDef* huart, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    TEST_ASSERT_EQUAL_PTR(uart_expect.expected_huart, huart);
    TEST_ASSERT_EQUAL_PTR(uart_expect.expected_data, pData);
    TEST_ASSERT_EQUAL(uart_expect.expected_size, Size);
    TEST_ASSERT_EQUAL(uart_expect.expected_timeout, Timeout);
    return uart_expect.return_val;
}

void setUp(void)
{
    g_uart_driver = uart_driver_register();
    static UART_HandleTypeDef* uart_ifcs[] = { &huart1, &huart2 };
    l_uart_ifcs = uart_ifcs;
    l_uart_ifcs_len = 2;
}

void tearDown(void) {}

void test_uart_write_when_driver_not_initialized_should_return_error(void)
{
    uint8_t buffer[] = "Hello World!";
    size_t  buffer_len = sizeof(buffer);
    l_uart_ifcs = NULL; // Simulate driver not initialized
    get_uart_ifcs_StubWithCallback(get_uart_ifcs_stub);
    g_uart_driver->api->init();
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, g_uart_driver->api->receive(0, buffer, buffer_len, 0));
}

void test_uart_read_when_driver_not_initialized_should_return_error(void)
{
    uint8_t buffer[] = "Hello World!";
    size_t  buffer_len = sizeof(buffer);
    uart_expect = (UartReceiveExpectation) { .expected_huart = &huart1,
                                             .expected_data = buffer,
                                             .expected_size = buffer_len,
                                             .expected_timeout = 0,
                                             .return_val = HAL_OK };
    l_uart_ifcs = NULL; // Simulate driver not initialized
    get_uart_ifcs_StubWithCallback(get_uart_ifcs_stub);
    g_uart_driver->api->init();
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, g_uart_driver->api->receive(0, buffer, buffer_len, 0));
}

void test_uart_init_with_null_interfaces_should_return_error(void)
{
    get_uart_ifcs_StubWithCallback(get_uart_ifcs_stub);
    l_uart_ifcs = NULL;
    TEST_ASSERT_EQUAL(RET_ERROR, g_uart_driver->api->init());
}

void test_uart_init_with_empty_interfaces_should_return_not_supported(void)
{
    get_uart_ifcs_StubWithCallback(get_uart_ifcs_stub);
    l_uart_ifcs_len = 0;
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, g_uart_driver->api->init());
}

void test_uart_init_with_valid_interfaces_should_return_ok(void)
{
    get_uart_ifcs_StubWithCallback(get_uart_ifcs_stub);
    TEST_ASSERT_EQUAL(RET_OK, g_uart_driver->api->init());
}

void test_uart_write_with_out_of_bound_index_should_return_error(void)
{
    uint8_t buffer[] = "Hello World!";
    size_t  buffer_len = sizeof(buffer);
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, g_uart_driver->api->receive(ARRAY_SIZE(l_uart_ifcs), buffer, buffer_len, 0));
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED,
                      g_uart_driver->api->receive(ARRAY_SIZE(l_uart_ifcs) + 1, buffer, buffer_len, 0));
}

void test_uart_write_with_invalid_buffer_should_return_error(void)
{
    uint8_t* buffer = NULL;
    size_t   buffer_len = 10;
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_uart_driver->api->receive(0, buffer, buffer_len, 0));

    buffer = "Hello World!";
    buffer_len = 0;
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_uart_driver->api->receive(0, buffer, buffer_len, 0));
}

void test_uart_write_with_valid_parameters_should_return_ok(void)
{
    uint8_t buffer[] = "Hello World!";
    size_t  buffer_len = sizeof(buffer);
    HAL_UART_Transmit_StubWithCallback(HAL_UART_Transmit_fake);

    for (size_t i = 0; i < l_uart_ifcs_len; i++)
    {
        uart_expect = (UartReceiveExpectation) { .expected_huart = l_uart_ifcs[i],
                                                 .expected_data = buffer,
                                                 .expected_size = buffer_len,
                                                 .expected_timeout = 0,
                                                 .return_val = HAL_OK };
        TEST_ASSERT_EQUAL(RET_OK, g_uart_driver->api->transmit(i, buffer, buffer_len, 0));
    }
}

void test_uart_read_with_out_of_bound_index_should_return_error(void)
{
    uint8_t buffer[sizeof("Hello World!")];
    size_t  buffer_len = sizeof(sizeof("Hello World!"));
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, g_uart_driver->api->receive(ARRAY_SIZE(l_uart_ifcs), buffer, buffer_len, 0));
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED,
                      g_uart_driver->api->receive(ARRAY_SIZE(l_uart_ifcs) + 1, buffer, buffer_len, 0));
}

void test_uart_read_with_invalid_buffer_should_return_error(void)
{
    uint8_t* buffer = NULL;
    size_t   buffer_len = 10;
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_uart_driver->api->receive(0, buffer, buffer_len, 0));

    buffer = "Hello World!";
    buffer_len = 0;
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_uart_driver->api->receive(0, buffer, buffer_len, 0));
}

void test_uart_read_with_valid_parameters_should_return_ok(void)
{
    uint8_t buffer[] = "Hello World!";
    size_t  buffer_len = sizeof(buffer);

    HAL_UART_Receive_StubWithCallback(HAL_UART_Receive_fake);

    for (size_t i = 0; i < l_uart_ifcs_len; i++)
    {
        uart_expect = (UartReceiveExpectation) { .expected_huart = l_uart_ifcs[i],
                                                 .expected_data = buffer,
                                                 .expected_size = buffer_len,
                                                 .expected_timeout = 0,
                                                 .return_val = HAL_OK };
        TEST_ASSERT_EQUAL(RET_OK, g_uart_driver->api->receive(i, buffer, buffer_len, 0));
    }
}

#endif // TEST