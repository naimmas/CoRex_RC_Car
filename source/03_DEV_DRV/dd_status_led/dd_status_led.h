
#ifndef DD_STATUS_LED_H
#define DD_STATUS_LED_H

#include "su_common.h"

response_status_t dd_status_led_init(void);
void              dd_status_led_normal(void);
void              dd_status_led_error(void);

#endif // DD_STATUS_LED_H
