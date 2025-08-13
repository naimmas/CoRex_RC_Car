#include "mp_iic.h"

#include "../mp_common.h"
#include "main.h"
#include "su_common.h"

typedef struct st_stm32_iic_driver
{
    iic_driver_t               base;
    I2C_HandleTypeDef* const * hw_insts;
} stm32_iic_driver_t;

static stm32_iic_driver_t g_iic_drv = { .base = { 0U }, .hw_insts = NULL };

static response_status_t init(void)
{
    response_status_t ret_val = RET_OK;

    g_iic_drv.base.hw_inst_cnt = get_iic_ifcs(&g_iic_drv.hw_insts);

    if (g_iic_drv.hw_insts == NULL)
    {
        ret_val = RET_ERROR;
    }
    else if (g_iic_drv.base.hw_inst_cnt == 0)
    {
        ret_val = RET_NOT_SUPPORTED;
    }
    else
    {
        for (uint8_t i = 0; i < g_iic_drv.base.hw_inst_cnt; ++i)
        {
            if (g_iic_drv.hw_insts[i] == NULL)
            {
                ret_val = RET_ERROR;
                break;
            }
        }
    }

    return ret_val;
}

static response_status_t master_write(uint8_t p_ifc_index, uint8_t p_dev_addr,
                                      const uint8_t* ppt_data, size_t p_len, timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_iic_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_len == 0, RET_PARAM_ERROR);

    HAL_StatusTypeDef  hal_ret       = HAL_OK;
    I2C_HandleTypeDef* pt_i2c_handle = g_iic_drv.hw_insts[p_ifc_index];

    hal_ret = HAL_I2C_Master_Transmit(pt_i2c_handle,
                                      p_dev_addr << 1,
                                      (uint8_t*)ppt_data,
                                      p_len,
                                      p_timeout_ms);

    return translate_hal_status(hal_ret);
}

static response_status_t master_read(uint8_t p_ifc_index, uint8_t p_dev_addr,
                                     uint8_t* const ppt_data, size_t p_len, timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_iic_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_len == 0, RET_PARAM_ERROR);

    HAL_StatusTypeDef  hal_ret       = HAL_OK;
    I2C_HandleTypeDef* pt_i2c_handle = g_iic_drv.hw_insts[p_ifc_index];

    hal_ret = HAL_I2C_Master_Receive(pt_i2c_handle, p_dev_addr << 1, ppt_data, p_len, p_timeout_ms);

    return translate_hal_status(hal_ret);
}

static response_status_t mem_write(uint8_t p_ifc_index, uint8_t p_dev_addr, uint16_t p_mem_addr,
                                   uint8_t p_mem_size, const uint8_t* ppt_data, size_t p_len,
                                   timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_iic_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_len == 0, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_mem_size != I2C_MEMADD_SIZE_8BIT && p_mem_size != I2C_MEMADD_SIZE_16BIT,
                      RET_PARAM_ERROR);

    HAL_StatusTypeDef  hal_ret       = HAL_OK;
    I2C_HandleTypeDef* pt_i2c_handle = g_iic_drv.hw_insts[p_ifc_index];

    hal_ret = HAL_I2C_Mem_Write(pt_i2c_handle,
                                p_dev_addr << 1,
                                p_mem_addr,
                                p_mem_size,
                                (uint8_t*)ppt_data,
                                p_len,
                                p_timeout_ms);

    return translate_hal_status(hal_ret);
}

static response_status_t mem_read(uint8_t p_ifc_index, uint8_t p_dev_addr, uint16_t p_mem_addr,
                                  uint8_t p_mem_size, uint8_t* const ppt_data, size_t p_len,
                                  timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_iic_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);
    ASSERT_AND_RETURN(ppt_data == NULL, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_len == 0, RET_PARAM_ERROR);
    ASSERT_AND_RETURN(p_mem_size != I2C_MEMADD_SIZE_8BIT && p_mem_size != I2C_MEMADD_SIZE_16BIT,
                      RET_PARAM_ERROR);

    HAL_StatusTypeDef  hal_ret       = HAL_OK;
    I2C_HandleTypeDef* pt_i2c_handle = g_iic_drv.hw_insts[p_ifc_index];

    hal_ret = HAL_I2C_Mem_Read(pt_i2c_handle,
                               p_dev_addr << 1,
                               p_mem_addr,
                               p_mem_size,
                               ppt_data,
                               p_len,
                               p_timeout_ms);

    return translate_hal_status(hal_ret);
}

static response_status_t is_dev_ready(uint8_t p_ifc_index, uint8_t p_dev_addr,
                                      timeout_t p_timeout_ms)
{
    ASSERT_AND_RETURN(g_iic_drv.hw_insts == NULL, RET_NOT_INITIALIZED);
    ASSERT_AND_RETURN(p_ifc_index >= g_iic_drv.base.hw_inst_cnt, RET_NOT_SUPPORTED);

    HAL_StatusTypeDef  hal_ret       = HAL_OK;
    I2C_HandleTypeDef* pt_i2c_handle = g_iic_drv.hw_insts[p_ifc_index];

    hal_ret =
      HAL_I2C_IsDeviceReady(pt_i2c_handle, p_dev_addr << 1, IIC_DEVICE_CHECK_TRIES, p_timeout_ms);

    return translate_hal_status(hal_ret);
}

static struct st_iic_driver_ifc g_interface = {
    .init        = init,
    .write       = master_write,
    .read        = master_read,
    .mem_write   = mem_write,
    .mem_read    = mem_read,
    .bus_recover = NULL,
    .dev_check   = is_dev_ready,
};

iic_driver_t* iic_driver_register(void)
{
    g_iic_drv.base.api = &g_interface;
    return (iic_driver_t*)&g_iic_drv;
}
