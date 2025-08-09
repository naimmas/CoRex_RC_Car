
#include "mp_timer_general.h"

#include "../mp_common.h"
#include "main.h"
#include "su_common.h"

typedef struct st_stm32_timer_driver
{
    timer_driver_t     base;
    TIM_HandleTypeDef* hw_inst;
    uint32_t           sub_timers_periods[MP_TIMER_CNT];
    void (*timers_user_cb[MP_TIMER_CNT])(void);
} stm32_timer_driver_t;

static stm32_timer_driver_t g_driver = { .base = { 0U }, .hw_inst = NULL };

// void hal_main_tim_cb(TIM_HandleTypeDef* htim)
// {
//     if (g_driver.timers_user_cb[MP_TIMER_10US_ID] != NULL)
//     {
//         g_driver.timers_user_cb[MP_TIMER_10US_ID]();
//     }
// }

void hal_channel_tim_cb(TIM_HandleTypeDef* htim)
{
    volatile uint32_t* const ccrs[4] = { &htim->Instance->CCR1,
                                         &htim->Instance->CCR2,
                                         &htim->Instance->CCR3,
                                         &htim->Instance->CCR4 };

    uint8_t idx = __builtin_ctz(htim->Channel);
    if (g_driver.timers_user_cb[idx] != NULL)
    {
        g_driver.timers_user_cb[idx]();
    }

    *(ccrs[idx]) += g_driver.sub_timers_periods[idx];
}

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;
    HAL_StatusTypeDef hal_ret = HAL_OK;

    get_base_tim_ifc(&(g_driver.hw_inst));

    if (g_driver.hw_inst == NULL)
    {
        ret_val = RET_ERROR;
    }

    if (ret_val == RET_OK)
    {
        g_driver.sub_timers_periods[0] = g_driver.hw_inst->Instance->CCR1;
        g_driver.sub_timers_periods[1] = g_driver.hw_inst->Instance->CCR2;
        g_driver.sub_timers_periods[2] = g_driver.hw_inst->Instance->CCR3;
        g_driver.sub_timers_periods[3] = g_driver.hw_inst->Instance->CCR4;
        ret_val = translate_hal_status(hal_ret);
    }

    return ret_val;
}

static response_status_t start(uint8_t p_timer_id)
{
    ASSERT_AND_RETURN(g_driver.hw_inst == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer_id >= MP_TIMER_CNT, RET_PARAM_ERROR);

    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (p_timer_id)
    {
        case MP_TIMER_10US_ID:
        hal_ret = HAL_TIM_OC_Start_IT(g_driver.hw_inst, TIM_CHANNEL_1);
            break;
        case MP_TIMER_1MS_ID:
        hal_ret = HAL_TIM_OC_Start_IT(g_driver.hw_inst, TIM_CHANNEL_2);
            break;
        case MP_TIMER_10MS_ID:
        hal_ret = HAL_TIM_OC_Start_IT(g_driver.hw_inst, TIM_CHANNEL_3);
        break;
        case MP_TIMER_100MS_ID:
        hal_ret = HAL_TIM_OC_Start_IT(g_driver.hw_inst, TIM_CHANNEL_4);
            break;
        default:
            return RET_PARAM_ERROR;
    }

    return translate_hal_status(hal_ret);
}

static response_status_t stop(uint8_t p_timer_id)
{
    ASSERT_AND_RETURN(g_driver.hw_inst == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_timer_id >= MP_TIMER_CNT, RET_PARAM_ERROR);

    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (p_timer_id)
    {
        case MP_TIMER_10US_ID:
        hal_ret = HAL_TIM_OC_Stop_IT(g_driver.hw_inst, TIM_CHANNEL_1);
            break;
        case MP_TIMER_1MS_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_driver.hw_inst, TIM_CHANNEL_2);
            break;
        case MP_TIMER_10MS_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_driver.hw_inst, TIM_CHANNEL_3);
            break;
            case MP_TIMER_100MS_ID:
            hal_ret = HAL_TIM_OC_Stop_IT(g_driver.hw_inst, TIM_CHANNEL_4);
            
            break;
            return RET_PARAM_ERROR;
    }

    return translate_hal_status(hal_ret);
}

static response_status_t register_callback(uint8_t p_timer_id, void (*callback)(void))
{
    ASSERT_AND_RETURN(g_driver.hw_inst == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(callback == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_timer_id >= MP_TIMER_CNT, RET_NOT_SUPPORTED);

    __disable_irq();
    // If it's the first time we register a callback for this timer, we need to register the HAL
    // callback
    if (g_driver.timers_user_cb[p_timer_id] == NULL)
    {
            HAL_TIM_RegisterCallback(g_driver.hw_inst,
                                     HAL_TIM_OC_DELAY_ELAPSED_CB_ID,
                                     hal_channel_tim_cb);
    }
    g_driver.timers_user_cb[p_timer_id] = callback;
    __enable_irq();
    return RET_OK;
}

static response_status_t get_state(void)
{

    return RET_NOT_SUPPORTED;

    ASSERT_AND_RETURN(g_driver.hw_inst == NULL, RET_NOT_INITIALIZED);

    response_status_t    ret_val   = RET_OK;
    HAL_TIM_StateTypeDef tim_state = HAL_TIM_Base_GetState(g_driver.hw_inst);

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

static struct st_timer_driver_ifc g_interface = {
    .init              = init,
    .start             = start,
    .stop              = stop,
    .register_callback = register_callback,
    .get_state         = get_state,
    .get_frequency     = NULL,
};

timer_driver_t* timer_driver_register(void)
{
    g_driver.base.api = &g_interface;
    return (timer_driver_t*)&g_driver;
}
