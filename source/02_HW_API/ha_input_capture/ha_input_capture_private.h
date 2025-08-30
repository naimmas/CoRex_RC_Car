
#ifndef HA_INPUT_CAPTURE_PRIVATE_H
#define HA_INPUT_CAPTURE_PRIVATE_H

#include "ha_input_capture.h"
#include "su_common.h"

typedef struct st_ic_driver_ifc* ic_driver_ifc;
typedef input_capture_channel_t mp_timer_capture_channels_t;
typedef input_capture_type_t mp_timer_capture_type_t;
typedef input_capture_mode_t mp_timer_capture_mode_t;

typedef ic_finished_callback_t timer_capture_callback_t;

typedef struct st_ic_driver
{
    ic_driver_ifc api;
    uint8_t       hw_inst_cnt;
} timer_capture_driver_t;

struct st_ic_driver_ifc
{
    /**
     * @brief This function shall initialize the input capture interface.
     * it shall get all hardware instances information from the MCU SDK.
     *
     * @retval `RET_OK` if the interface is initialized successfully, else error code.
     */
    response_status_t (*init)(void);

    response_status_t (*capture_pulse)(mp_timer_capture_channels_t, mp_timer_capture_mode_t);

    response_status_t (*capture_frequency)(mp_timer_capture_channels_t, mp_timer_capture_mode_t);

    response_status_t (*capture_edge)(mp_timer_capture_channels_t, mp_timer_capture_type_t, mp_timer_capture_mode_t);

    response_status_t (*register_callback)(mp_timer_capture_channels_t, timer_capture_callback_t);
    response_status_t (*get_data)(mp_timer_capture_channels_t, uint32_t*);
    response_status_t (*stop_capture)(mp_timer_capture_channels_t);
};

#endif // HA_INPUT_CAPTURE_PRIVATE_H
