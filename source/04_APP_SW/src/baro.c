#include "baro.h"

#include "string.h"

bmp388_dev_t* g_pt_baro = NULL;

response_status_t baro_get_data(float* ppt_pres_hndlr)
{
    ASSERT_AND_RETURN(g_pt_baro == NULL, RET_PARAM_ERROR);

    bmp388_status_t status = dd_bmp388_get_data(g_pt_baro, BMP388_READ_ALL);
    if (status == BMP388_NO_ERROR)
    {
        *ppt_pres_hndlr = g_pt_baro->data.pressure;
        return RET_OK;
    }
    if (status == BMP388_WAITING_DATA || status == BMP388_WAITING_PRESS
             || status == BMP388_WAITING_TEMP)
    {
        return RET_TIMEOUT;
    }
    else
    {
        return RET_ERROR;
    }
}
response_status_t baro_init()
{
    response_status_t ret_val = dd_bmp388_init(&g_pt_baro, BMP388_DEV_1);
    if (ret_val != RET_OK)
    {
        return ret_val;
    }
    dd_bmp388_reset(g_pt_baro);
    ret_val  = dd_bmp388_get_data_settings(g_pt_baro);
    ret_val |= dd_bmp388_get_dev_settings(g_pt_baro);
    ret_val |= dd_bmp388_get_ifc_settings(g_pt_baro);
    ret_val |= dd_bmp388_get_interrupt_settings(g_pt_baro);

    g_pt_baro->settings.data_settings.iir_filter         = BMP388_IIR_COEFF_15;
    g_pt_baro->settings.data_settings.output_data_rate   = BMP388_ODR_50_HZ;
    g_pt_baro->settings.data_settings.press_oversampling = BMP388_OVERSAMPLING_4X;
    g_pt_baro->settings.data_settings.temp_oversampling  = BMP388_OVERSAMPLING_2X;
    g_pt_baro->settings.dev_settings.power_mode          = BMP388_POWER_MODE_NORMAL;
    g_pt_baro->settings.dev_settings.sensor_enable       = BMP388_SENS_ENABLE_ALL;
    g_pt_baro->settings.int_settings.int_enable          = BMP388_INT_ENABLE_DRDY;
    ret_val                                        |= dd_bmp388_set_ifc_settings(g_pt_baro);
    ret_val                                        |= dd_bmp388_set_data_settings(g_pt_baro);
    ret_val                                        |= dd_bmp388_set_interrupt_settings(g_pt_baro);
    ret_val                                        |= dd_bmp388_set_dev_settings(g_pt_baro);
    bmp388_status_t status                          = dd_bmp388_get_error_state(g_pt_baro);

    if (ret_val != RET_OK || status != BMP388_NO_ERROR)
    {
        return ret_val;
    }

    memset(&(g_pt_baro->settings), 0x00, sizeof(g_pt_baro->settings));
    ret_val  = dd_bmp388_get_data_settings(g_pt_baro);
    ret_val |= dd_bmp388_get_dev_settings(g_pt_baro);
    ret_val |= dd_bmp388_get_ifc_settings(g_pt_baro);
    ret_val |= dd_bmp388_get_interrupt_settings(g_pt_baro);

    if (ret_val != RET_OK)
    {
        return ret_val;
    }

    return ret_val;
}
