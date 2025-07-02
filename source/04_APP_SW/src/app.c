#include "dd_bmp388/dd_bmp388.h"
#include "ps_iic_bus_scanner/ps_iic_bus_scanner.h"
#include "ps_logger/ps_logger.h"
#include "ps_timer/ps_timer.h"
#include "stdio.h"
#include "string.h"

int app(void)
{
    bmp388_dev_t*     pt_baro = NULL;
    response_status_t ret_val = RET_OK;
    bmp388_status_t   bmp_ret = BMP388_NO_ERROR;
    char              buffer[500U];

    ps_logger_init();
    ps_bus_scanner_init();
    ps_scan_iic_bus();
    ps_hard_delay_ms(100);

    ret_val = dd_bmp388_init(&pt_baro, BMP388_DEV_1);
    if (ret_val != RET_OK)
    {
        LOG_ERR("BMP388 initialization failed.\n");
        return -1;
    }

    LOG_INFO("BMP388 successfully initialized.\n");

    pt_baro->settings.data_settings.iir_filter        = BMP388_IIR_COEFF_15;
    pt_baro->settings.data_settings.output_data_rate  = BMP388_ODR_25_HZ;
    pt_baro->settings.data_settings.temp_oversampling = BMP388_OVERSAMPLING_2X;
    pt_baro->settings.data_settings.press_oversampling =
      BMP388_OVERSAMPLING_16X;
    pt_baro->settings.dev_settings.power_mode    = BMP388_POWER_MODE_NORMAL;
    pt_baro->settings.dev_settings.sensor_enable = BMP388_SENS_ENABLE_ALL;
    pt_baro->settings.int_settings.int_enable    = BMP388_INT_DISABLE_ALL;

    ret_val  = dd_bmp388_set_data_settings(pt_baro);
    ret_val |= dd_bmp388_set_dev_settings(pt_baro);
    ret_val |= dd_bmp388_set_interrupt_settings(pt_baro);
    bmp_ret  = dd_bmp388_get_error_state(pt_baro);

    if (ret_val != RET_OK || bmp_ret != BMP388_NO_ERROR)
    {
        LOG_ERR("BMP388 settings update failed.\n");
        return -1;
    }

    LOG_INFO("BMP388 settings successfully updated.\n");

    while (1)
    {
        bmp_ret = dd_bmp388_get_data(pt_baro, BMP388_READ_PRESS_TEMP);
        if (bmp_ret == BMP388_NO_ERROR)
        {
            snprintf(
              buffer,
              sizeof(buffer),
              "Pressure: %.2f hPa - Health: %s, Temperature: %.2f C - Health: "
              "%s \n",
              pt_baro->data.pressure,
              pt_baro->data.pressure_health == BMP388_HEALTH_OK ? "OK" : "ERR",
              pt_baro->data.temperature,
              pt_baro->data.temperature_health == BMP388_HEALTH_OK ? "OK"
                                                                   : "ERR");

            LOG_INFO(buffer);
        }
        else if (bmp_ret >= BMP388_WAITING_TEMP)
        {
            LOG_INFO("Waiting data to be ready...\n");
        }
        else
        {
            LOG_ERR("Error reading BMP388 data.\n");
        }

        ps_hard_delay_ms(500);
    }
}
