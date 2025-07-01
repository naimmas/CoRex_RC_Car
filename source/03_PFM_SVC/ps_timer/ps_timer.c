#include "ps_timer.h"

#include "ha_timer/ha_timer.h"

void ps_hard_delay_ms(uint32_t p_delay_ms)
{
    ha_timer_hard_delay_ms(p_delay_ms);
}
