
#include "mp_timer_general.h"

#include "main.h"
#include "mp_common.h"
#include "su_common.h"

typedef struct st_stm32_timer_driver
{
    timer_driver_t     base;
    TIM_HandleTypeDef* hw_inst;
    uint32_t           sub_timers_periods[MP_TIMER_CNT];
    void (*timers_user_cb[MP_TIMER_CNT])(void);
} stm32_timer_driver_t;

static stm32_timer_driver_t g_timer_drv = {
    .base               = { 0U },
    .hw_inst            = NULL,
    .sub_timers_periods = { 0 },
    .timers_user_cb     = { NULL, NULL, NULL, NULL }
};

void hal_channel_tim_cb(TIM_HandleTypeDef* htim)
{
    volatile uint32_t* const ccrs[4] = { &htim->Instance->CCR1,
                                         &htim->Instance->CCR2,
                                         &htim->Instance->CCR3,
                                         &htim->Instance->CCR4 };
    uint8_t                  idx     = __builtin_ctz(htim->Channel);

    *(ccrs[idx]) += g_timer_drv.sub_timers_periods[idx];
    if (g_timer_drv.timers_user_cb[idx] != NULL)
    {
        g_timer_drv.timers_user_cb[idx]();
    }
}

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;

    get_base_tim_ifc(&(g_timer_drv.hw_inst));

    if (g_timer_drv.hw_inst != NULL)
    {
        g_timer_drv.sub_timers_periods[0] = g_timer_drv.hw_inst->Instance->CCR1;
        g_timer_drv.sub_timers_periods[1] = g_timer_drv.hw_inst->Instance->CCR2;
        g_timer_drv.sub_timers_periods[2] = g_timer_drv.hw_inst->Instance->CCR3;
        g_timer_drv.sub_timers_periods[3] = g_timer_drv.hw_inst->Instance->CCR4;
        // Start timer without interrupt to use its counter
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    }
    else
    {
        ret_val = RET_ERROR;
    }

    return ret_val;
}

static inline HAL_StatusTypeDef OC_StartPeriodic(mp_timer_id_t p_timer_id, uint32_t ch)
{
    HAL_StatusTypeDef hal_ret = HAL_OK;
    TIM_HandleTypeDef* htim = g_timer_drv.hw_inst;
    // 1) Mask this channel's interrupt and clear any stale flags
    uint32_t it_bit =
        (ch == TIM_CHANNEL_1) ? TIM_IT_CC1 :
        (ch == TIM_CHANNEL_2) ? TIM_IT_CC2 :
        (ch == TIM_CHANNEL_3) ? TIM_IT_CC3 : TIM_IT_CC4;

    __HAL_TIM_DISABLE_IT(htim, it_bit);
    __HAL_TIM_CLEAR_IT(htim, it_bit);

    // Optional but nice: clear any pending NVIC for this timer
    // NVIC_ClearPendingIRQ(TIM2_IRQn); // use correct IRQn for your timer

    // 2) Program the first compare **relative to now** so it's in the future
    uint32_t first = __HAL_TIM_GET_COUNTER(htim) + g_timer_drv.sub_timers_periods[p_timer_id];
    __HAL_TIM_SET_COMPARE(htim, ch, first);

    // // Edge case: if very tight, push it further to avoid immediate match
    // // (use signed diff to handle wrap)
    int32_t delta = (int32_t)(__HAL_TIM_GET_COMPARE(htim, ch) - __HAL_TIM_GET_COUNTER(htim));
    if (delta <= 2) {  // a couple of ticks margin
        __HAL_TIM_SET_COMPARE(htim, ch, __HAL_TIM_GET_COMPARE(htim, ch) + 10);
    }

    // 3) Enable channel output-compare (no output) and then its interrupt
    hal_ret = HAL_TIM_OC_Start(htim, ch);         // enables CCxE; does NOT set CCxIE
    __HAL_TIM_ENABLE_IT(htim, it_bit);  // enable IRQ **after** CCR is set

    return hal_ret;
}

static response_status_t start(mp_timer_id_t p_timer_id)
{
    ASSERT_AND_RETURN(g_timer_drv.hw_inst == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer_id >= MP_TIMER_CNT, RET_PARAM_ERROR);

    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (p_timer_id)
    {
        case MP_TIMER_10US_ID:
            hal_ret = OC_StartPeriodic(p_timer_id, TIM_CHANNEL_1);
            break;
        case MP_TIMER_1MS_ID:
            hal_ret = OC_StartPeriodic(p_timer_id, TIM_CHANNEL_2);
            break;
        case MP_TIMER_10MS_ID:
            hal_ret = OC_StartPeriodic(p_timer_id, TIM_CHANNEL_3);
            break;
        case MP_TIMER_100MS_ID:
            hal_ret = OC_StartPeriodic(p_timer_id, TIM_CHANNEL_4);
            break;
        default:
            return RET_PARAM_ERROR;
    }

    return translate_hal_status(hal_ret);
}

static response_status_t stop(mp_timer_id_t p_timer_id)
{
    ASSERT_AND_RETURN(g_timer_drv.hw_inst == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer_id >= MP_TIMER_CNT, RET_PARAM_ERROR);

    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (p_timer_id)
    {
        case MP_TIMER_10US_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_timer_drv.hw_inst, TIM_CHANNEL_1);
            // __HAL_TIM_CLEAR_IT(g_timer_drv.hw_inst, TIM_IT_CC1);
            break;
            case MP_TIMER_1MS_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_timer_drv.hw_inst, TIM_CHANNEL_2);
            // __HAL_TIM_CLEAR_IT(g_timer_drv.hw_inst, TIM_IT_CC2);
            break;
            case MP_TIMER_10MS_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_timer_drv.hw_inst, TIM_CHANNEL_3);
            // __HAL_TIM_CLEAR_IT(g_timer_drv.hw_inst, TIM_IT_CC3);
            break;
            case MP_TIMER_100MS_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_timer_drv.hw_inst, TIM_CHANNEL_4);
            // __HAL_TIM_CLEAR_IT(g_timer_drv.hw_inst, TIM_IT_CC4);

        case MP_TIMER_CNT:
        default:
            return RET_PARAM_ERROR;
    }

    return translate_hal_status(hal_ret);
}

static response_status_t register_callback(mp_timer_id_t p_timer_id, void (*callback)(void))
{
    ASSERT_AND_RETURN(g_timer_drv.hw_inst == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(callback == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timer_id >= MP_TIMER_CNT, RET_NOT_SUPPORTED);

    CRITICAL_ENTER();
    // If it's the first time we register a callback for this timer, we need to register the HAL
    // callback
    if (g_timer_drv.timers_user_cb[p_timer_id] == NULL)
    {
        HAL_TIM_RegisterCallback(g_timer_drv.hw_inst,
                                 HAL_TIM_OC_DELAY_ELAPSED_CB_ID,
                                 hal_channel_tim_cb);
    }
    g_timer_drv.timers_user_cb[p_timer_id] = callback;
    CRITICAL_EXIT();
    return RET_OK;
}

static response_status_t get_state(void)
{

    return RET_NOT_SUPPORTED;

    ASSERT_AND_RETURN(g_timer_drv.hw_inst == NULL, RET_NOT_INITIALIZED);

    response_status_t    ret_val   = RET_OK;
    HAL_TIM_StateTypeDef tim_state = HAL_TIM_Base_GetState(g_timer_drv.hw_inst);

    switch (tim_state)
    {
        case HAL_TIM_STATE_RESET:
            ret_val = RET_NOT_INITIALIZED;
            break;
        case HAL_TIM_STATE_READY:
            ret_val = RET_OK;
            break;
        case HAL_TIM_STATE_BUSY:
            ret_val = RET_BUSY;
            break;
        case HAL_TIM_STATE_TIMEOUT:
            ret_val = RET_TIMEOUT;
            break;
        case HAL_TIM_STATE_ERROR:
            ret_val = RET_ERROR;
            break;
    }

    return ret_val;
}

static uint32_t get_cpu_time(mp_timer_unit_t p_time_unit)
{
    switch (p_time_unit)
    {
        case MP_TIMER_UNIT_MS:
            return HAL_GetTick();
        case MP_TIMER_UNIT_US:
            return (uint32_t)(DWT->CYCCNT / 80u);
        default:
            return 0;
    }
}

static void hard_delay(uint32_t p_delay, mp_timer_unit_t p_delay_unit)
{
    switch (p_delay_unit)
    {
        case MP_TIMER_UNIT_MS:
            HAL_Delay(p_delay);
            break;
        case MP_TIMER_UNIT_US:
            uint32_t t0 =get_cpu_time(MP_TIMER_UNIT_US);
            while ((uint32_t)(get_cpu_time(MP_TIMER_UNIT_US) - t0) < p_delay) { __NOP(); }
            break;
        default:
            break;
    }
}

static struct st_timer_driver_ifc g_interface = { .init              = init,
                                                  .start             = start,
                                                  .stop              = stop,
                                                  .register_callback = register_callback,
                                                  .get_state         = get_state,
                                                  .get_frequency     = NULL,
                                                  .hard_delay        = hard_delay,
                                                  .get_cpu_time      = get_cpu_time };

timer_driver_t* timer_driver_register(void)
{
    g_timer_drv.base.api = &g_interface;
    return (timer_driver_t*)&g_timer_drv;
}
