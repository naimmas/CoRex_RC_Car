#include "ha_gpio.h"
#include "mock_mp_gpio.h"
#include "su_common.h"
#include "unity.h"

uint8_t hw_gpio_port_reg = 0x00;

static response_status_t drv_init(void);
static response_status_t drv_write(uint8_t pin, bool_t value);
static response_status_t drv_read(uint8_t pin, bool_t* value);
static response_status_t drv_toggle(uint8_t pin);

struct st_gpio_driver_ifc fake_driver_ifc = {
    .init = drv_init,
    .write = drv_write,
    .read = drv_read,
    .toggle = drv_toggle,
};

struct st_gpio_driver fake_gpio_driver = {
    .api = &fake_driver_ifc,
    .hw_pin_cnt = GP_PIN_CNT,
};

static response_status_t drv_init(void)
{
    fake_gpio_driver.hw_pin_cnt = GP_PIN_CNT;
    hw_gpio_port_reg = 0x00;

    return RET_OK;
}
static response_status_t drv_write(uint8_t pin, bool_t value)
{
    if (value == 0)
    {
        hw_gpio_port_reg &= ~(1 << pin);
    }
    else
    {
        hw_gpio_port_reg |= (1 << pin);
    }

    return RET_OK;
}
static response_status_t drv_read(uint8_t pin, bool_t* value)
{
    if (hw_gpio_port_reg & (1 << pin))
    {
        *value = 1;
    }
    else
    {
        *value = 0;
    }

    return RET_OK;
}
static response_status_t drv_read_inv(uint8_t pin, bool_t* value)
{
    *value = 3U;
    return RET_OK;
}
static response_status_t drv_toggle(uint8_t pin)
{
    hw_gpio_port_reg ^= (1 << pin);

    return RET_OK;
}

void setUp(void) {}

void tearDown(void) {}

/********* Tests for GPIO driver *********/

void test_gpio_when_driver_not_initialized_should_return_error(void)
{
    gpio_pin_state_t pin_state;
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_gpio_set(GP_PIN_LED, GP_PIN_SET));
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_gpio_get(GP_PIN_IN_1, &pin_state));
    TEST_ASSERT_EQUAL(RET_NOT_INITIALIZED, ha_gpio_toggle(GP_PIN_LED));
}

void test_gpio_with_null_driver_should_return_error(void)
{
    gpio_driver_register_ExpectAndReturn(NULL);
    TEST_ASSERT_EQUAL(RET_ERROR, ha_gpio_init());
}

void test_gpio_init_with_valid_driver_should_return_ok(void)
{
    gpio_driver_register_ExpectAndReturn(&fake_gpio_driver);
    TEST_ASSERT_EQUAL(RET_OK, ha_gpio_init());
}

void test_gpio_with_out_of_bound_pin_should_return_param_error(void)
{
    gpio_pin_state_t pin_state = 0xff;

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_set(GP_PIN_CNT, GP_PIN_SET));
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_set(GP_PIN_CNT, GP_PIN_RESET));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_get(GP_PIN_CNT, &pin_state));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);
    TEST_ASSERT_EQUAL(0xff, pin_state);

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_toggle(GP_PIN_CNT));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);
}

void test_gpio_with_no_hardware_support_should_return_not_supported(void)
{
    gpio_pin_state_t pin_state = 0xff;

    fake_gpio_driver.hw_pin_cnt = 0;
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_gpio_set(GP_PIN_LED, GP_PIN_SET));
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_gpio_set(GP_PIN_LED, GP_PIN_RESET));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);

    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_gpio_get(GP_PIN_LED, &pin_state));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);
    TEST_ASSERT_EQUAL(0xff, pin_state);

    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, ha_gpio_toggle(GP_PIN_LED));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);
    fake_gpio_driver.hw_pin_cnt = GP_PIN_CNT;
}

/********* Tests for GPIO driver with input pin *********/

void test_gpio_set_operation_on_input_pin_should_return_param_error(void)
{
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_set(GP_PIN_IN_1, GP_PIN_SET));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);
}

void test_gpio_set_operation_on_output_pin_should_return_ok(void)
{
    TEST_ASSERT_EQUAL(RET_OK, ha_gpio_set(GP_PIN_LED, GP_PIN_SET));
    TEST_ASSERT_BIT_HIGH(GP_PIN_LED, hw_gpio_port_reg);

    TEST_ASSERT_EQUAL(RET_OK, ha_gpio_set(GP_PIN_LED, GP_PIN_RESET));
    TEST_ASSERT_BIT_LOW(GP_PIN_LED, hw_gpio_port_reg);
}

/********* Tests for GPIO driver with i/o pins *********/

void test_gpio_get_operation_with_null_pointer_should_return_param_error(void)
{

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_get(GP_PIN_IN_1, NULL));
    TEST_ASSERT_EQUAL(0x00, hw_gpio_port_reg);
}

void test_gpio_get_operation_with_valid_pin_should_return_ok(void)
{
    hw_gpio_port_reg = 0xaa;
    gpio_pin_state_t pin_state = 0xff;

    char test_msg[50];
    for (uint8_t i = GP_PIN_LED; i < GP_PIN_CNT; i++)
    {
        TEST_ASSERT_EQUAL(RET_OK, ha_gpio_get(i, &pin_state));
        sprintf(test_msg, "pin no: %d", i);
        if (i % 2 == 0)
        {
            TEST_ASSERT_EQUAL_MESSAGE(GP_PIN_RESET, pin_state, test_msg);
        }
        else
        {
            TEST_ASSERT_EQUAL_MESSAGE(GP_PIN_SET, pin_state, test_msg);
        }
        TEST_ASSERT_EQUAL(0xaa, hw_gpio_port_reg);
    }
}

void test_gpio_toggle_operation_with_valid_pin_should_return_ok(void)
{
    hw_gpio_port_reg = 0x00;

    for (uint8_t i = GP_PIN_LED; i < GP_OUT_PIN_CNT; i++)
    {
        TEST_ASSERT_EQUAL(RET_OK, ha_gpio_toggle(i));
        TEST_ASSERT_BIT_HIGH(i, hw_gpio_port_reg);
    }

    hw_gpio_port_reg = 0xff;

    for (uint8_t i = GP_PIN_LED; i < GP_OUT_PIN_CNT; i++)
    {
        TEST_ASSERT_EQUAL(RET_OK, ha_gpio_toggle(i));
        TEST_ASSERT_BIT_LOW(i, hw_gpio_port_reg);
    }
}

void test_gpio_set_with_invalid_state(void)
{

    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, ha_gpio_set(GP_PIN_LED, GP_PIN_SET + 1));
}

void test_gpio_get_with_invalid_state(void)
{
    gpio_pin_state_t pin_val = 0xFF;
    fake_driver_ifc.read = drv_read_inv;

    TEST_ASSERT_EQUAL(RET_ERROR, ha_gpio_get(GP_PIN_LED, &pin_val));
}
