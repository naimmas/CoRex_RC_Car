
#ifndef HA_TIMER_H
#define HA_TIMER_H

#include "su_common.h"

void ha_timer_hard_delay_ms(uint32_t p_delay_ms);
uint32_t ha_timer_get_tick(void);

#endif // HA_TIMER_H
