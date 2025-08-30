
#ifndef HA_TIMER_H
#define HA_TIMER_H

#include "su_common.h"

typedef struct st_timer_driver* timer_driver;

typedef enum en_gp_timers
{
    TIMER_10US = 0,
    TIMER_1MS,
    TIMER_10MS,
    TIMER_100MS,
    TIMER_CNT,
} gp_timers_t;

response_status_t ha_timer_init(void);
response_status_t ha_timer_start(gp_timers_t p_timer);
response_status_t ha_timer_stop(gp_timers_t p_timer);
response_status_t ha_timer_get_state(gp_timers_t p_timer);
response_status_t ha_timer_get_frequency(gp_timers_t p_timer, uint32_t* ppt_frequency);
response_status_t ha_timer_register_callback(gp_timers_t p_timer, void (*ppt_callback)(void));
void              ha_timer_hard_delay_ms(uint32_t p_delay_ms);
void              ha_timer_hard_delay_us(uint32_t p_delay_ms);
uint32_t          ha_timer_get_cpu_time_ms(void);
uint32_t          ha_timer_get_cpu_time_us(void);
#endif // HA_TIMER_H
