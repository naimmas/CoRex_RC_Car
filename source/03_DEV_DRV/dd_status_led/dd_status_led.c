#include "dd_status_led.h"

#include "ha_gpio/ha_gpio.h"
#include "ps_app_timer/ps_app_timer.h"
#include "su_common.h"

app_timer_handler_t* g_led_timer              = NULL;
bool_t               g_status_led_initialized = FALSE;

void led_cb(void)
{
    ha_gpio_toggle(GP_PIN_LED);
}

response_status_t dd_status_led_init(void)
{
    if (g_status_led_initialized == TRUE)
    {
        return RET_OK; // Already initialized
    }

    response_status_t ret_val = RET_OK;

    ret_val  = ha_gpio_init();
    ret_val |= ha_gpio_set(GP_PIN_LED, GP_PIN_SET);

    if (ret_val == RET_OK)
    {
        ret_val = ps_app_timer_init();
    }

    if (ret_val == RET_OK)
    {
        ret_val  = ps_app_timer_create(&g_led_timer, FALSE, led_cb);
        ret_val |= ps_app_timer_start(g_led_timer, 2000, APP_TIMER_UNIT_MS);
    }

    if (ret_val == RET_OK)
    {
        g_status_led_initialized = TRUE;
    }
    else
    {
        g_status_led_initialized = FALSE;
    }

    return ret_val;
}

void dd_status_led_normal(void)
{
    ASSERT_AND_RETURN(g_status_led_initialized == FALSE, );
    ps_app_timer_update_period(g_led_timer, 1000, APP_TIMER_UNIT_MS);
}

void dd_status_led_error(void)
{
    ASSERT_AND_RETURN(g_status_led_initialized == FALSE, );
    ps_app_timer_update_period(g_led_timer, 500, APP_TIMER_UNIT_MS);
}
