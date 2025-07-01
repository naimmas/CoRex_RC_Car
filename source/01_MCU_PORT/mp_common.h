#ifndef MP_COMMON_H
#define MP_COMMON_H

#include "stm32f4xx_hal.h" //NOLINT(misc-header-include-cycle,misc-include-cleaner)
#include "stm32f4xx_hal_def.h" //NOLINT(misc-header-include-cycle,misc-include-cleaner)
#include "su_common.h"

static inline response_status_t translate_hal_status(
  HAL_StatusTypeDef p_hal_ret)
{
    response_status_t ret_val = RET_OK;

    switch (p_hal_ret)
    {
        case HAL_OK:
            ret_val = RET_OK;
            break;
        case HAL_TIMEOUT:
            ret_val = RET_TIMEOUT;
            break;
        case HAL_ERROR:
            ret_val = RET_ERROR;
            break;
        case HAL_BUSY:
            ret_val = RET_BUSY;
            break;
        default:
            ret_val = RET_ERROR;
            break;
    }

    return ret_val;
}
#endif /* MP_COMMON_H */
