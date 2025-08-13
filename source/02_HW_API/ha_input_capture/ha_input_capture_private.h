
#ifndef HA_INPUT_CAPTURE_PRIVATE_H
#define HA_INPUT_CAPTURE_PRIVATE_H

#include "su_common.h"

typedef struct st_ic_driver_ifc* ic_driver_ifc;

typedef void (*timer_capture_callback_t)(uint8_t, uint32_t);

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

    /**
     * @brief This function shall start the input capture hardware.
     *
     * @param[in] p_timer_id The ID of the timer to start @ref mp_timer_id_t.
     *
     * @retval `RET_OK` if the timer is started successfully, else error code.
     */
    response_status_t (*capture_pulse)(uint8_t, bool_t);

    response_status_t (*capture_frequency)(uint8_t, bool_t);

    response_status_t (*capture_edge)(uint8_t, uint8_t, bool_t);

    response_status_t (*register_callback)(uint8_t, timer_capture_callback_t);
    response_status_t (*get_data)(uint8_t, uint32_t*);
    response_status_t (*abort)(uint8_t);
};

#endif // HA_INPUT_CAPTURE_PRIVATE_H
