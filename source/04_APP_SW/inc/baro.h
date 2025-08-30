#ifndef BARO_H
#define BARO_H

#include "dd_bmp388/dd_bmp388.h"
#include "su_common.h"

response_status_t baro_get_data(float* ppt_pres_hndlr);
response_status_t baro_init(void);

#endif // BARO_H
