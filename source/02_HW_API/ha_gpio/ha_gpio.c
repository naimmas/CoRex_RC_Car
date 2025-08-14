
#include "ha_gpio.h"

#include "mp_gpio/mp_gpio.h"
#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

static gpio_driver g_pt_io_drv          = NULL;
static bool_t      g_io_drv_ready = FALSE;

/**
 * @brief This function sets the pin state to logic high or low.
 * @param[in] p_pin The pin number to change.
 * @param[in] p_value The value to set the pin to,
 * `GP_PIN_SET` for logic high,
 * `GP_PIN_RESET` for logic low.
 * @return Result of the operation.
 */
response_status_t ha_gpio_set(gpio_pins_t p_pin, gpio_pin_state_t p_value)
{
    ASSERT_AND_RETURN(g_io_drv_ready == FALSE, RET_NOT_INITIALIZED);
    /// Ensure the pin is an output pin
    ASSERT_AND_RETURN(p_pin >= GP_OUT_PIN_CNT, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_pin >= g_pt_io_drv->hw_pin_cnt, RET_NOT_SUPPORTED);

    bool_t            pin_val = 0U;
    response_status_t ret_val = RET_OK;

    switch (p_value)
    {
        case GP_PIN_RESET:
            pin_val = FALSE;
            break;
        case GP_PIN_SET:
            pin_val = TRUE;
            break;
        default:
            return RET_PARAM_ERROR;
    }

    ret_val = g_pt_io_drv->api->write(p_pin, pin_val);

    return ret_val;
}

/**
 * @brief This function reads the pin state.
 *
 * @param[in] p_pin The pin number to read.
 * @param[out] ppt_value Pointer to store the pin value.
 * @return Result of the operation.
 */
response_status_t ha_gpio_get(gpio_pins_t p_pin, gpio_pin_state_t* ppt_value)
{
    ASSERT_AND_RETURN(g_io_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_pin >= GP_PIN_CNT, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_pin >= g_pt_io_drv->hw_pin_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_value == NULL, RET_PARAM_ERROR);

    bool_t pin_val = 0U;

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_io_drv->api->read(p_pin, &pin_val);

    if (ret_val == RET_OK)
    {

        switch (pin_val)
        {
            case FALSE:
                *ppt_value = GP_PIN_RESET;
                break;
            case TRUE:
                *ppt_value = GP_PIN_SET;
                break;
            default:
                return RET_ERROR;
        }
    }

    return ret_val;
}

/**
 * @brief This function toggles the pin state.
 * It will set the pin to logic high if it is currently low,
 * or set it to logic low if it is currently high.
 *
 * @param[in] p_pin The pin number to toggle.
 * @return Result of the operation.
 */
response_status_t ha_gpio_toggle(gpio_pins_t p_pin)
{
    ASSERT_AND_RETURN(g_io_drv_ready == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_pin >= GP_OUT_PIN_CNT, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_pin >= g_pt_io_drv->hw_pin_cnt, RET_NOT_SUPPORTED);
    response_status_t ret_val = RET_OK;
    ret_val                   = g_pt_io_drv->api->toggle(p_pin);

    return ret_val;
}

/**
 * @brief This function initializes the GPIO driver once per power cycle. With
 * no consequences multiple calls.
 * @return Result of the operation.
 */
response_status_t ha_gpio_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_io_drv_ready != TRUE)
    {
        g_pt_io_drv = gpio_driver_register();

        if (g_pt_io_drv == NULL)
        {
            ret_val = RET_ERROR;
        }
        else
        {
            g_pt_io_drv->hw_pin_cnt = 0;
            ret_val                 = g_pt_io_drv->api->init();

            if (ret_val == RET_OK)
            {
                g_io_drv_ready = TRUE;
            }
        }
    }

    return ret_val;
}
