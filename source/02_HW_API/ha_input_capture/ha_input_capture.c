
#include "ha_input_capture.h"

#include "mp_timer/mp_timer_capture.h"

static ic_driver g_pt_ic_drv    = NULL;
static bool_t    g_ic_drv_ready = FALSE;

response_status_t ha_input_capture_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_ic_drv_ready == FALSE)
    {
        g_pt_ic_drv = timer_capture_driver_register();
        if (g_pt_ic_drv == NULL)
        {
            ret_val = RET_ERROR;
        }
        else
        {
            ret_val = g_pt_ic_drv->api->init();
            if (ret_val == RET_OK)
            {
                g_ic_drv_ready = TRUE;
            }
        }
    }

    return ret_val;
}

response_status_t ha_input_capture_request_capture(input_capture_channel_t p_chnl,
                                                   input_capture_type_t    p_type,
                                                   input_capture_mode_t    p_capture_mode)
{
    ASSERT_AND_RETURN(!g_ic_drv_ready, RET_NOT_INITIALIZED);

    response_status_t ret_val = RET_OK;

    switch (p_type)
    {
        case IC_CAPTURE_RISING_EDGE:
        case IC_CAPTURE_FALLING_EDGE:
            ret_val = g_pt_ic_drv->api->capture_edge(p_chnl, p_type, p_capture_mode);
            break;
        case IC_MEASURE_PULSE_WIDTH:
            ret_val = g_pt_ic_drv->api->capture_pulse(p_chnl, p_capture_mode);
            break;
        case IC_MEASURE_FREQUENCY:
            ret_val = g_pt_ic_drv->api->capture_frequency(p_chnl, p_capture_mode);
            break;
        default:
            ret_val = RET_PARAM_ERROR;
            break;
    }

    return ret_val;
}

response_status_t ha_input_capture_abort(input_capture_channel_t p_chnl)
{
    ASSERT_AND_RETURN(!g_ic_drv_ready, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_chnl >= INPUT_CAPTURE_CHANNEL_CNT, RET_PARAM_ERROR);

    response_status_t ret_val = g_pt_ic_drv->api->stop_capture(p_chnl);

    return ret_val;
}

response_status_t ha_input_capture_register_callback(input_capture_channel_t p_chnl,
                                                     ic_finished_callback_t  ppt_callback)
{
    ASSERT_AND_RETURN(!g_ic_drv_ready, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(ppt_callback == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_ic_drv->api->register_callback(p_chnl, ppt_callback);

    return ret_val;
}
