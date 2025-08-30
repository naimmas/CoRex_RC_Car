#ifndef PS_APP_TIMER_H
#define PS_APP_TIMER_H

#include "su_common.h"

#define MAX_TIMER_PERIOD (60000000U)
#define MAX_USER_TIMER (10U)
typedef void (*app_timer_callback_t)(void);

typedef enum
{
    APP_TIMER_UNIT_US,
    APP_TIMER_UNIT_MS,
    APP_TIMER_UNIT_S,
    APP_TIMER_UNIT_MIN,
} app_timer_unit_t;

typedef struct
{
    bool_t          is_running; // Timer running state
    volatile bool_t is_fired;   // Timer fired state
} app_timer_handler_t;

response_status_t ps_app_timer_init(void);
response_status_t ps_app_timer_create(app_timer_handler_t** ppt_timer_handler, bool_t p_oneshot_timer,
                                      app_timer_callback_t ppt_callback);
response_status_t ps_app_timer_delete(app_timer_handler_t* ppt_timer_handler);
response_status_t ps_app_timer_start(app_timer_handler_t* ppt_timer_handler, uint32_t p_timer_period,
                                     app_timer_unit_t p_time_unit);
response_status_t ps_app_timer_stop(app_timer_handler_t* ppt_timer_handler);
response_status_t ps_app_timer_update_period(app_timer_handler_t* ppt_timer_handler,
                                             uint32_t p_new_period, app_timer_unit_t p_time_unit);
#endif // PS_APP_TIMER_H
