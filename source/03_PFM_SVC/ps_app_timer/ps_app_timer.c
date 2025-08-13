#include "ps_app_timer.h"

#include "ha_input_capture/ha_input_capture.h"
#include "ha_timer/ha_timer.h"
#include "string.h"

#define SET_ACTIVE_TIMER_MASK(timer_type, timer_id) \
    (g_active_timers_msk[timer_type] |= (1U << timer_id))
#define CLEAR_ACTIVE_TIMER_MASK(timer_type, timer_id) \
    (g_active_timers_msk[timer_type] &= ~(1U << timer_id))

typedef struct
{
    app_timer_handler_t  user_timer_handler;
    uint32_t             timer_period; // Timer period in ticks
    volatile uint32_t    timer_counter;
    app_timer_callback_t callback; // User callback function
    gp_timers_t          timer_type;
    bool_t               one_shot; // Timer type (oneshot or periodic)
    int8_t               timer_id; // Unique timer ID
} app_timer_t;

static app_timer_t       g_app_timer[MAX_USER_TIMER]    = { 0 };
static volatile uint32_t g_active_timers_msk[TIMER_CNT] = { 0 };

void ps_hard_delay_ms(uint32_t p_delay_ms)
{
    ha_timer_hard_delay_ms(p_delay_ms);
}

uint32_t ps_get_cpu_ms()
{
    return 0;
}

static response_status_t determine_timer_type_and_period(uint32_t         p_timer_period,
                                                         app_timer_unit_t p_time_unit,
                                                         uint32_t*        ppt_timer_counter,
                                                         gp_timers_t*     ppt_determined_type)
{
    response_status_t ret_val = RET_OK;

    switch (p_time_unit)
    {
        case APP_TIMER_UNIT_US:
            if (p_timer_period % 10 == 0 && p_timer_period <= MAX_TIMER_PERIOD)
            {
                *ppt_determined_type = TIMER_10US;
                *ppt_timer_counter   = p_timer_period / 10;
            }
            else
            {
                ret_val = RET_PARAM_ERROR; // Invalid period for microseconds
            }
            break;
        case APP_TIMER_UNIT_MS:
            if (p_timer_period % 100 == 0 && p_timer_period <= MAX_TIMER_PERIOD)
            {
                *ppt_determined_type = TIMER_100MS;
                *ppt_timer_counter   = p_timer_period / 100;
            }
            else if (p_timer_period % 10 == 0 && p_timer_period <= MAX_TIMER_PERIOD)
            {
                *ppt_determined_type = TIMER_10MS;
                *ppt_timer_counter   = p_timer_period / 10;
            }
            else if (p_timer_period <= MAX_TIMER_PERIOD)
            {
                *ppt_determined_type = TIMER_1MS;
                *ppt_timer_counter   = p_timer_period;
            }
            else
            {
                ret_val = RET_PARAM_ERROR;
            }
            break;
        case APP_TIMER_UNIT_S:
            if ((p_timer_period * 10) <= MAX_TIMER_PERIOD)
            {
                *ppt_determined_type = TIMER_100MS;
                *ppt_timer_counter   = p_timer_period * 10;
            }
            else
            {
                ret_val = RET_PARAM_ERROR;
            }
            break;
        case APP_TIMER_UNIT_MIN:
            if ((p_timer_period * (10 * 60)) <= MAX_TIMER_PERIOD)
            {
                *ppt_determined_type = TIMER_100MS;
                *ppt_timer_counter   = p_timer_period * (10 * 60);
            }
            else
            {
                ret_val = RET_PARAM_ERROR;
            }
            break;
    }
    return ret_val;
}
static int8_t find_free_timer(void)
{
    for (size_t i = 0; i < MAX_USER_TIMER; i++)
    {
        if (g_app_timer[i].timer_id == -1)
        {
            return i;
        }
    }
    return -1; // No free timer found
}

static void dispatch_user_timer(gp_timers_t p_timer_type)
{
    uint32_t active_mask = g_active_timers_msk[p_timer_type];

    while (active_mask != 0)
    {
        uint8_t      timer_id = __builtin_ctz(active_mask);
        app_timer_t* timer    = &g_app_timer[timer_id];

        timer->timer_counter++;

        if (timer->timer_counter >= timer->timer_period)
        {
            timer->user_timer_handler.is_fired = TRUE;

            if (timer->callback != NULL)
            {
                timer->callback();
            }

            if (timer->one_shot == TRUE)
            {
                ps_app_timer_stop(&timer->user_timer_handler); // Stop the timer if it's one-shot
            }
            else
            {
                timer->timer_counter = 0; // Reset counter for periodic timers
            }
        }
        active_mask &= ~(1U << timer_id); // Clear the bit for this timer
    }
}
static void timer_10us_cb(void)
{
    dispatch_user_timer(TIMER_10US);
}
static void timer_1ms_cb(void)
{
    dispatch_user_timer(TIMER_1MS);
}
static void timer_10ms_cb(void)
{
    dispatch_user_timer(TIMER_10MS);
}
static void timer_100ms_cb(void)
{
    dispatch_user_timer(TIMER_100MS);
}

response_status_t ps_app_timer_init(void)
{
    response_status_t ret_val = RET_OK;

    // Initialize the timer driver
    ret_val = ha_timer_init();
    if (ret_val == RET_OK)
    {
        for (size_t i = 0; i < MAX_USER_TIMER; i++)
        {
            g_app_timer[i].user_timer_handler.is_running = FALSE;
            g_app_timer[i].user_timer_handler.is_fired   = FALSE;
            g_app_timer[i].timer_id                      = -1; // Mark as unused
            g_app_timer[i].timer_period                  = 0;
            g_app_timer[i].timer_counter                 = 0;
        }
        ret_val  = ha_timer_register_callback(TIMER_10US, timer_10us_cb);
        ret_val |= ha_timer_register_callback(TIMER_1MS, timer_1ms_cb);
        ret_val |= ha_timer_register_callback(TIMER_10MS, timer_10ms_cb);
        ret_val |= ha_timer_register_callback(TIMER_100MS, timer_100ms_cb);
    }

    return ret_val;
}

response_status_t ps_app_timer_create(app_timer_handler_t** p_timer_handler, bool_t p_oneshot_timer,
                                      app_timer_callback_t p_callback)
{
    ASSERT_AND_RETURN(p_timer_handler == NULL, RET_PARAM_ERROR);

    response_status_t ret_val   = RET_OK;
    app_timer_t*      timer     = NULL;
    int8_t            timer_idx = find_free_timer();
    if (timer_idx != -1)
    {
        timer                                = &g_app_timer[timer_idx];
        timer->one_shot                      = p_oneshot_timer;
        timer->callback                      = p_callback;
        timer->timer_id                      = timer_idx;
        timer->timer_counter                 = 0;
        timer->timer_period                  = 0;
        timer->user_timer_handler.is_running = FALSE;
        timer->user_timer_handler.is_fired   = FALSE;
        *p_timer_handler                     = &timer->user_timer_handler;
        ret_val                              = RET_OK;
    }
    else
    {
        ret_val = RET_NO_MEMORY;
    }

    return ret_val;
}

response_status_t ps_app_timer_delete(app_timer_handler_t* p_timer_handler)
{
    ASSERT_AND_RETURN(p_timer_handler == NULL, RET_PARAM_ERROR);
    response_status_t ret_val = RET_OK;
    app_timer_t*      timer   = (app_timer_t*)p_timer_handler;

    ret_val = ps_app_timer_stop(p_timer_handler);

    if (ret_val == RET_OK)
    {
        memset(timer, 0, sizeof(app_timer_t)); // Reset the timer structure
        timer->timer_id = -1;                  // Mark as unused
    }

    return ret_val;
}

response_status_t ps_app_timer_start(app_timer_handler_t* p_timer_handler, uint32_t p_timer_period,
                                     app_timer_unit_t p_time_unit)
{
    ASSERT_AND_RETURN(p_timer_handler == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timer_period > MAX_TIMER_PERIOD, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(((app_timer_t*)p_timer_handler)->timer_id == -1, RET_NOT_INITIALIZED);

    response_status_t ret_val = RET_OK;
    app_timer_t*      timer   = (app_timer_t*)p_timer_handler;

    // Find the timer type based on the time unit and period
    gp_timers_t timer_type_to_use;
    uint32_t    period_to_use = 0;

    if (determine_timer_type_and_period(p_timer_period,
                                        p_time_unit,
                                        &period_to_use,
                                        &timer_type_to_use)
        != RET_OK)
    {
        ret_val = RET_PARAM_ERROR; // Invalid timer type
    }
    else
    {
        // If no active timer using this type, start the hardware timer
        if (g_active_timers_msk[timer_type_to_use] == 0)
        {
            ret_val = ha_timer_start(timer_type_to_use);
        }
    }

    if (ret_val == RET_OK)
    {
        timer->timer_type                    = timer_type_to_use;
        timer->timer_period                  = period_to_use;
        timer->user_timer_handler.is_running = TRUE;
        timer->user_timer_handler.is_fired   = FALSE;
        timer->timer_counter                 = 0;
        SET_ACTIVE_TIMER_MASK(timer->timer_type, timer->timer_id);
    }

    return ret_val;
}

response_status_t ps_app_timer_stop(app_timer_handler_t* p_timer_handler)
{
    ASSERT_AND_RETURN(p_timer_handler == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;
    app_timer_t*      timer   = (app_timer_t*)p_timer_handler;

    p_timer_handler->is_running = FALSE;

    CLEAR_ACTIVE_TIMER_MASK(timer->timer_type, timer->timer_id);

    // If no active timer using this type, stop the hardware timer
    if (g_active_timers_msk[timer->timer_type] == 0)
    {
        ret_val = ha_timer_stop(timer->timer_type);
    }

    return ret_val;
}
