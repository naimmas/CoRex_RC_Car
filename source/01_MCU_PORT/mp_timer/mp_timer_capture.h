#ifndef MP_TIMER_CAPTURE_H
#define MP_TIMER_CAPTURE_H

#include "ha_input_capture/ha_input_capture_private.h"

typedef enum
{
    MP_TIMER_CAP_TYPE_RISING_EDGE = 0,
    MP_TIMER_CAP_TYPE_FALLING_EDGE,
    MP_TIMER_CAP_TYPE_PULSE_WIDTH,
    MP_TIMER_CAP_TYPE_FREQUENCY,
    MP_TIMER_CAP_TYPE_CNT
} mp_timer_capture_type_t;

typedef enum
{
    MP_TIMER_CAP_CHNL_1 = 0,
    MP_TIMER_CAP_CHNL_2,
    MP_TIMER_CAP_CHNL_3,
    MP_TIMER_CAP_CHNL_CNT
} mp_timer_capture_channels_t;

timer_capture_driver_t* timer_capture_driver_register(void);

#endif // MP_TIMER_CAPTURE_H
