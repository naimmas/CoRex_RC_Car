#include "baro.h"
#include "dd_esp32/dd_esp32.h"
#include "dd_fsi6/dd_fsi6.h"
#include "dd_status_led/dd_status_led.h"
#include "imu.h"
#include "ps_app_timer/ps_app_timer.h"
#include "ps_iic_bus_scanner/ps_iic_bus_scanner.h"
#include "ps_logger/ps_logger.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define CHECK_APP_ERR(ret_val) \
    if ((ret_val) != RET_OK) { \
        app_err_handler(); \
    }
#define CHECK_APP_ERR_LOG(ret_val, msg) \
    if ((ret_val) != RET_OK) { \
        LOG_ERR(msg); \
        app_err_handler(); \
    }

app_timer_handler_t* g_pt_g_esp32_msg_timer = NULL;

int32_t map(int32_t p_au32_in, int32_t p_au32_i_nmin, int32_t p_au32_i_nmax, int32_t p_au32_ou_tmin,
            int32_t p_au32_ou_tmax)
{
    return ((((p_au32_in - p_au32_i_nmin) * (p_au32_ou_tmax - p_au32_ou_tmin)) / (p_au32_i_nmax - p_au32_i_nmin))
            + p_au32_ou_tmin);
}

void app_err_handler(void)
{
    dd_status_led_error();
    while (1) {
        ;
}
}

int app(void)
{
    response_status_t ret_val = RET_OK;

    ret_val = ps_logger_init();
    CHECK_APP_ERR(ret_val);

    ps_bus_scanner_init();
    ps_scan_iic_bus();
    ps_app_timer_init();

    ret_val = ps_app_timer_create(&g_pt_g_esp32_msg_timer, TRUE, NULL);
    CHECK_APP_ERR_LOG(ret_val, "Error creating ESP32 message timer\n");

    ret_val = dd_status_led_init();
    CHECK_APP_ERR_LOG(ret_val, "Error initializing Status LED\n");

    ret_val = dd_esp32_init();
    CHECK_APP_ERR_LOG(ret_val, "Error initializing ESP32\n");

    ret_val = dd_fsi6_init(TRUE);
    CHECK_APP_ERR_LOG(ret_val, "Error initializing FSI6\n");

    ret_val = imu_init();
    CHECK_APP_ERR_LOG(ret_val, "Error initializing IMU\n");

    ret_val = baro_init();
    CHECK_APP_ERR_LOG(ret_val, "Error initializing Baro\n");

    dd_esp32_data_packet_t data_msg = { 0 };
    bool_t                 send_msg = FALSE;

    ps_app_timer_start(g_pt_g_esp32_msg_timer, 50, APP_TIMER_UNIT_MS);
    dd_status_led_normal();
    while (1)
    {
        ret_val  = imu_get_data(&data_msg.acc[0].f,
                               &data_msg.gyro[0].f,
                               &data_msg.mag[0].f,
                               &data_msg.quat[0].f);
        ret_val |= baro_get_data(&data_msg.baro.f);

        dd_fsi6_get_data(FSI6_IN_L_S_UD, &data_msg.throttle_stick);
        dd_fsi6_get_data(FSI6_IN_R_S_LR, &data_msg.steering_stick);

        if (ret_val == RET_OK && g_pt_g_esp32_msg_timer->is_fired == TRUE)
        {
            g_pt_g_esp32_msg_timer->is_fired = FALSE;
            LOG_INFO("DATA OK\n");
            ret_val = dd_esp32_send_data_packet(&data_msg);
            if (ret_val != RET_OK)
            {
                LOG_ERR("Error sending data packet to ESP32\n");
            }
            ps_app_timer_start(g_pt_g_esp32_msg_timer,
                               50,
                               APP_TIMER_UNIT_MS); // Restart timer for next message
        }
    }

    return 0;
}
