
#ifndef PS_LOGGER_SERIAL_IFC_H
#define PS_LOGGER_SERIAL_IFC_H

#include "su_common.h"

void serial_ifc_send(const uint8_t *data, size_t len);
response_status_t serial_ifc_init(void);

#endif // PS_LOGGER_SERIAL_IFC_H
