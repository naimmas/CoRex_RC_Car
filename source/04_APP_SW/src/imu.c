#include "imu.h"

#include "dd_icm209/dd_icm209.h"

static const float g_acc_a[3][3] = {
    {  1.190553391091500F,  0.017123734237795F,  0.007837760042511F },
    {  0.001996992431559F,  1.196668563340221F, -0.000775887418440F },
    { -0.064525734014990F, -0.017518888406769F,  1.193724153187501F }
};

static const float g_acc_b[3] = { 0.063981206956114F, 0.106560263265376F, -0.338901066521774F };

static const float g_mag_a[3][3] = {
    {  1.000941865753861F, -0.006342268652996F,  0.090698015712243F },
    { -0.006342268652996F,  1.000548758124559F, -0.004594496153562F },
    {  0.090698015712243F, -0.004594496153562F,  1.006785725808204F }
};

static const float g_mag_b[3] = { -188.8444335021131F, 78.349830134154089F, 40.446694449254487F };

static const int g_mounting_matrix[3][3] = {
    {  0, -1, 0 }, // X row 
    {  -1, 0, 0 }, // Y row
    {  0, 0, 1 }  // Z row
};


void imu_apply_mounting_matrix(float* ppt_acc, float* ppt_gyro, float* ppt_mag)
{
    float acc_temp[3] = { 0 };
    float gyro_temp[3] = { 0 };
    float mag_temp[3] = { 0 };

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            acc_temp[i] += (float)(g_mounting_matrix[i][j]) * ppt_acc[j];
            gyro_temp[i] += (float)(g_mounting_matrix[i][j]) * ppt_gyro[j];
            mag_temp[i] += (float)(g_mounting_matrix[i][j]) * ppt_mag[j];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        ppt_acc[i] = acc_temp[i];
        ppt_gyro[i] = gyro_temp[i];
        ppt_mag[i] = mag_temp[i];
    }
}

TeensyICM20948Settings g_icm_settings = {
    .mode                    = 1,    // 0 = low power mode, 1 = high performance mode
    .enable_gyroscope        = TRUE, // Enables gyroscope output
    .enable_accelerometer    = TRUE, // Enables accelerometer output
    .enable_magnetometer     = TRUE, // Enables magnetometer output
    .enable_quaternion       = TRUE, // Enables quaternion output
    .gyroscope_frequency     = 50,   // Max frequency = 225, min frequency = 1
    .accelerometer_frequency = 50,   // Max frequency = 225, min frequency = 1
    .magnetometer_frequency  = 50,   // Max frequency = 70, min frequency = 1
    .quaternion_frequency    = 50    // Max frequency = 225, min frequency = 50
};

response_status_t imu_init()
{
    return dd_icm209_init(g_icm_settings);
}
response_status_t imu_get_data(float* ppt_acc, float* ppt_gyro, float* ppt_mag, float* ppt_quat)
{
    response_status_t ret_val = RET_ERROR;

    dd_icm209_task();
    if (dd_icm209_gyro_data_is_ready())
    {
        dd_icm209_read_gyro_data(&ppt_gyro[0], &ppt_gyro[1], &ppt_gyro[2]);
        // gyro[0] /= 1000.f;
        // gyro[1] /= 1000.f;
        // gyro[2] /= 1000.f;
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }

    if (ret_val == RET_OK && dd_icm209_accel_data_is_ready())
    {
        dd_icm209_read_accel_data(&ppt_acc[0], &ppt_acc[1], &ppt_acc[2]);
        ppt_acc[0] *= 9.806F;
        ppt_acc[1] *= 9.806F;
        ppt_acc[2] *= 9.806F;
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }

    if (ret_val == RET_OK && dd_icm209_mag_data_is_ready())
    {
        dd_icm209_read_mag_data(&ppt_mag[0], &ppt_mag[1], &ppt_mag[2]);
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }
    if (ret_val == RET_OK)
    {
        imu_apply_mounting_matrix(ppt_acc, ppt_gyro, ppt_mag);
    }
    if (ret_val == RET_OK && dd_icm209_quat_data_is_ready())
    {
        dd_icm209_read_quat_data(&ppt_quat[0], &ppt_quat[1], &ppt_quat[2], &ppt_quat[3]);
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }
    return ret_val;
}
