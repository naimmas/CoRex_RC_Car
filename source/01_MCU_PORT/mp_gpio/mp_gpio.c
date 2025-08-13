
#include "mp_gpio.h"

#include "ha_gpio/ha_gpio_private.h"
#include "main.h"
#include "su_common.h"

typedef struct st_stm32_gpio_driver
{
    gpio_driver_t         base;
    GPIO_TypeDef* const * port;
    const uint16_t*       pin;
} stm32_gpio_driver_t;

static stm32_gpio_driver_t g_gpio_drv = { .base = { 0U }, .port = NULL, .pin = NULL };

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;

    g_gpio_drv.base.hw_pin_cnt = get_gpio_pins(&g_gpio_drv.port, &g_gpio_drv.pin);

    if (g_gpio_drv.port == NULL || g_gpio_drv.pin == NULL)
    {
        ret_val = RET_ERROR;
    }
    else if (g_gpio_drv.base.hw_pin_cnt == 0U)
    {
        ret_val = RET_NOT_SUPPORTED;
    }
    else
    {
        for (uint16_t i = 0; i < g_gpio_drv.base.hw_pin_cnt; ++i)
        {
            if (g_gpio_drv.port[i] == NULL)
            {
                ret_val = RET_ERROR;
                break;
            }
        }
    }

    return ret_val;
}

static response_status_t write(uint8_t p_pin, bool_t p_value)
{
    ASSERT_AND_RETURN(g_gpio_drv.pin == NULL || g_gpio_drv.port == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_pin >= g_gpio_drv.base.hw_pin_cnt, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    HAL_GPIO_WritePin(g_gpio_drv.port[p_pin], g_gpio_drv.pin[p_pin], (uint8_t)p_value);

    return ret_val;
}

static response_status_t read(uint8_t p_pin, bool_t* ppt_value)
{
    ASSERT_AND_RETURN(g_gpio_drv.pin == NULL || g_gpio_drv.port == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_pin >= g_gpio_drv.base.hw_pin_cnt, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(ppt_value == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(g_gpio_drv.port[p_pin], g_gpio_drv.pin[p_pin]);

    if (pin_state == GPIO_PIN_SET)
    {
        *ppt_value = TRUE;
    }
    else
    {
        *ppt_value = FALSE;
    }

    return ret_val;
}

static response_status_t toggle(uint8_t p_pin)
{
    ASSERT_AND_RETURN(g_gpio_drv.pin == NULL || g_gpio_drv.port == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_pin >= g_gpio_drv.base.hw_pin_cnt, RET_PARAM_ERROR);

    HAL_GPIO_TogglePin(g_gpio_drv.port[p_pin], g_gpio_drv.pin[p_pin]);

    return RET_OK;
}

static struct st_gpio_driver_ifc g_interface = { .init   = init,
                                                 .write  = write,
                                                 .read   = read,
                                                 .toggle = toggle };

gpio_driver_t* gpio_driver_register(void)
{
    g_gpio_drv.base.api = &g_interface;
    return (struct st_gpio_driver*)&g_gpio_drv;
}
