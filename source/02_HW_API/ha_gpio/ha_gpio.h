
#ifndef HA_GPIO_H
#define HA_GPIO_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

typedef struct st_gpio_driver* gpio_driver;

typedef enum en_gpio_pins // NOLINT(readability-enum-initial-value)
{
    GP_PIN_LED = 0X00,
    /// Output pins count it is used to prevent using input pins as output
    GP_OUT_PIN_CNT,
    GP_PIN_IN_1 = GP_OUT_PIN_CNT,
    /// Total pins count
    GP_PIN_CNT
} gpio_pins_t;

typedef enum en_gpio_pin_state
{
    /// Logic low state
    GP_PIN_RESET = 0,
    /// Logic high state
    GP_PIN_SET = 1
} gpio_pin_state_t;

response_status_t ha_gpio_set(gpio_pins_t p_pin, gpio_pin_state_t p_value);
response_status_t ha_gpio_get(gpio_pins_t p_pin, gpio_pin_state_t* ppt_value);
response_status_t ha_gpio_toggle(gpio_pins_t p_pin);
response_status_t ha_gpio_init(void);

#endif // HA_GPIO_H
