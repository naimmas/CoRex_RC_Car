#ifndef HA_IIC_PRIVATE_H
#define HA_IIC_PRIVATE_H

#include "stddef.h"
#include "stdint.h"
#include "su_common.h"

#define IIC_DEVICE_CHECK_TRIES 10U

typedef struct st_iic_driver_ifc* iic_driver_ifc;

typedef struct st_iic_driver
{
    iic_driver_ifc api;
    uint8_t        hw_inst_cnt;
} iic_driver_t;

struct st_iic_driver_ifc
{
    /**
     * @brief This function shall initialize the I2C interface.
     * it shall get all hardware instances information from the MCU SDK.
     * @retval `RET_OK` if the interface is initialized successfully.
     * @retval `RET_ERROR` if there is an error during initialization.
     * @retval `RET_NOT_SUPPORTED` if the SDK has no I2C interfaces.
     */
    response_status_t (*init)(void);
    /**
     * @brief This function shall send data to an I2C device.
     * @param[in] uint8_t The index of the I2C interface to use.
     * it's from 0 to `hw_inst_cnt - 1`.
     * @param[in] uint8_t The address of the I2C device to write to.
     * @param[in] uint8_t* Pointer to the data buffer to send.
     * @param[in] size_t The length of the data buffer in bytes.
     * @param[in] timeout_t The timeout for the operation in milliseconds.
     * @return Result of the execution status.
     */
    response_status_t (*write)(uint8_t, uint8_t, const uint8_t*, size_t,
                               timeout_t);
    /**
     * @brief This function shall request data from an I2C device.
     * @param[in] uint8_t The index of the I2C interface to use.
     * it's from 0 to `hw_inst_cnt - 1`.
     * @param[in] uint8_t The address of the I2C device to write to.
     * @param[in] uint8_t* Pointer to the data buffer to write received data.
     * @param[in] size_t The length of the data requested in bytes.
     * @param[in] timeout_t The timeout for the operation in milliseconds.
     * @return Result of the execution status.
     */
    response_status_t (*read)(uint8_t, uint8_t, uint8_t* const, size_t,
                              timeout_t);
    /**
     * @brief This function shall write data to a specific memory address of an
     * I2C device.
     * @param[in] uint8_t The index of the I2C interface to use.
     * it's from 0 to `hw_inst_cnt - 1`.
     * @param[in] uint8_t The address of the I2C device to write to.
     * @param[in] uint16_t The memory address to write to in the I2C device.
     * @param[in] uint8_t The size of the memory address, either 8 or 16 bits.
     * @param[in] uint8_t* Pointer to the data buffer to send.
     * @param[in] size_t The length of the data buffer in bytes.
     * @param[in] timeout_t The timeout for the operation in milliseconds.
     * @return Result of the execution status.
     */
    response_status_t (*mem_write)(uint8_t, uint8_t, uint16_t, uint8_t,
                                   const uint8_t*, size_t, timeout_t);
    /**
     * @brief This function shall read data from a specific memory address of an
     * I2C device.
     * @param[in] uint8_t The index of the I2C interface to use.
     * it's from 0 to `hw_inst_cnt - 1`.
     * @param[in] uint8_t The address of the I2C device to read from.
     * @param[in] uint16_t The memory address to read from in the I2C device.
     * @param[in] uint8_t The size of the memory address, either 8 or 16 bits.
     * @param[out] uint8_t* Pointer to the data buffer to write received data.
     * @param[in] size_t The length of the data requested in bytes.
     * @param[in] timeout_t The timeout for the operation in milliseconds.
     * @return Result of the execution status.
     */
    response_status_t (*mem_read)(uint8_t, uint8_t, uint16_t, uint8_t,
                                  uint8_t* const, size_t, timeout_t);
    /**
     * @brief This function shall recover the stalled I2C bus.
     * It should clocking the SCL multiple times to free up the bus.
     * @note refer to this solution from linux kernel
     * @link
     * https://github.com/torvalds/linux/blob/27605c8c0f69e319df156b471974e4e223035378/drivers/i2c/i2c-core-base.c#L223
     */
    response_status_t (*bus_recover)(uint8_t);
    /**
     * @brief This function shall check if the I2C device is present and ready.
     * It will try to communicate with the device `IIC_DEVICE_CHECK_TRIES`
     * times.
     * @param[in] uint8_t The index of the I2C interface to use.
     * it's from 0 to `hw_inst_cnt - 1`.
     * @param[in] uint8_t The address of the I2C device to check.
     * @param[in] timeout_t The timeout for the operation in milliseconds.
     * @return Result of the execution status.
     */
    response_status_t (*dev_check)(uint8_t, uint8_t, timeout_t);
};

#endif /* HA_IIC_PRIVATE_H */
