#ifndef HA_TIMER_PRIVATE_H
#define HA_TIMER_PRIVATE_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_timer_driver_ifc* timer_driver_ifc;

typedef struct st_timer_driver
{
    timer_driver_ifc api;
} timer_driver_t;

struct st_timer_driver_ifc
{
    /**
     * @brief This function shall initialize the timer interface.
     * it shall get all hardware instances information from the MCU SDK.
     *
     * @retval `RET_OK` if the interface is initialized successfully, else error code.
     */
    response_status_t (*init)(void);

    /**
     * @brief This function shall start the hardware timer.
     *
     * @param[in] p_timer_id The ID of the timer to start @ref mp_timer_id_t.
     *
     * @retval `RET_OK` if the timer is started successfully, else error code.
     */
    response_status_t (*start)(uint8_t);

    /**
     * @brief This function shall stop the hardware timer.
     *
     * @param[in] p_timer_id The ID of the timer to stop @ref mp_timer_id_t.
     *
     * @retval `RET_OK` if the timer is stopped successfully, else error code.
     */
    response_status_t (*stop)(uint8_t);

    /**
     * @brief This function shall get the current state of the timer.
     *
     * @retval `RET_OK` Timer Initialized and ready for use
     * @retval `RET_NOT_INITIALIZED` Timer not yet initialized
     * @retval `RET_BUSY` Timer is running
     * @retval `RET_TIMEOUT` Timeout state
     * @retval `RET_ERROR` Timer in error state
     */
    response_status_t (*get_state)(void);

    /**
     * @brief This function shall get the current frequency of the timer.
     *
     * @param[out] frequency Pointer to store the current frequency of the timer.
     *
     * @retval `RET_OK` if the frequency is retrieved successfully, else error code.
     */
    response_status_t (*get_frequency)(uint32_t*);

    /**
     * @brief This function shall register a callback for a specific timer event.
     *
     * @param[in] p_cb_id The ID of the callback to register @ref mp_timer_callback_id_t.
     * @param[in] callback The callback function to register.
     *
     * @retval `RET_OK` if the callback is registered successfully, else error code.
     */
    response_status_t (*register_callback)(uint8_t, void (*callback)(void));
};

#endif /* HA_TIMER_PRIVATE_H */
