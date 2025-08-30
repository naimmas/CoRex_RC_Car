#include "su_common.h"

typedef enum
{
    IMU_ACC,
    IMU_GYRO,
    IMU_MAG,
    IMU_QUAT
} imu_sens_type_t;

response_status_t imu_init();
response_status_t imu_get_data(float* ppt_acc, float* ppt_gyro, float* ppt_mag, float* ppt_quat);
