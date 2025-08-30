#ifndef HA_TIMER_PRIVATE_H
#define HA_TIMER_PRIVATE_H

#include "ha_timer.h"
#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_timer_driver_ifc* timer_driver_ifc;
typedef gp_timers_t mp_timer_id_t;

typedef enum
{
    MP_TIMER_UNIT_US = 0,
    MP_TIMER_UNIT_MS,
} mp_timer_unit_t;

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
     * @param[in] p_timer_id The ID of the timer to start
     *
     * @retval `RET_OK` if the timer is started successfully, else error code.
     */
    response_status_t (*start)(mp_timer_id_t);

    /**
     * @brief This function shall stop the hardware timer.
     *
     * @param[in] p_timer_id The ID of the timer to stop
     *
     * @retval `RET_OK` if the timer is stopped successfully, else error code.
     */
    response_status_t (*stop)(mp_timer_id_t);

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
     * @param[in] p_timer_id The ID of the timer to register the callback for
     * @param[in] callback The callback function to register.
     *
     * @retval `RET_OK` if the callback is registered successfully, else error code.
     */
    response_status_t (*register_callback)(mp_timer_id_t, void (*ppt_callback)(void));

    /**
     * @brief This function shall provide a hard delay in milliseconds or microseconds.
     *
     * @param[in] p_delay The delay duration.
     * @param[in] p_delay_unit The unit of the delay, @ref mp_timer_unit_t.
     *
     */
    void (*hard_delay)(uint32_t, mp_timer_unit_t);

    /**
     * @brief This function shall get the current CPU time in milliseconds or microseconds.
     *
     * @param[in] p_time_unit The unit of the time, @ref mp_timer_unit_t.
     *
     * @retval The current CPU time in the specified unit.
     */
    uint32_t (*get_cpu_time)(mp_timer_unit_t);
};

#endif /* HA_TIMER_PRIVATE_H */
