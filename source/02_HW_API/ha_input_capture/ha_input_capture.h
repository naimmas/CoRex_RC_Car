
#ifndef HA_INPUT_CAPTURE_H
#define HA_INPUT_CAPTURE_H

#include "su_common.h"

typedef struct st_ic_driver* ic_driver;

typedef enum
{
    INPUT_CAPTURE_CHANNEL_1 = 0,
    INPUT_CAPTURE_CHANNEL_2,
    INPUT_CAPTURE_CHANNEL_CNT
} input_capture_channel_t;

typedef enum
{
    IC_CAPTURE_RISING_EDGE,
    IC_CAPTURE_FALLING_EDGE,
    IC_MEASURE_PULSE_WIDTH,
    IC_MEASURE_FREQUENCY,
    IC_CAPTURE_TYPE_CNT
} input_capture_type_t;

typedef enum
{
    IC_CONTINUOUS_CAPTURE = 0,
    IC_ONE_SHOT_CAPTURE   = 1,
} input_capture_mode_t;

typedef void (*ic_finished_callback_t)(input_capture_channel_t channel, uint32_t value);

response_status_t ha_input_capture_init(void);
response_status_t ha_input_capture_request_capture(input_capture_channel_t p_chnl,
                                                   input_capture_type_t    p_type,
                                                   input_capture_mode_t    p_capture_mode);
response_status_t ha_input_capture_register_callback(input_capture_channel_t p_chnl,
                                                     ic_finished_callback_t  p_callback);
response_status_t ha_input_capture_abort(input_capture_channel_t p_chnl);

#endif // HA_INPUT_CAPTURE_H
