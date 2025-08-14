#include "mp_timer_capture.h"

#include "mp_common.h"
#include "main.h"
#include "string.h"
#include "su_common.h"

#define HAL_SUPPORTED_TIMERS_CNT 8

union capture_data
{
    struct
    {
        uint32_t last_ccr;
        uint32_t diff_ticks;
    } edge_data;

    struct
    {
        uint32_t falling_edge;
        uint32_t rising_edge;
        uint32_t waiting_edge;
        uint32_t pulse_width;
    } pulse_width_data;
};

typedef struct st_stm32_ic_driver
{
    timer_capture_driver_t      base;
    struct hal_capture_tim_ifc* hal_drv;
    volatile union capture_data cap_data[MP_TIMER_CAP_CHNL_CNT];
    timer_capture_callback_t    capture_cb[MP_TIMER_CAP_CHNL_CNT];
    mp_timer_capture_type_t     req_cap_type[MP_TIMER_CAP_CHNL_CNT];
    volatile bool_t             one_shot[MP_TIMER_CAP_CHNL_CNT];
    volatile bool_t             on_going[MP_TIMER_CAP_CHNL_CNT];
} stm32_ic_driver_t;

static stm32_ic_driver_t g_ic_drv = { .base = { 0U }, .hal_drv = NULL };

static uint32_t hal_timers_slot[HAL_SUPPORTED_TIMERS_CNT] = { -1 };

static inline void hash_hal_timer(TIM_HandleTypeDef* hal_tim,
                                  uint16_t           hal_chnl_shift, // 0,4,8,12 (nibble shift)
                                  uint16_t           slot) // dense slot start for this TIMx
{
    uint32_t addr   = (uint32_t)hal_tim->Instance;
    uint32_t block  = (((addr >> 16) & 0xF) == 1u); // 0: APB1, 1: APB2
    uint32_t offset = (addr & 0xFFFFu) >> 10;       // /0x400 -> 0..3
    uint32_t index  = ((block << 2) + offset);      // 0..7

    uint32_t packed = hal_timers_slot[index];

    if (packed == 0xFFFFFFFFu)
    {
        // first channel for this timer: set base slot and write nibble=1 at this channel
        uint32_t encoded_tim   = (uint32_t)slot; // base index in upper 16
        uint32_t encoded_chnl  = (1u & 0xFu) << hal_chnl_shift;
        hal_timers_slot[index] = (encoded_tim << 16) | encoded_chnl;
    }
    else
    {
        // find max nibble among 4 channels to get next position
        uint32_t max_pos = 0;
        for (uint8_t s = 0; s <= 12; s += 4)
        {
            uint32_t v = (packed >> s) & 0xFu;
            if (v > max_pos)
            {
                max_pos = v;
            }
        }
        uint32_t pos_plus1 = ((max_pos + 1u) & 0xFu);
        uint32_t nib_mask  = (0xFu << hal_chnl_shift);
        // keep upper 16 bits (base slot), update only this channel's nibble
        packed                 = (packed & ~nib_mask) | (pos_plus1 << hal_chnl_shift);
        hal_timers_slot[index] = packed;
    }
}

// Return: dense slot index (0..N-1) or -1 if not found / not active
static inline int32_t unhash_hal_slot(TIM_HandleTypeDef* htim)
{
    uint32_t addr   = (uint32_t)htim->Instance;
    uint32_t block  = (((addr >> 16) & 0xF) == 1u);
    uint32_t offset = (addr & 0xFFFFu) >> 10;
    uint32_t tindex = ((block << 2) + offset);

    uint32_t packed = hal_timers_slot[tindex];
    if (packed == 0xFFFFFFFFu)
    {
        return -1;
    }

    uint32_t active = (uint32_t)htim->Channel; // 1..4
    if ((active & 0xF) == 0)
    {
        return -1; // defensive
    }
    uint32_t ch_idx    = __builtin_ctz(active); // 0..3
    uint32_t nib_shift = ch_idx * 4;

    uint32_t base_idx  = packed >> 16;
    uint32_t pos_plus1 = (packed >> nib_shift) & 0xFu;
    if (pos_plus1 == 0)
    {
        return -1;
    }

    return (int32_t)(base_idx + (pos_plus1 - 1u));
}

static void cap_cb(TIM_HandleTypeDef* htim)
{
    volatile uint32_t drv_tim_idx = unhash_hal_slot(htim);
    volatile uint32_t channel     = __builtin_ctz(htim->Channel) << 2;

    if (drv_tim_idx > g_ic_drv.base.hw_inst_cnt)
    {
        return;
    }

    g_ic_drv.on_going[drv_tim_idx] = TRUE;

    volatile union capture_data* cap_data = &g_ic_drv.cap_data[drv_tim_idx];
    volatile uint32_t            ccap     = __HAL_TIM_GET_COMPARE(htim, channel);

    switch (g_ic_drv.req_cap_type[drv_tim_idx])
    {
        case MP_TIMER_CAP_TYPE_RISING_EDGE:
        case MP_TIMER_CAP_TYPE_FALLING_EDGE:
            cap_data->edge_data.diff_ticks =
              (ccap >= cap_data->edge_data.last_ccr)
                ? (ccap - cap_data->edge_data.last_ccr)
                : ((htim->Instance->ARR - cap_data->edge_data.last_ccr) + ccap + 1u);

            cap_data->edge_data.last_ccr = ccap;

            if (g_ic_drv.capture_cb[drv_tim_idx] != NULL)
            {
                g_ic_drv.capture_cb[drv_tim_idx](drv_tim_idx, cap_data->edge_data.diff_ticks);
            }
            if (g_ic_drv.one_shot[drv_tim_idx])
            {
                __HAL_TIM_SET_COMPARE(htim, channel, 0);
                HAL_TIM_IC_Stop_IT(htim, channel);
            }
            break;

        case MP_TIMER_CAP_TYPE_PULSE_WIDTH:

            if (cap_data->pulse_width_data.waiting_edge == 0)
            {
                // Rising edge detected
                cap_data->pulse_width_data.rising_edge = HAL_TIM_ReadCapturedValue(htim, channel);

                // Switch to falling edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, channel, TIM_INPUTCHANNELPOLARITY_FALLING);
                cap_data->pulse_width_data.waiting_edge = 1;
            }
            else
            {
                // Falling edge detected
                cap_data->pulse_width_data.falling_edge = HAL_TIM_ReadCapturedValue(htim, channel);

                // Calculate pulse width (in µs)
                if (cap_data->pulse_width_data.falling_edge
                    >= cap_data->pulse_width_data.rising_edge)
                {
                    cap_data->pulse_width_data.pulse_width =
                      cap_data->pulse_width_data.falling_edge
                      - cap_data->pulse_width_data.rising_edge;
                }
                else
                {
                    cap_data->pulse_width_data.pulse_width =
                      (htim->Instance->ARR - cap_data->pulse_width_data.rising_edge + 1)
                      + cap_data->pulse_width_data.falling_edge;
                }

                // Switch back to rising edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, channel, TIM_INPUTCHANNELPOLARITY_RISING);
                cap_data->pulse_width_data.waiting_edge = 0;

                if (g_ic_drv.capture_cb[drv_tim_idx] != NULL)
                {
                    g_ic_drv.capture_cb[drv_tim_idx](drv_tim_idx,
                                                     cap_data->pulse_width_data.pulse_width);
                }
                if (g_ic_drv.one_shot[drv_tim_idx])
                {
                    __HAL_TIM_SET_COMPARE(htim, channel, 0);
                    HAL_TIM_IC_Stop_IT(htim, channel);
                }
            }
            break;
        case MP_TIMER_CAP_TYPE_FREQUENCY:
            return;
        default:
            return;
    }

    g_ic_drv.on_going[drv_tim_idx] = FALSE;
}

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;

    g_ic_drv.base.hw_inst_cnt = get_ic_tim_ifcs(&(g_ic_drv.hal_drv));

    ASSERT_AND_RETURN(g_ic_drv.base.hw_inst_cnt != ARRAY_SIZE(g_ic_drv.capture_cb), RET_ERROR);

    if (g_ic_drv.base.hw_inst_cnt == 0)
    {
        ret_val = RET_NOT_SUPPORTED;
    }
    else
    {
        for (uint8_t i = 0; i < g_ic_drv.base.hw_inst_cnt; ++i)
        {
            if (g_ic_drv.hal_drv[i].base_timer == NULL)
            {
                ret_val = RET_ERROR;
                break;
            }
            else
            {
                hash_hal_timer(g_ic_drv.hal_drv[i].base_timer,
                               g_ic_drv.hal_drv[i].engaged_channels,
                               i);
                HAL_TIM_RegisterCallback(g_ic_drv.hal_drv[i].base_timer,
                                         HAL_TIM_IC_CAPTURE_CB_ID,
                                         cap_cb);
            }
        }

        ret_val = RET_OK;
    }

    return ret_val;
}

static void obtain_timer_from_channel(mp_timer_capture_channels_t p_channel,
                                      TIM_HandleTypeDef** base_timer, uint32_t* channel_to_activate)
{
    ASSERT_AND_RETURN(g_ic_drv.hal_drv == NULL, );

    uint32_t total = 0;
    for (uint8_t i = 0; i < g_ic_drv.base.hw_inst_cnt; i++)
    {
        uint8_t engaged_channels_cnt = __builtin_popcount(g_ic_drv.hal_drv[i].engaged_channels);
        if (p_channel < total + engaged_channels_cnt)
        {
            *base_timer          = g_ic_drv.hal_drv[i].base_timer;
            *channel_to_activate = p_channel - total;
            break;
        }
        else
        {
            total += engaged_channels_cnt;
        }
    }
}

static response_status_t req_edge_capture(mp_timer_capture_channels_t p_channel,
                                          mp_timer_capture_type_t     rising_falling_edge,
                                          bool_t                      one_shot)
{
    ASSERT_AND_RETURN(g_ic_drv.hal_drv == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_channel >= MP_TIMER_CAP_CHNL_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(rising_falling_edge > MP_TIMER_CAP_TYPE_FALLING_EDGE, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(g_ic_drv.on_going[p_channel], RET_BUSY);

    HAL_StatusTypeDef  hal_ret    = HAL_OK;
    TIM_HandleTypeDef* base_timer = NULL;
    uint32_t           hal_chnl   = 0;

    g_ic_drv.req_cap_type[p_channel] = rising_falling_edge;
    memset((void*)&g_ic_drv.cap_data[p_channel], 0, sizeof(union capture_data));
    g_ic_drv.one_shot[p_channel] = one_shot;
    __HAL_TIM_SET_COMPARE(g_ic_drv.hal_drv[p_channel].base_timer,
                          g_ic_drv.hal_drv[p_channel].engaged_channels,
                          0);

    __HAL_TIM_SET_CAPTUREPOLARITY(g_ic_drv.hal_drv[p_channel].base_timer,
                                  g_ic_drv.hal_drv[p_channel].engaged_channels,
                                  TIM_INPUTCHANNELPOLARITY_RISING);
    hal_ret = HAL_TIM_IC_Start_IT(g_ic_drv.hal_drv[p_channel].base_timer,
                                  g_ic_drv.hal_drv[p_channel].engaged_channels);

    return translate_hal_status(hal_ret);
}

static response_status_t req_pulse_capture(mp_timer_capture_channels_t p_channel, bool_t one_shot)
{
    ASSERT_AND_RETURN(g_ic_drv.hal_drv == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_channel >= MP_TIMER_CAP_CHNL_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(g_ic_drv.on_going[p_channel], RET_BUSY);
    HAL_StatusTypeDef  hal_ret    = HAL_OK;
    TIM_HandleTypeDef* base_timer = NULL;
    uint32_t           hal_chnl   = 0;

    g_ic_drv.req_cap_type[p_channel] = MP_TIMER_CAP_TYPE_PULSE_WIDTH;
    memset((void*)&g_ic_drv.cap_data[p_channel], 0, sizeof(union capture_data));
    g_ic_drv.one_shot[p_channel] = one_shot;
    __HAL_TIM_SET_COMPARE(g_ic_drv.hal_drv[p_channel].base_timer,
                          g_ic_drv.hal_drv[p_channel].engaged_channels,
                          0);

    __HAL_TIM_SET_CAPTUREPOLARITY(g_ic_drv.hal_drv[p_channel].base_timer,
                                  g_ic_drv.hal_drv[p_channel].engaged_channels,
                                  TIM_INPUTCHANNELPOLARITY_RISING);

    hal_ret = HAL_TIM_IC_Start_IT(g_ic_drv.hal_drv[p_channel].base_timer,
                                  g_ic_drv.hal_drv[p_channel].engaged_channels);

    return translate_hal_status(hal_ret);
}

static response_status_t register_callback(mp_timer_capture_channels_t p_channel,
                                           timer_capture_callback_t    p_callback)
{
    ASSERT_AND_RETURN(g_ic_drv.hal_drv == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_callback == NULL, RET_PARAM_ERROR);

    response_status_t ret_val = RET_OK;

    CRITICAL_ENTER();
    g_ic_drv.capture_cb[p_channel] = p_callback;
    CRITICAL_EXIT();

    return ret_val;
}

static response_status_t get_data(mp_timer_capture_channels_t p_channel, uint32_t* p_data)
{
    ASSERT_AND_RETURN(g_ic_drv.hal_drv == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_channel >= MP_TIMER_CAP_CHNL_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(p_data == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(g_ic_drv.on_going[p_channel], RET_BUSY);

    response_status_t ret_val = RET_OK;

    switch (g_ic_drv.req_cap_type[p_channel])
    {
        case MP_TIMER_CAP_TYPE_RISING_EDGE:
        case MP_TIMER_CAP_TYPE_FALLING_EDGE:
            *p_data = g_ic_drv.cap_data[p_channel].edge_data.diff_ticks;
            ret_val = RET_OK;
            break;

        case MP_TIMER_CAP_TYPE_PULSE_WIDTH:
            *p_data = g_ic_drv.cap_data[p_channel].pulse_width_data.pulse_width;
            ret_val = RET_OK;
            break;

        case MP_TIMER_CAP_TYPE_FREQUENCY:
            // Not implemented in this driver
            ret_val = RET_NOT_SUPPORTED;
            break;

        default:
            ret_val = RET_PARAM_ERROR;
            break;
    }

    return ret_val;
}

static response_status_t stop_capture(mp_timer_capture_channels_t p_channel)
{
    ASSERT_AND_RETURN(g_ic_drv.hal_drv == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_channel >= MP_TIMER_CAP_CHNL_CNT, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(g_ic_drv.on_going[p_channel], RET_BUSY);

    HAL_StatusTypeDef  hal_ret    = HAL_OK;
    TIM_HandleTypeDef* base_timer = NULL;
    uint32_t           hal_chnl   = 0;

    obtain_timer_from_channel(p_channel, &base_timer, &hal_chnl);

    memset((void*)&g_ic_drv.cap_data[p_channel], 0, sizeof(union capture_data));
    __HAL_TIM_SET_COMPARE(g_ic_drv.hal_drv[p_channel].base_timer,
                          g_ic_drv.hal_drv[p_channel].engaged_channels,
                          0);

    hal_ret = HAL_TIM_IC_Stop_IT(base_timer, hal_chnl);

    g_ic_drv.on_going[p_channel] = FALSE;

    return translate_hal_status(hal_ret);
}

static struct st_ic_driver_ifc g_interface = {
    .init              = init,
    .capture_edge      = req_edge_capture,
    .capture_pulse     = req_pulse_capture,
    .capture_frequency = NULL,
    .register_callback = register_callback,
    .get_data          = get_data,    
    .stop_capture      = stop_capture,
};

timer_capture_driver_t* timer_capture_driver_register(void)
{

    memset(&g_ic_drv, 0, sizeof(g_ic_drv));
    memset(hal_timers_slot, (uint32_t)-1, sizeof(hal_timers_slot));

    g_ic_drv.base.api = &g_interface;
    return (timer_capture_driver_t*)&g_ic_drv;
}
