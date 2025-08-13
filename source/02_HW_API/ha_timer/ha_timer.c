
#include "ha_timer.h"

#include "ha_timer_private.h"
#include "mp_timer/mp_timer_general.h"

static timer_driver g_pt_driver          = NULL;
static bool_t       g_driver_initialized = FALSE;

response_status_t ha_timer_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_pt_driver == NULL)
    {
        g_pt_driver = timer_driver_register();
        if (g_pt_driver == NULL)
        {
            ret_val = RET_ERROR;
        }
        else
        {
            ret_val = g_pt_driver->api->init();
            if (ret_val == RET_OK)
            {
                g_driver_initialized = TRUE;
            }
        }
    }

    return ret_val;
}

response_status_t ha_timer_start(gp_timers_t p_timer)
{
    ASSERT_AND_RETURN(g_driver_initialized != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);

    return g_pt_driver->api->start(p_timer);
}

response_status_t ha_timer_stop(gp_timers_t p_timer)
{
    ASSERT_AND_RETURN(g_driver_initialized != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);

    return g_pt_driver->api->stop(p_timer);
}

response_status_t ha_timer_get_state(gp_timers_t p_timer)
{
    ASSERT_AND_RETURN(g_driver_initialized != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);

    UNUSED(p_timer);

    response_status_t ret_val = g_pt_driver->api->get_state();

    return ret_val;
}

response_status_t ha_timer_get_frequency(gp_timers_t p_timer, uint32_t* ppt_frequency)
{
    ASSERT_AND_RETURN(g_driver_initialized != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_frequency == NULL, RET_PARAM_ERROR);

    UNUSED(p_timer);

    response_status_t ret_val = g_pt_driver->api->get_frequency(ppt_frequency);

    return ret_val;
}

response_status_t ha_timer_register_callback(gp_timers_t p_timer, void (*callback)(void))
{
    ASSERT_AND_RETURN(g_driver_initialized != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(callback == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = g_pt_driver->api->register_callback((uint8_t)p_timer, callback);

    return ret_val;
}

void ha_timer_hard_delay_ms(uint32_t p_delay_ms)
{
    ASSERT_AND_RETURN(g_driver_initialized != TRUE,);
    g_pt_driver->api->hard_delay(p_delay_ms);
}