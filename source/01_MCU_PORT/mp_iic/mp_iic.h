#ifndef MP_IIC_H
#define MP_IIC_H

#include "ha_iic/ha_iic_private.h"

iic_driver_t* iic_driver_register(void);

typedef enum en_iic_memadd_size
{
    IIC_MEMADD_SZ_8  = 0x1,
    IIC_MEMADD_SZ_16 = 0x10,
} iic_memadd_size_t;

#endif /* MP_IIC_H */
