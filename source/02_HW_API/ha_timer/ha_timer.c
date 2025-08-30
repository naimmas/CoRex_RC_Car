
#include "ha_timer.h"

#include "ha_timer_private.h"
#include "mp_timer/mp_timer_general.h"

static timer_driver g_pt_timer_drv    = NULL;
static bool_t       g_timer_drv_ready = FALSE;

response_status_t ha_timer_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_timer_drv_ready == FALSE)
    {
        g_pt_timer_drv = timer_driver_register();
        if (g_pt_timer_drv == NULL)
        {
            ret_val = RET_ERROR;
        }
        else
        {
            ret_val = g_pt_timer_drv->api->init();
            if (ret_val == RET_OK)
            {
                g_timer_drv_ready = TRUE;
            }
        }
    }

    return ret_val;
}

response_status_t ha_timer_start(gp_timers_t p_timer)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);

    return g_pt_timer_drv->api->start(p_timer);
}

response_status_t ha_timer_stop(gp_timers_t p_timer)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);

    return g_pt_timer_drv->api->stop(p_timer);
}

response_status_t ha_timer_get_state(gp_timers_t p_timer)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);

    UNUSED(p_timer);

    response_status_t ret_val = g_pt_timer_drv->api->get_state();

    return ret_val;
}

response_status_t ha_timer_get_frequency(gp_timers_t p_timer, uint32_t* ppt_frequency)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_frequency == NULL, RET_PARAM_ERROR);

    UNUSED(p_timer);

    response_status_t ret_val = g_pt_timer_drv->api->get_frequency(ppt_frequency);

    return ret_val;
}

response_status_t ha_timer_register_callback(gp_timers_t p_timer, void (*ppt_callback)(void))
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer >= TIMER_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_callback == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = g_pt_timer_drv->api->register_callback((uint8_t)p_timer, ppt_callback);

    return ret_val;
}

void ha_timer_hard_delay_ms(uint32_t p_delay_ms)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, );
    g_pt_timer_drv->api->hard_delay(p_delay_ms, MP_TIMER_UNIT_MS);
}

void ha_timer_hard_delay_us(uint32_t p_delay_ms)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, );
    g_pt_timer_drv->api->hard_delay(p_delay_ms, MP_TIMER_UNIT_US);
}

uint32_t ha_timer_get_cpu_time_ms(void)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, 0);
    return g_pt_timer_drv->api->get_cpu_time(MP_TIMER_UNIT_MS);
}

uint32_t ha_timer_get_cpu_time_us(void)
{
    ASSERT_AND_RETURN(g_timer_drv_ready != TRUE, 0);
    return g_pt_timer_drv->api->get_cpu_time(MP_TIMER_UNIT_US);
}
