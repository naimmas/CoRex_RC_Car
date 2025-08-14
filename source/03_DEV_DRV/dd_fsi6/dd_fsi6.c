
#include "dd_fsi6.h"

#include "ha_input_capture/ha_input_capture.h"
#include "ha_timer/ha_timer.h"
#include "ps_app_timer/ps_app_timer.h"
#include "su_common.h"

#include <string.h>

typedef struct
{
    uint32_t             value[FSI6_IN_CNT];
    fsi6_inputs_t        channel_to_in[FSI6_IN_CNT];
    bool_t               initialized;
    volatile bool_t      waiting_for_data;
    app_timer_handler_t* timeout_handler;
    volatile bool_t      timeout_occurred;
} fsi6_device_t;

fsi6_device_t g_fsi6_dev = { .initialized = FALSE, .waiting_for_data = FALSE, .channel_to_in = {
    [INPUT_CAPTURE_CHANNEL_1] = FSI6_IN_L_S_UD,
    [INPUT_CAPTURE_CHANNEL_2] = FSI6_IN_R_S_LR,
} , .timeout_handler = NULL, .timeout_occurred = FALSE };

static input_capture_channel_t in2ch(fsi6_inputs_t input)
{
    switch (input)
    {
        case FSI6_IN_L_S_UD:
            return INPUT_CAPTURE_CHANNEL_1;
        case FSI6_IN_R_S_LR:
            return INPUT_CAPTURE_CHANNEL_2;
        default:
            return INPUT_CAPTURE_CHANNEL_CNT; // Invalid input
    }
}

void timeout_cb(void)
{
    g_fsi6_dev.timeout_occurred = TRUE;
}

void ic_api_cb(input_capture_channel_t channel, uint32_t value)
{
    if (g_fsi6_dev.initialized && g_fsi6_dev.waiting_for_data)
    {
        g_fsi6_dev.value[g_fsi6_dev.channel_to_in[channel]] = value;
        g_fsi6_dev.waiting_for_data                         = FALSE;
    }
    else
    {
        // Handle error or unexpected callback
    }
}

response_status_t dd_fsi6_init(void)
{
    response_status_t ret_val = RET_OK;

    memset(&(g_fsi6_dev.value), 0, sizeof(g_fsi6_dev.value));

    ret_val = ha_input_capture_init();
    if (ret_val == RET_OK)
    {
        for (int i = 0; i < FSI6_IN_CNT; i++)
        {
            ret_val |= ha_input_capture_register_callback(g_fsi6_dev.channel_to_in[i], ic_api_cb);
        }
    }

    if (ret_val == RET_OK)
    {
        ret_val  = ps_app_timer_init();
        ret_val |= ps_app_timer_create(&(g_fsi6_dev.timeout_handler), TRUE, timeout_cb);
    }

    if (ret_val == RET_OK)
    {
        g_fsi6_dev.initialized = TRUE;
    }

    return ret_val;
}

response_status_t dd_fsi6_read_input(fsi6_inputs_t input, uint32_t* value)
{
    ASSERT_AND_RETURN(value == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(g_fsi6_dev.initialized == FALSE, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(input >= FSI6_IN_CNT, RET_PARAM_ERROR);

    response_status_t ret_val       = RET_OK;
    g_fsi6_dev.waiting_for_data     = TRUE;
    g_fsi6_dev.timeout_occurred     = FALSE;
    input_capture_channel_t channel = in2ch(input);

    ret_val = ps_app_timer_start(g_fsi6_dev.timeout_handler, 50, APP_TIMER_UNIT_MS);
    if (ret_val == RET_OK)
    {
        ret_val =
          ha_input_capture_request_capture(channel, IC_MEASURE_PULSE_WIDTH, IC_ONE_SHOT_CAPTURE);
    }

    while (g_fsi6_dev.waiting_for_data && !g_fsi6_dev.timeout_occurred)
    {
        // Wait for the callback to be called or timeout to occur
        ha_timer_hard_delay_ms(10); // Polling delay
    }

    if (g_fsi6_dev.timeout_occurred)
    {
        ret_val = RET_TIMEOUT;
        // ha_input_capture_abort(channel);
        *value                      = 0; // Return 0 if timeout occurred
        g_fsi6_dev.waiting_for_data = FALSE;
    }
    else
    {
        *value                      = g_fsi6_dev.value[g_fsi6_dev.channel_to_in[channel]];
        g_fsi6_dev.waiting_for_data = FALSE;
        ret_val                     = RET_OK;
    }

    return ret_val;
}
