
#include "dd_bmp388.h"
#include "dd_bmp388_defs.h"
#include "mock_ha_iic.h"
#include "unity.h"

#include <stdlib.h>
#include <string.h>

#define BIT(x, y) (y##U << (x))

bmp388_dev_t* baro_sens;
uint8_t*      iic_tx_reg = NULL;
uint16_t      iic_tx_reg_idx = 0U;

typedef struct
{
    uint32_t descriptor;
    uint8_t  data;
} register_data_t;

response_status_t ha_iic_master_mem_write_stub(iic_comm_port_t p_port, uint8_t p_slave_addr, uint8_t* ppt_data_buffer,
                                               size_t p_data_size, uint16_t p_mem_addr, i2c_mem_size_t p_mem_size,
                                               timeout_t p_timeout_ms)
{
    TEST_ASSERT_NOT_NULL(iic_tx_reg);

    for (size_t i = 0; i < p_data_size; i++)
    {
        iic_tx_reg[iic_tx_reg_idx] = ppt_data_buffer[i];
        // printf("Size %d IIC TX[%d]: %02X\n", p_data_size, iic_tx_reg_idx, ppt_data_buffer[i]);
        iic_tx_reg_idx++;
    }
    return RET_OK;
}

void setUp(void) {}

void tearDown(void) {}

void test_bmp388_init_error_in_iic_init_should_return_error(void)
{
    ha_iic_init_ExpectAndReturn(RET_ERROR);

    response_status_t ret_val = dd_bmp388_init(&baro_sens, BMP388_DEV_1);
    TEST_ASSERT_EQUAL(RET_ERROR, ret_val);
}

void test_bmp388_init_with_no_dev_should_return_not_found(void)
{
    uint8_t buff[] = { 0x10 };
    ha_iic_init_ExpectAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(buff);

    response_status_t ret_val = dd_bmp388_init(&baro_sens, BMP388_DEV_1);
    TEST_ASSERT_EQUAL(RET_NOT_FOUND, ret_val);
}

void test_bmp388_init_with_dev_should_return_ok(void)
{
    uint8_t buff[] = { BMP388_CHIP_ID };
    uint8_t sample_calib_data[] = {
        0x15, 0x6C, 0x4F, 0x4A, 0xF6, 0xC5, 0x01, 0x57, 0xF6, 0x19, 0x00,
        0xEE, 0x61, 0x9B, 0x78, 0xFC, 0xF6, 0x19, 0x41, 0x1B, 0xC4,
    };

    ha_iic_init_ExpectAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(buff);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(sample_calib_data, sizeof(sample_calib_data));
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);

    response_status_t ret_val = dd_bmp388_init(&baro_sens, BMP388_DEV_1);
    TEST_ASSERT_EQUAL(RET_OK, ret_val);
}

void test_bmp388_set_get_dev_settings(void)
{

    uint8_t           data_buffer = 0x00;
    response_status_t ret_val;
    register_data_t   pwr_reg[] = {
        {  .descriptor = BMP388_POWER_MODE_SLEEP, .data = BIT(5, 0) | BIT(4, 0) },
        { .descriptor = BMP388_POWER_MODE_FORCED, .data = BIT(5, 0) | BIT(4, 1) },
        { .descriptor = BMP388_POWER_MODE_NORMAL, .data = BIT(5, 1) | BIT(4, 1) }
    };

    register_data_t sensor_enables[] = {
        {      .descriptor = BMP388_SENS_DISABLE, .data = BIT(1, 0) | BIT(0, 0) },
        { .descriptor = BMP388_SENS_ENABLE_PRESS, .data = BIT(1, 0) | BIT(0, 1) },
        {  .descriptor = BMP388_SENS_ENABLE_TEMP, .data = BIT(1, 1) | BIT(0, 0) },
        {   .descriptor = BMP388_SENS_ENABLE_ALL, .data = BIT(1, 1) | BIT(0, 1) },
    };
    iic_tx_reg_idx = 0U;
    iic_tx_reg = (uint8_t*)malloc(1 * ARRAY_SIZE(pwr_reg) * ARRAY_SIZE(sensor_enables) * sizeof(uint8_t));
    for (size_t i = 0; i < ARRAY_SIZE(pwr_reg); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(sensor_enables); j++)
        {
            baro_sens->settings.dev_settings.power_mode = pwr_reg[i].descriptor;
            baro_sens->settings.dev_settings.sensor_enable = sensor_enables[j].descriptor;

            ha_iic_master_mem_write_StubWithCallback(ha_iic_master_mem_write_stub);
            ret_val = dd_bmp388_set_dev_settings(baro_sens);

            data_buffer = pwr_reg[i].data | sensor_enables[j].data;
            TEST_ASSERT_EQUAL(data_buffer, iic_tx_reg[iic_tx_reg_idx - 1]);
            TEST_ASSERT_EQUAL(RET_OK, ret_val);
        }
    }

    data_buffer = 0x00;

    for (size_t i = 0; i < ARRAY_SIZE(pwr_reg); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(sensor_enables); j++)
        {
            data_buffer = pwr_reg[i].data | sensor_enables[j].data;

            ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
            ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&data_buffer);
            ret_val = dd_bmp388_get_dev_settings(baro_sens);

            TEST_ASSERT_EQUAL(pwr_reg[i].descriptor, baro_sens->settings.dev_settings.power_mode);
            TEST_ASSERT_EQUAL(sensor_enables[j].descriptor, baro_sens->settings.dev_settings.sensor_enable);
            TEST_ASSERT_EQUAL(RET_OK, ret_val);
        }
    }
}

void test_bmp388_set_get_ifc_settings(void)
{
    uint8_t           data_buffer = 0x00;
    response_status_t ret_val;
    register_data_t   spi_modes[] = {
        { .descriptor = BMP388_IFC_SPI_MODE_3_WIRE, .data = BIT(0, 0) },
        { .descriptor = BMP388_IFC_SPI_MODE_4_WIRE, .data = BIT(0, 1) }
    };

    register_data_t iic_wdts[] = {
        { .descriptor = BMP388_IIC_WDT_DISABLE, .data = BIT(1, 0) | BIT(2, 0) },
        { .descriptor = BMP388_IIC_WDT_1_25_MS, .data = BIT(1, 1) | BIT(2, 0) },
        {   .descriptor = BMP388_IIC_WDT_40_MS, .data = BIT(1, 1) | BIT(2, 1) }
    };

    iic_tx_reg_idx = 0U;
    iic_tx_reg = (uint8_t*)malloc(1 * ARRAY_SIZE(spi_modes) * ARRAY_SIZE(iic_wdts) * sizeof(uint8_t));
    for (size_t i = 0; i < ARRAY_SIZE(spi_modes); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(iic_wdts); j++)
        {
            baro_sens->settings.comm_ifc_settings.spi_mode = spi_modes[i].descriptor;
            baro_sens->settings.comm_ifc_settings.iic_wdt = iic_wdts[j].descriptor;

            ha_iic_master_mem_write_StubWithCallback(ha_iic_master_mem_write_stub);
            ret_val = dd_bmp388_set_ifc_settings(baro_sens);

            data_buffer = spi_modes[i].data | iic_wdts[j].data;
            TEST_ASSERT_EQUAL(data_buffer, iic_tx_reg[iic_tx_reg_idx - 1]);
            TEST_ASSERT_EQUAL(RET_OK, ret_val);
        }
    }

    data_buffer = 0x00;

    for (size_t i = 0; i < ARRAY_SIZE(spi_modes); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(iic_wdts); j++)
        {
            data_buffer = spi_modes[i].data | iic_wdts[j].data;

            ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
            ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&data_buffer);
            ret_val = dd_bmp388_get_ifc_settings(baro_sens);
            TEST_ASSERT_EQUAL(spi_modes[i].descriptor, baro_sens->settings.comm_ifc_settings.spi_mode);
            TEST_ASSERT_EQUAL(iic_wdts[j].descriptor, baro_sens->settings.comm_ifc_settings.iic_wdt);
            TEST_ASSERT_EQUAL(RET_OK, ret_val);
        }
    }
}

void test_bmp388_set_get_interrupt_settings(void)
{
    uint8_t           data_buffer = 0x00;
    response_status_t ret_val;
    register_data_t   int_types[] = {
        {  .descriptor = BMP388_INT_TYPE_PP_LOW_NON_LATCHED, .data = BIT(0, 0) | BIT(1, 0) | BIT(2, 0) },
        {  .descriptor = BMP388_INT_TYPE_OD_LOW_NON_LATCHED, .data = BIT(0, 1) | BIT(1, 0) | BIT(2, 0) },
        { .descriptor = BMP388_INT_TYPE_PP_HIGH_NON_LATCHED, .data = BIT(0, 0) | BIT(1, 1) | BIT(2, 0) },
        { .descriptor = BMP388_INT_TYPE_OD_HIGH_NON_LATCHED, .data = BIT(0, 1) | BIT(1, 1) | BIT(2, 0) },
        {      .descriptor = BMP388_INT_TYPE_PP_LOW_LATCHED, .data = BIT(0, 0) | BIT(1, 0) | BIT(2, 1) },
        {      .descriptor = BMP388_INT_TYPE_OD_LOW_LATCHED, .data = BIT(0, 1) | BIT(1, 0) | BIT(2, 1) },
        {     .descriptor = BMP388_INT_TYPE_PP_HIGH_LATCHED, .data = BIT(0, 0) | BIT(1, 1) | BIT(2, 1) },
        {     .descriptor = BMP388_INT_TYPE_OD_HIGH_LATCHED, .data = BIT(0, 1) | BIT(1, 1) | BIT(2, 1) },
    };

    register_data_t int_enables[] = {
        {       .descriptor = BMP388_INT_DISABLE_ALL, .data = BIT(3, 0) | BIT(4, 0) | BIT(6, 0) },
        {       .descriptor = BMP388_INT_ENABLE_FWTM, .data = BIT(3, 1) | BIT(4, 0) | BIT(6, 0) },
        {      .descriptor = BMP388_INT_ENABLE_FFULL, .data = BIT(3, 0) | BIT(4, 1) | BIT(6, 0) },
        { .descriptor = BMP388_INT_ENABLE_FWTM_FFULL, .data = BIT(3, 1) | BIT(4, 1) | BIT(6, 0) },
        {       .descriptor = BMP388_INT_ENABLE_DRDY, .data = BIT(3, 0) | BIT(4, 0) | BIT(6, 1) },
        {  .descriptor = BMP388_INT_ENABLE_FWTM_DRDY, .data = BIT(3, 1) | BIT(4, 0) | BIT(6, 1) },
        { .descriptor = BMP388_INT_ENABLE_FFULL_DRDY, .data = BIT(3, 0) | BIT(4, 1) | BIT(6, 1) },
        {        .descriptor = BMP388_INT_ENABLE_ALL, .data = BIT(3, 1) | BIT(4, 1) | BIT(6, 1) }
    };
    iic_tx_reg_idx = 0U;
    iic_tx_reg = (uint8_t*)malloc(1 * ARRAY_SIZE(int_types) * ARRAY_SIZE(int_enables) * sizeof(uint8_t));
    for (size_t i = 0; i < ARRAY_SIZE(int_types); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(int_enables); j++)
        {
            baro_sens->settings.int_settings.int_type = int_types[i].descriptor;
            baro_sens->settings.int_settings.int_enable = int_enables[j].descriptor;

            ha_iic_master_mem_write_StubWithCallback(ha_iic_master_mem_write_stub);
            ret_val = dd_bmp388_set_interrupt_settings(baro_sens);
            data_buffer = int_types[i].data | int_enables[j].data;
            TEST_ASSERT_EQUAL(data_buffer, iic_tx_reg[iic_tx_reg_idx - 1]);
            TEST_ASSERT_EQUAL(RET_OK, ret_val);
        }
    }
    data_buffer = 0x00;
    for (size_t i = 0; i < ARRAY_SIZE(int_types); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(int_enables); j++)
        {
            data_buffer = int_types[i].data | int_enables[j].data;

            ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
            ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&data_buffer);
            ret_val = dd_bmp388_get_interrupt_settings(baro_sens);
            TEST_ASSERT_EQUAL(int_types[i].descriptor, baro_sens->settings.int_settings.int_type);
            TEST_ASSERT_EQUAL(int_enables[j].descriptor, baro_sens->settings.int_settings.int_enable);
            TEST_ASSERT_EQUAL(RET_OK, ret_val);
        }
    }
}

void test_dd_bmp388_set_get_data_settings(void)
{
    uint8_t           data_buffer[3] = { 0x00 };
    response_status_t ret_val;

    register_data_t iir[] = {
        {   .descriptor = BMP388_IIR_DISABLE, .data = 0 << 1 },
        {   .descriptor = BMP388_IIR_COEFF_1, .data = 1 << 1 },
        {   .descriptor = BMP388_IIR_COEFF_3, .data = 2 << 1 },
        {   .descriptor = BMP388_IIR_COEFF_7, .data = 3 << 1 },
        {  .descriptor = BMP388_IIR_COEFF_15, .data = 4 << 1 },
        {  .descriptor = BMP388_IIR_COEFF_31, .data = 5 << 1 },
        {  .descriptor = BMP388_IIR_COEFF_63, .data = 6 << 1 },
        { .descriptor = BMP388_IIR_COEFF_127, .data = 7 << 1 },
    };

    register_data_t odr[] = {
        { .descriptor = BMP388_ODR_0P0015_HZ, .data = 17 },
        {  .descriptor = BMP388_ODR_0P003_HZ, .data = 16 },
        {  .descriptor = BMP388_ODR_0P006_HZ, .data = 15 },
        {   .descriptor = BMP388_ODR_0P01_HZ, .data = 14 },
        {   .descriptor = BMP388_ODR_0P02_HZ, .data = 13 },
        {   .descriptor = BMP388_ODR_0P05_HZ, .data = 12 },
        {   .descriptor = BMP388_ODR_0P10_HZ, .data = 11 },
        {   .descriptor = BMP388_ODR_0P20_HZ, .data = 10 },
        {   .descriptor = BMP388_ODR_0P39_HZ,  .data = 9 },
        {   .descriptor = BMP388_ODR_0P78_HZ,  .data = 8 },
        {    .descriptor = BMP388_ODR_1P5_HZ,  .data = 7 },
        {    .descriptor = BMP388_ODR_3P1_HZ,  .data = 6 },
        {   .descriptor = BMP388_ODR_6P25_HZ,  .data = 5 },
        {   .descriptor = BMP388_ODR_12P5_HZ,  .data = 4 },
        {     .descriptor = BMP388_ODR_25_HZ,  .data = 3 },
        {     .descriptor = BMP388_ODR_50_HZ,  .data = 2 },
        {    .descriptor = BMP388_ODR_100_HZ,  .data = 1 },
        {    .descriptor = BMP388_ODR_200_HZ,  .data = 0 },
    };

    register_data_t osrp[] = {
        { .descriptor = BMP388_OVERSAMPLING_NONE, .data = 0 },
        {   .descriptor = BMP388_OVERSAMPLING_2X, .data = 1 },
        {   .descriptor = BMP388_OVERSAMPLING_4X, .data = 2 },
        {   .descriptor = BMP388_OVERSAMPLING_8X, .data = 3 },
        {  .descriptor = BMP388_OVERSAMPLING_16X, .data = 4 },
        {  .descriptor = BMP388_OVERSAMPLING_32X, .data = 5 },
    };

    register_data_t osrt[] = {
        { .descriptor = BMP388_OVERSAMPLING_NONE, .data = 0 << 3 },
        {   .descriptor = BMP388_OVERSAMPLING_2X, .data = 1 << 3 },
        {   .descriptor = BMP388_OVERSAMPLING_4X, .data = 2 << 3 },
        {   .descriptor = BMP388_OVERSAMPLING_8X, .data = 3 << 3 },
        {  .descriptor = BMP388_OVERSAMPLING_16X, .data = 4 << 3 },
        {  .descriptor = BMP388_OVERSAMPLING_32X, .data = 5 << 3 },
    };
    iic_tx_reg_idx = 0U;
    iic_tx_reg = (uint8_t*)malloc(3 * ARRAY_SIZE(iir) * ARRAY_SIZE(odr) * ARRAY_SIZE(osrp) * sizeof(uint8_t));
    for (size_t i = 0; i < ARRAY_SIZE(iir); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(odr); j++)
        {
            for (size_t k = 0; k < ARRAY_SIZE(osrp); k++)
            {
                baro_sens->settings.data_settings.iir_filter = iir[i].descriptor;
                baro_sens->settings.data_settings.output_data_rate = odr[j].descriptor;
                baro_sens->settings.data_settings.press_oversampling = osrp[k].descriptor;
                baro_sens->settings.data_settings.temp_oversampling = osrt[k].descriptor;

                data_buffer[0] = (osrp[k].data | osrt[k].data);
                data_buffer[1] = odr[j].data;
                data_buffer[2] = iir[i].data;

                ha_iic_master_mem_write_StubWithCallback(ha_iic_master_mem_write_stub);

                ret_val = dd_bmp388_set_data_settings(baro_sens);
                TEST_ASSERT_EQUAL(data_buffer[0], iic_tx_reg[iic_tx_reg_idx - 3]);
                TEST_ASSERT_EQUAL(data_buffer[1], iic_tx_reg[iic_tx_reg_idx - 2]);
                TEST_ASSERT_EQUAL(data_buffer[2], iic_tx_reg[iic_tx_reg_idx - 1]);
                TEST_ASSERT_EQUAL(RET_OK, ret_val);
            }
        }
    }

    memset(data_buffer, 0x00, sizeof(data_buffer));
    for (size_t i = 0; i < ARRAY_SIZE(iir); i++)
    {
        for (size_t j = 0; j < ARRAY_SIZE(odr); j++)
        {
            for (size_t k = 0; k < ARRAY_SIZE(osrp); k++)
            {
                data_buffer[0] = (osrp[k].data | osrt[k].data);
                data_buffer[1] = odr[j].data;
                data_buffer[2] = iir[i].data;

                ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
                ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&data_buffer[0]);
                ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
                ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&data_buffer[1]);
                ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
                ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&data_buffer[2]);

                ret_val = dd_bmp388_get_data_settings(baro_sens);
                TEST_ASSERT_EQUAL(iir[i].descriptor, baro_sens->settings.data_settings.iir_filter);
                TEST_ASSERT_EQUAL(odr[j].descriptor, baro_sens->settings.data_settings.output_data_rate);
                TEST_ASSERT_EQUAL(osrt[k].descriptor, baro_sens->settings.data_settings.temp_oversampling);
                TEST_ASSERT_EQUAL(osrp[k].descriptor, baro_sens->settings.data_settings.press_oversampling);
                TEST_ASSERT_EQUAL(RET_OK, ret_val);
            }
        }
    }
}

void test_dd_bmp388_get_dev(void)
{
    for (bmp388_devices_t i = BMP388_DEV_1; i < BMP388_DEV_CNT; i++)
    {
        TEST_ASSERT_NOT_NULL(dd_bmp388_get_dev(BMP388_DEV_1));
    }
}

void test_dd_bmp388_get_data_comm_failed_should_return_error(void)
{
    response_status_t     ret_val;
    bmp388_data_request_t data_request[] = { BMP388_READ_TIME,
                                             BMP388_READ_PRESSURE,
                                             BMP388_READ_TEMP,
                                             BMP388_READ_PRESS_TEMP,
                                             BMP388_READ_ALL };
    for (size_t i = 0; i < ARRAY_SIZE(data_request); i++)
    {
        ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_ERROR);

        TEST_ASSERT_EQUAL(0, baro_sens->data.sensortime);
        TEST_ASSERT_EQUAL(0, baro_sens->data.temperature);
        TEST_ASSERT_EQUAL(0, baro_sens->data.pressure);
        TEST_ASSERT_EQUAL(BMP388_ERROR_API, dd_bmp388_get_data(baro_sens, data_request[i]));
    }
}

void test_dd_bmp388_get_data_temp_not_ready_should_return_busy(void)
{
    response_status_t     ret_val;
    bmp388_data_request_t data_request[] = { BMP388_READ_TEMP, BMP388_READ_PRESSURE, BMP388_READ_PRESS_TEMP };

    uint8_t rdy_buf = 0x00;
    for (size_t i = 0; i < ARRAY_SIZE(data_request); i++)
    {
        ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
        ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
        ret_val = dd_bmp388_get_data(baro_sens, data_request[i]);
        TEST_ASSERT_EQUAL(BMP388_WAITING_TEMP, ret_val);
        TEST_ASSERT_EQUAL(0, baro_sens->data.sensortime);
        TEST_ASSERT_EQUAL(0, baro_sens->data.temperature);
        TEST_ASSERT_EQUAL(0, baro_sens->data.pressure);
    }
}

void test_dd_bmp388_get_data_temp_ready_pressure_not_ready_should_return_busy(void)
{
    response_status_t     ret_val;
    bmp388_data_request_t data_request = BMP388_READ_PRESSURE;

    uint8_t rdy_buf = 0b01000000;
    uint8_t temp_buf[] = { 0x00, 0x62, 0x81 };
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(temp_buf, sizeof(temp_buf));
    ret_val = dd_bmp388_get_data(baro_sens, data_request);
    TEST_ASSERT_EQUAL(BMP388_WAITING_PRESS, ret_val);
    TEST_ASSERT_EQUAL(0, baro_sens->data.sensortime);
    TEST_ASSERT_EQUAL(0, baro_sens->data.temperature);
    TEST_ASSERT_EQUAL(0, baro_sens->data.pressure);
}

void test_dd_bmp388_get_data_max_should_not_cause_overflow(void)
{
    response_status_t     ret_val;
    bmp388_data_request_t data_request = BMP388_READ_PRESS_TEMP;

    uint8_t rdy_buf = 0b01100000;
    uint8_t temp_buf[] = { 0xFF, 0xFF, 0xFF };
    uint8_t pres_buf[] = { 0x00, 0x00, 0x00 };

    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(temp_buf, sizeof(temp_buf));
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(pres_buf, sizeof(pres_buf));

    ret_val = dd_bmp388_get_data(baro_sens, data_request);
    TEST_ASSERT_EQUAL(BMP388_NO_ERROR, ret_val);
    TEST_ASSERT_EQUAL_FLOAT(BMP388_MAX_TEMP, baro_sens->data.temperature);
    TEST_ASSERT_EQUAL(BMP388_HEALTH_CRITICAL, baro_sens->data.temperature_health);
    TEST_ASSERT_EQUAL_FLOAT(BMP388_MAX_PRES, baro_sens->data.pressure);
    TEST_ASSERT_EQUAL(BMP388_HEALTH_CRITICAL, baro_sens->data.pressure_health);
}

void test_dd_bmp388_get_data_min_should_not_cause_overflow(void)
{
    response_status_t     ret_val;
    bmp388_data_request_t data_request = BMP388_READ_PRESS_TEMP;

    uint8_t rdy_buf = 0b01100000;
    uint8_t temp_buf[] = { 0x00, 0x00, 0x00 };
    uint8_t pres_buf[] = { 0xFF, 0xFF, 0xFF };

    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(temp_buf, sizeof(temp_buf));
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(pres_buf, sizeof(pres_buf));

    ret_val = dd_bmp388_get_data(baro_sens, data_request);
    TEST_ASSERT_EQUAL(BMP388_NO_ERROR, ret_val);
    TEST_ASSERT_EQUAL_FLOAT(BMP388_MIN_TEMP, baro_sens->data.temperature);
    TEST_ASSERT_EQUAL(BMP388_HEALTH_CRITICAL, baro_sens->data.temperature_health);
    TEST_ASSERT_EQUAL_FLOAT(BMP388_MIN_PRES, baro_sens->data.pressure);
    TEST_ASSERT_EQUAL(BMP388_HEALTH_CRITICAL, baro_sens->data.pressure_health);
}

void test_dd_bmp388_get_data_temp_and_pressure_ready_should_return_ok(void)
{
    response_status_t     ret_val;
    bmp388_data_request_t data_request = BMP388_READ_PRESS_TEMP;

    uint8_t rdy_buf = 0b01100000;
    uint8_t temp_buf[] = { 0x00, 0x62, 0x81 };
    uint8_t pres_buf[] = { 0x80, 0x0A, 0x6C };

    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(temp_buf, sizeof(temp_buf));
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&rdy_buf);
    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
    ha_iic_master_mem_read_ReturnMemThruPtr_ppt_data_buffer(pres_buf, sizeof(pres_buf));

    ret_val = dd_bmp388_get_data(baro_sens, data_request);
    TEST_ASSERT_EQUAL(BMP388_NO_ERROR, ret_val);
    TEST_ASSERT_EQUAL(0, baro_sens->data.sensortime);
    TEST_ASSERT_EQUAL_FLOAT(24.6625, baro_sens->data.temperature);
    TEST_ASSERT_EQUAL_FLOAT(101269.68, baro_sens->data.pressure);
}

void test_dd_bmp388_get_error_codes(void)
{
    register_data_t error_codes[] = {
        { .descriptor = BMP388_NO_ERROR, .data = 0x00 },
        { .descriptor = BMP388_ERROR_FATAL, .data = BIT(0, 1) },
        { .descriptor = BMP388_ERROR_CMD, .data = BIT(1, 1) },
        { .descriptor = BMP388_ERROR_CONFIG, .data = BIT(2, 1) },
    };
    uint8_t buff = 0x00;

    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_ERROR);
    TEST_ASSERT_EQUAL(BMP388_ERROR_API, dd_bmp388_get_error_state(baro_sens));

    ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_BUSY);
    TEST_ASSERT_EQUAL(BMP388_ERROR_API, dd_bmp388_get_error_state(baro_sens));

    for (size_t i = 0; i < ARRAY_SIZE(error_codes); i++)
    {
        buff = error_codes[i].data;
        ha_iic_master_mem_read_ExpectAnyArgsAndReturn(RET_OK);
        ha_iic_master_mem_read_ReturnThruPtr_ppt_data_buffer(&buff);
        TEST_ASSERT_EQUAL(error_codes[i].descriptor, dd_bmp388_get_error_state(baro_sens));
    }
}
