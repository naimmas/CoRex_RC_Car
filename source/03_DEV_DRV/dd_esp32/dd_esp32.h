
#ifndef DD_ESP32_H
#define DD_ESP32_H

#include "su_common.h"

union float_to_bytes
{
    float   f;
    uint8_t bytes[sizeof(float)];
};

typedef struct
{
    union float_to_bytes acc[3];
    union float_to_bytes gyro[3];
    union float_to_bytes mag[3];
    union float_to_bytes quat[4];
    union float_to_bytes baro;
    uint32_t             throttle_stick;
    uint32_t             steering_stick;
} dd_esp32_data_packet_t;

response_status_t dd_esp32_init(void);
response_status_t dd_esp32_send_data_packet(dd_esp32_data_packet_t* ppt_data_packet);

#endif // DD_ESP32_H
