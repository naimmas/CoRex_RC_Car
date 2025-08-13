
#include "ha_input_capture.h"

#include "mp_timer/mp_timer_capture.h"

static ic_driver g_pt_driver          = NULL;
static bool_t    g_driver_initialized = FALSE;

response_status_t ha_input_capture_init(void)
{
    response_status_t ret_val = RET_OK;

    if (g_pt_driver == NULL)
    {
        g_pt_driver = timer_capture_driver_register();
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

response_status_t ha_input_capture_request_capture(input_capture_channel_t p_chnl,
                                                   input_capture_type_t    p_type,
                                                   input_capture_mode_t    p_capture_mode)
{
    ASSERT_AND_RETURN(!g_driver_initialized, RET_NOT_INITIALIZED);

    response_status_t ret_val = RET_OK;

    switch (p_type)
    {
        case IC_CAPTURE_RISING_EDGE:
            ret_val = g_pt_driver->api->capture_edge(p_chnl, 1, p_capture_mode);
        case IC_CAPTURE_FALLING_EDGE:
            ret_val = g_pt_driver->api->capture_edge(p_chnl, 0, p_capture_mode);
            break;
        case IC_MEASURE_PULSE_WIDTH:
            ret_val = g_pt_driver->api->capture_pulse(p_chnl, p_capture_mode);
            break;
        case IC_MEASURE_FREQUENCY:
            ret_val = g_pt_driver->api->capture_frequency(p_chnl, p_capture_mode);
            break;
        default:
            ret_val = RET_PARAM_ERROR;
            break;
    }

    return ret_val;
}

response_status_t ha_input_capture_abort(input_capture_channel_t p_chnl)
{
    ASSERT_AND_RETURN(!g_driver_initialized, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_chnl >= INPUT_CAPTURE_CHANNEL_CNT, RET_PARAM_ERROR);

    response_status_t ret_val = g_pt_driver->api->abort(p_chnl);

    return ret_val;
}

response_status_t ha_input_capture_register_callback(input_capture_channel_t p_chnl, ic_finished_callback_t p_callback)
{
    ASSERT_AND_RETURN(!g_driver_initialized, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_callback == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    ret_val = g_pt_driver->api->register_callback(p_chnl, p_callback);

    return ret_val;
}
