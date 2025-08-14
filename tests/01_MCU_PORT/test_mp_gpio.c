#ifdef TEST

#include "mock_main.h"
#include "mock_stm32f4xx_hal_gpio.h"
#include "mp_gpio.h"
#include "su_common.h"
#include "unity.h"

#define PORT1 GPIOA
#define PORT2 GPIOB
#define PORT3 GPIOC

#define PIN1 GPIO_PIN_0
#define PIN2 GPIO_PIN_1
#define PIN3 GPIO_PIN_2

#undef ARRAY_SIZE
#define ARRAY_SIZE(x) x##_len

gpio_driver_t* g_gpio_driver = NULL;

static GPIO_TypeDef* const * l_hw_ports = NULL;
static const uint16_t*       l_hw_pins = NULL;
static int                   l_hw_ports_len = 0;
static int                   l_hw_pins_len = 0;

size_t get_gpio_pins_stub(GPIO_TypeDef* const ** const port, const uint16_t** pin, size_t cmock_num_calls)
{
    if (ARRAY_SIZE(l_hw_ports) != ARRAY_SIZE(l_hw_pins))
    {
        return 0;
    }

    *port = l_hw_ports;
    *pin = l_hw_pins;

    return ARRAY_SIZE(l_hw_ports);
}

void setUp(void)
{
    g_gpio_driver = gpio_driver_register();

    static GPIO_TypeDef* const hw_ports[] = { PORT1, PORT2, PORT3 };
    l_hw_ports = hw_ports;
    l_hw_ports_len = 3U;

    static const uint16_t hw_pins[] = { PIN1, PIN2, PIN3 };
    l_hw_pins = hw_pins;
    l_hw_pins_len = 3U;
}

void tearDown(void) {}

/********* Tests for init GPIO driver *********/

void test_gpio_init_with_null_ports_or_pins_should_return_error(void)
{
    get_gpio_pins_StubWithCallback(get_gpio_pins_stub);

    l_hw_ports = NULL;
    TEST_ASSERT_EQUAL(RET_ERROR, g_gpio_driver->api->init());

    l_hw_pins = NULL;
    TEST_ASSERT_EQUAL(RET_ERROR, g_gpio_driver->api->init());
}

void test_gpio_init_with_no_hardware_pins_should_return_not_supported(void)
{
    static GPIO_TypeDef* const hw_ports[] = {};
    l_hw_ports_len = 0U;
    l_hw_ports = hw_ports;

    static const uint16_t hw_pins[] = {};
    l_hw_pins_len = 0U;
    l_hw_pins = hw_pins;

    get_gpio_pins_StubWithCallback(get_gpio_pins_stub);
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, g_gpio_driver->api->init());
}

void test_gpio_init_with_mismatched_ports_and_pins_arrays_should_return_not_supported(void)
{
    static GPIO_TypeDef* const hw_ports[] = { PORT1, PORT2 };
    l_hw_ports_len = 2U;
    l_hw_ports = hw_ports;

    static const uint16_t hw_pins[] = { PIN1 };
    l_hw_pins_len = 1U;
    l_hw_pins = hw_pins;

    get_gpio_pins_StubWithCallback(get_gpio_pins_stub);
    TEST_ASSERT_EQUAL(RET_NOT_SUPPORTED, g_gpio_driver->api->init());
}

void test_gpio_init_with_valid_ports_and_pins_should_return_ok(void)
{
    get_gpio_pins_StubWithCallback(get_gpio_pins_stub);

    TEST_ASSERT_EQUAL(RET_OK, g_gpio_driver->api->init());
}

/********* Tests for write GPIO driver *********/

void test_gpio_write_with_out_of_bound_pin_index_should_return_param_error(void)
{
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_gpio_driver->api->write(ARRAY_SIZE(l_hw_pins) + 1, 0));
}

void test_gpio_write_with_valid_pin_index_should_return_ok(void)
{
    for (int i = 0; i < ARRAY_SIZE(l_hw_pins); i++)
    {
        HAL_GPIO_WritePin_Expect(l_hw_ports[i], l_hw_pins[i], GPIO_PIN_RESET);
        TEST_ASSERT_EQUAL(RET_OK, g_gpio_driver->api->write(i, 0));
        HAL_GPIO_WritePin_Expect(l_hw_ports[i], l_hw_pins[i], GPIO_PIN_SET);
        TEST_ASSERT_EQUAL(RET_OK, g_gpio_driver->api->write(i, 1));
    }
}

/********* Tests for read GPIO driver *********/

void test_gpio_read_with_null_value_pointer_should_return_param_error(void)
{
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_gpio_driver->api->read(0, NULL));
}

void test_gpio_read_with_out_of_bound_pin_index_should_return_param_error(void)
{
    bool_t value = 0;
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_gpio_driver->api->read(ARRAY_SIZE(l_hw_pins) + 1, &value));
}

void test_gpio_read_with_valid_pin_index_should_return_ok(void)
{
    for (int i = 0; i < ARRAY_SIZE(l_hw_pins); i++)
    {
        HAL_GPIO_ReadPin_ExpectAndReturn(l_hw_ports[i], l_hw_pins[i], GPIO_PIN_RESET);
        bool_t value = 0;
        TEST_ASSERT_EQUAL(RET_OK, g_gpio_driver->api->read(i, &value));
        TEST_ASSERT_EQUAL(0, value);

        HAL_GPIO_ReadPin_ExpectAndReturn(l_hw_ports[i], l_hw_pins[i], GPIO_PIN_SET);
        TEST_ASSERT_EQUAL(RET_OK, g_gpio_driver->api->read(i, &value));
        TEST_ASSERT_EQUAL(1, value);
    }
}

/********* Tests for toggle GPIO driver *********/

void test_gpio_toggle_with_out_of_bound_pin_index_should_return_param_error(void)
{
    TEST_ASSERT_EQUAL(RET_PARAM_ERROR, g_gpio_driver->api->toggle(ARRAY_SIZE(l_hw_pins) + 1));
}

void test_gpio_toggle_with_valid_pin_index_should_return_ok(void)
{
    for (int i = 0; i < ARRAY_SIZE(l_hw_pins); i++)
    {
        HAL_GPIO_TogglePin_Expect(l_hw_ports[i], l_hw_pins[i]);
        TEST_ASSERT_EQUAL(RET_OK, g_gpio_driver->api->toggle(i));
    }
}

#endif // TEST