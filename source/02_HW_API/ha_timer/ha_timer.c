
#include "ha_timer.h"

#include "mp_timer/mp_timer.h"

void ha_timer_hard_delay_ms(uint32_t p_delay_ms)
{
    mp_timer_hard_delay_ms(p_delay_ms);
}

uint32_t ha_timer_get_tick(void)
{
    return mp_timer_get_tick();
}