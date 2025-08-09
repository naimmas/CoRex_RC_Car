
#ifndef MP_TIMER_H
#define MP_TIMER_H

#include "ha_timer/ha_timer_private.h"

typedef enum
{
    MP_TIMER_10US_ID = 0,
    MP_TIMER_1MS_ID,
    MP_TIMER_10MS_ID,
    MP_TIMER_100MS_ID,
    MP_TIMER_CNT
} mp_timer_id_t;

timer_driver_t* timer_driver_register(void);

#endif // MP_TIMER_H
