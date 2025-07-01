
#include "mock_main.h"
#include "mock_stm32f4xx_hal_i2c.h"
#include "mp_iic.h"
#include "unity.h"

#undef ARRAY_SIZE
#define ARRAY_SIZE(x) x##_len

typedef struct
{
    I2C_HandleTypeDef* expected_hi2c;
    uint16_t           expected_DevAddress;
    uint16_t           expected_MemAddress;
    uint16_t           expected_MemAddSize;
    uint8_t*           expected_pData;
    uint16_t           expected_Size;
    uint32_t           expected_Timeout;
    HAL_StatusTypeDef  return_val;
} I2CReceiveExpectation;

static I2CReceiveExpectation i2c_expect;

I2C_HandleTypeDef hi2c1 = { 0 };
I2C_HandleTypeDef hi2c2 = { 0 };

iic_driver_t* g_i2c_driver = NULL;

static I2C_HandleTypeDef* const * l_i2c_ifcs = NULL;
static size_t                     l_i2c_ifcs_len = 0U;
static I2C_HandleTypeDef*         expected_i2c = NULL;

size_t get_iic_ifcs_stub(I2C_HandleTypeDef* const ** const i2c_ifcs_buffer)
{
    *i2c_ifcs_buffer = l_i2c_ifcs;
    // Return the number of I2C interfaces
    return ARRAY_SIZE(l_i2c_ifcs);
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_fake(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint8_t* pData,
                                              uint16_t Size, uint32_t Timeout)
{
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_hi2c, hi2c);
    TEST_ASSERT_EQUAL(i2c_expect.expected_DevAddress, DevAddress);
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_pData, pData);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Size, Size);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Timeout, Timeout);
    return i2c_expect.return_val;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_fake(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint8_t* pData,
                                               uint16_t Size, uint32_t Timeout)
{
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_hi2c, hi2c);
    TEST_ASSERT_EQUAL(i2c_expect.expected_DevAddress, DevAddress);
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_pData, pData);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Size, Size);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Timeout, Timeout);
    return i2c_expect.return_val;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read_fake(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
                                        uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_hi2c, hi2c);
    TEST_ASSERT_EQUAL(i2c_expect.expected_DevAddress, DevAddress);
    TEST_ASSERT_EQUAL(i2c_expect.expected_MemAddress, MemAddress);
    TEST_ASSERT_EQUAL(i2c_expect.expected_MemAddSize, MemAddSize);
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_pData, pData);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Size, Size);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Timeout, Timeout);
    return i2c_expect.return_val;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write_fake(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
                                         uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_hi2c, hi2c);
    TEST_ASSERT_EQUAL(i2c_expect.expected_DevAddress, DevAddress);
    TEST_ASSERT_EQUAL(i2c_expect.expected_MemAddress, MemAddress);
    TEST_ASSERT_EQUAL(i2c_expect.expected_MemAddSize, MemAddSize);
    TEST_ASSERT_EQUAL_PTR(i2c_expect.expected_pData, pData);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Size, Size);
    TEST_ASSERT_EQUAL(i2c_expect.expected_Timeout, Timeout);
    return i2c_expect.return_val;
}

void setUp(void)
{
    g_i2c_driver = iic_driver_register();
    static I2C_HandleTypeDef* i2c_ifcs[] = { &hi2c1, &hi2c2 };
    l_i2c_ifcs = i2c_ifcs;
    l_i2c_ifcs_len = 2;
    get_iic_ifcs_StubWithCallback(get_iic_ifcs_stub);
    g_i2c_driver->api->init();
}

void tearDown(void) {}

void test_iic_init_with_null_interface_should_return_error(void)
{
    l_i2c_ifcs = NULL; // Simulate driver not initialized
    get_iic_ifcs_StubWithCallback(get_iic_ifcs_stub);

    response_status_t ret_val = g_i2c_driver->api->init();

    TEST_ASSERT_EQUAL(RET_ERROR, ret_val);
}

void test_iic_init_with_zero_interface_should_return_not_supported(void)
{
    l_i2c_ifcs_len = 0; // Simulate driver not initialized
    get_iic_ifcs_StubWithCallback(get_iic_ifcs_stub);

    response_status_t ret_val = g_i2c_driver->api->init();

    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ret_val);
}

void test_iic_init_with_any_null_interface_should_return_error(void)
{
    static I2C_HandleTypeDef* i2c_ifcs[] = { &hi2c1, NULL };
    l_i2c_ifcs = i2c_ifcs;
    l_i2c_ifcs_len = 2;
    get_iic_ifcs_StubWithCallback(get_iic_ifcs_stub);

    response_status_t ret_val = g_i2c_driver->api->init();

    TEST_ASSERT_EQUAL(RET_ERROR, ret_val);
}

void test_iic_init_with_valid_interfaces_should_return_ok(void)
{
    static I2C_HandleTypeDef* i2c_ifcs[] = { &hi2c1, &hi2c2 };
    l_i2c_ifcs = i2c_ifcs;
    l_i2c_ifcs_len = 2;
    get_iic_ifcs_StubWithCallback(get_iic_ifcs_stub);

    response_status_t ret_val = g_i2c_driver->api->init();

    TEST_ASSERT_EQUAL(RET_OK, ret_val);
}

void test_iic_master_write_with_invalid_index_should_return_not_supported(void)
{
    uint8_t           data[] = { 0x01, 0x02, 0x03 };
    response_status_t ret_val = g_i2c_driver->api->write(2, 0x50, data, sizeof(data), 100);

    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ret_val);
}

void test_iic_master_write_with_null_data_should_return_param_error(void)
{
    response_status_t ret_val = g_i2c_driver->api->write(0, 0x50, NULL, 10, 100);

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ret_val);
}

void test_iic_master_write_with_zero_length_should_return_param_error(void)
{
    uint8_t           data[] = { 0x01, 0x02, 0x03 };
    response_status_t ret_val = g_i2c_driver->api->write(0, 0x50, data, 0, 100);

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ret_val);
}

void test_iic_master_write_with_valid_parameters_should_return_ok(void)
{
    uint8_t data[] = { 0x01, 0x02, 0x03 };

    HAL_I2C_Master_Transmit_StubWithCallback(HAL_I2C_Master_Transmit_fake);
    i2c_expect.expected_hi2c = &hi2c1;
    i2c_expect.expected_DevAddress = 0x50 << 1;
    i2c_expect.expected_pData = data;
    i2c_expect.expected_Size = sizeof(data);
    i2c_expect.expected_Timeout = 100;
    i2c_expect.return_val = HAL_OK;
    response_status_t ret_val = g_i2c_driver->api->write(0, 0x50, data, sizeof(data), 100);

    TEST_ASSERT_EQUAL(RET_OK, ret_val);
}

void test_iic_master_read_with_invalid_index_should_return_not_supported(void)
{
    uint8_t           data[10];
    response_status_t ret_val = g_i2c_driver->api->read(2, 0x50, data, sizeof(data), 100);

    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ret_val);
}

void test_iic_master_read_with_null_data_should_return_param_error(void)
{
    response_status_t ret_val = g_i2c_driver->api->read(0, 0x50, NULL, 10, 100);

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ret_val);
}

void test_iic_master_read_with_zero_length_should_return_param_error(void)
{
    uint8_t           data[10];
    response_status_t ret_val = g_i2c_driver->api->read(0, 0x50, data, 0, 100);

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ret_val);
}

void test_iic_master_read_with_valid_parameters_should_return_ok(void)
{
    uint8_t data[10];

    HAL_I2C_Master_Receive_StubWithCallback(HAL_I2C_Master_Receive_fake);
    i2c_expect.expected_hi2c = &hi2c1;
    i2c_expect.expected_DevAddress = 0x50 << 1;
    i2c_expect.expected_pData = data;
    i2c_expect.expected_Size = sizeof(data);
    i2c_expect.expected_Timeout = 100;
    i2c_expect.return_val = HAL_OK;
    response_status_t ret_val = g_i2c_driver->api->read(0, 0x50, data, sizeof(data), 100);

    TEST_ASSERT_EQUAL(RET_OK, ret_val);
}

