#ifndef HA_GPIO_PRIVATE_H
#define HA_GPIO_PRIVATE_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_gpio_driver_ifc* gpio_driver_ifc;

typedef struct st_gpio_driver
{
    gpio_driver_ifc api;
    uint16_t        hw_pin_cnt;
} gpio_driver_t;

struct st_gpio_driver_ifc
{
    /**
     * @brief This function initializes the interface. it shall get all hardware
     * pins information from the MCU SDK
     * @retval `RET_OK` if the interface is initialized successfully.
     * @retval `RET_ERROR` if there is an error during initialization.
     * @retval `RET_NOT_SUPPORTED` if the hardware does not support GPIO.
     */
    response_status_t (*init)(void);
    /**
     * @brief This function shall set the pin value to logic high or low.
     * @param[in] uint8_t pin number to write to.
     * @param[in] bool_t value to write to the pin, TRUE for logic high, FALSE
     * for logic low.
     * @retval `RET_OK` if the pin value is set successfully.
     * @retval `RET_PARAM_ERROR` if there is an invalid parameter, such as pin
     * number out of range.
     * @retval `RET_NOT_INITIALIZED` if the interface is not initialized.
     */
    response_status_t (*write)(uint8_t, bool_t);
    /**
     * @brief This function shall read the pin value.
     * @param[in] uint8_t pin number to read from.
     * @param[out] bool_t* pointer to store the read value, if the pin is logic
     * high, it will be set to TRUE, otherwise FALSE.
     * @retval `RET_OK` if the pin value is set successfully.
     * @retval `RET_PARAM_ERROR` if there is an invalid parameter, such as pin
     * number out of range.
     * @retval `RET_NOT_INITIALIZED` if the interface is not initialized.
     */
    response_status_t (*read)(uint8_t, bool_t*);
    /**
     * @brief This function shall toggle the pin value.
     * @param[in] uint8_t pin number to toggle.
     * @retval `RET_OK` if the pin value is toggled successfully.
     * @retval `RET_PARAM_ERROR` if there is an invalid parameter, such as pin
     * number out of range.
     * @retval `RET_NOT_INITIALIZED` if the interface is not initialized.
     */
    response_status_t (*toggle)(uint8_t);
};

#endif /* HA_GPIO_PRIVATE_H */
