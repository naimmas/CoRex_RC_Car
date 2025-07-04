
#include "mp_timer.h"

#include "main.h"

void mp_timer_hard_delay_ms(uint32_t p_delay_ms)
{
    HAL_Delay(p_delay_ms);
}

uint32_t mp_timer_get_tick(void)
{
    return HAL_GetTick(); // uwTick is defined in stm32f4xx_hal.h
}
