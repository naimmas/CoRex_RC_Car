#include "imu.h"

#include "dd_icm209/dd_icm209.h"

static const float g_acc_A[3][3] = {
    {  1.190553391091500f,  0.017123734237795f,  0.007837760042511f },
    {  0.001996992431559f,  1.196668563340221f, -0.000775887418440f },
    { -0.064525734014990f, -0.017518888406769f,  1.193724153187501f }
};

static const float g_acc_b[3] = { 0.063981206956114f, 0.106560263265376f, -0.338901066521774f };

static const float g_mag_A[3][3] = {
    {  1.000941865753861, -0.006342268652996,  0.090698015712243 },
    { -0.006342268652996,  1.000548758124559, -0.004594496153562 },
    {  0.090698015712243, -0.004594496153562,  1.006785725808204 }
};

static const float g_mag_b[3] = { -188.8444335021131f, 78.349830134154089f, 40.446694449254487f };

static const int g_mounting_matrix[3][3] = {
    {  0, -1, 0 }, // X row 
    {  -1, 0, 0 }, // Y row
    {  0, 0, 1 }  // Z row
};


void imu_apply_mounting_matrix(float* acc, float* gyro, float* mag)
{
    float acc_temp[3] = { 0 };
    float gyro_temp[3] = { 0 };
    float mag_temp[3] = { 0 };

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            acc_temp[i] += g_mounting_matrix[i][j] * acc[j];
            gyro_temp[i] += g_mounting_matrix[i][j] * gyro[j];
            mag_temp[i] += g_mounting_matrix[i][j] * mag[j];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        acc[i] = acc_temp[i];
        gyro[i] = gyro_temp[i];
        mag[i] = mag_temp[i];
    }
}

TeensyICM20948Settings icmSettings = {
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
    return dd_icm209_init(icmSettings);
}
response_status_t imu_get_data(float* acc, float* gyro, float* mag, float* quat)
{
    response_status_t ret_val = RET_ERROR;

    dd_icm209_task();
    if (dd_icm209_gyroDataIsReady())
    {
        dd_icm209_readGyroData(&gyro[0], &gyro[1], &gyro[2]);
        // gyro[0] /= 1000.f;
        // gyro[1] /= 1000.f;
        // gyro[2] /= 1000.f;
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }

    if (ret_val == RET_OK && dd_icm209_accelDataIsReady())
    {
        dd_icm209_readAccelData(&acc[0], &acc[1], &acc[2]);
        acc[0] *= 9.806;
        acc[1] *= 9.806;
        acc[2] *= 9.806;
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }

    if (ret_val == RET_OK && dd_icm209_magDataIsReady())
    {
        dd_icm209_readMagData(&mag[0], &mag[1], &mag[2]);
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }
    if (ret_val == RET_OK)
    {
        imu_apply_mounting_matrix(acc, gyro, mag);
    }
    if (ret_val == RET_OK && dd_icm209_quatDataIsReady())
    {
        dd_icm209_readQuatData(&quat[0], &quat[1], &quat[2], &quat[3]);
        ret_val = RET_OK;
    }
    else
    {
        ret_val = RET_BUSY;
    }
    return ret_val;
}
