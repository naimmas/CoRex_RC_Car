
#ifndef DD_FSI6_H
#define DD_FSI6_H

#include "su_common.h"

typedef enum
{
    FSI6_IN_L_S_UD,
    FSI6_IN_R_S_LR,
    FSI6_IN_CNT
} fsi6_inputs_t;

response_status_t dd_fsi6_init(void);
response_status_t dd_fsi6_read_input(fsi6_inputs_t input, uint32_t* value);

#endif // DD_FSI6_H
