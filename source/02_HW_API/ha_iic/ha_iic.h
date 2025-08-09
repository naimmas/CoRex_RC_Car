#ifndef HA_IIC_H
#define HA_IIC_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

/**
 * @brief This macro links I2C devices to their respective ports and addresses.
 * It should be used by the drivers to setup the I2C device connections.
 * @param[in] dev_cnt The number of I2C devices.
 * @param[in] ... A variable number of port and address pairs for each device.
 * This should be assigned using `IIC_DEFINE_CONNECTION` macro.
 */
#define IIC_SETUP_PORT_CONNECTION(dev_cnt, ...) \
    static const uint16_t iic_dev_ports[dev_cnt] = { \
        __VA_ARGS__ \
    };

/**
 * @brief This macro encodes the I2C device port and address into a 16-bit word.
 * @note this macro is used with `IIC_SETUP_PORT_CONNECTION`
 * @param[in] port The I2C communication port number should be one of the
 * `iic_comm_port_t` enum values.
 * @param[in] dev_idx The index of the I2C device it should be from 0 to
 * `dev_cnt - 1`.
 * @param[in] dev_addrs The address of the I2C device. It should be a valid
 * 7-bit address.
 */
#define IIC_DEFINE_CONNECTION(port, dev_idx, dev_addrs) \
    BYTES_TO_WORD(unsigned, dev_addrs, port)

/**
 * @brief This macro retrieves the I2C device port number from the encoded port
 * and address.
 * @param[in] dev_idx The index of the I2C device it should be from 0 to
 * `dev_cnt - 1`.
 * @return The I2C communication port number as a byte.
 */
#define IIC_GET_DEV_PORT(dev_idx) \
    (BYTE_HIGH(iic_dev_ports[dev_idx]))

/**
 * @brief This macro retrieves the I2C device address from the encoded port and
 * address.
 * @param[in] dev_idx The index of the I2C device it should be from 0 to
 * `dev_cnt - 1`.
 * @return The I2C device address as a byte.
 */
#define IIC_GET_DEV_ADDRESS(dev_idx) \
    (BYTE_LOW(iic_dev_ports[dev_idx]))

typedef struct st_iic_driver* iic_driver;

typedef enum en_iic_comm_port
{
    IIC_PORT1 = 0,
    IIC_PORT_CNT,
} iic_comm_port_t;

typedef enum en_iic_mem_size
{
    HW_IIC_MEM_SZ_8BIT  = 0x01,
    HW_IIC_MEM_SZ_16BIT = 0x02,
} i2c_mem_size_t;

response_status_t ha_iic_init(void);
response_status_t ha_iic_master_read(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                     uint8_t* ppt_data_buffer, size_t p_data_size,
                                     timeout_t p_timeout_ms);
response_status_t ha_iic_master_write(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                      const uint8_t* ppt_data_buffer, size_t p_data_size,
                                      timeout_t p_timeout_ms);
response_status_t ha_iic_master_mem_read(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                         uint8_t* ppt_data_buffer, size_t p_data_size,
                                         uint16_t p_mem_addr, i2c_mem_size_t p_mem_size,
                                         timeout_t p_timeout_ms);
response_status_t ha_iic_master_mem_write(iic_comm_port_t p_port, uint8_t p_slave_addr,
                                          const uint8_t* ppt_data_buffer, size_t p_data_size,
                                          uint16_t p_mem_addr, i2c_mem_size_t p_mem_size,
                                          timeout_t p_timeout_ms);
response_status_t ha_iic_bus_recover(iic_comm_port_t p_port);
response_status_t ha_iic_dev_check(iic_comm_port_t p_port, uint8_t p_dev_addr,
                                   timeout_t p_timeout_ms);

#endif /* HA_IIC_H */
