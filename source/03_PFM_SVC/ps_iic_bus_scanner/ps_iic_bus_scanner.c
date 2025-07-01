#include "ps_iic_bus_scanner.h"

#include "ha_iic/ha_iic.h"
#include "ps_logger/ps_logger.h"
#include "stdio.h"
#include "string.h"

#define IIC_TIMEOUT_MS (100U)

void ps_bus_scanner_init(void)
{
    ha_iic_init();
}

void ps_scan_iic_bus(void)
{
    uint8_t dev_addr = 0x00;
    LOG_INFO("Scanning I2C bus...\n");

    // NOLINTBEGIN
    for (dev_addr = 0x00; dev_addr < 0x7F; dev_addr++)
    {
        if ((dev_addr & 0x78) == 0 || (dev_addr & 0x78) == 0x78)
        {
            continue; // Skip reserved addresses
        }
        // NOLINTEND

        if (ha_iic_dev_check(IIC_PORT1, dev_addr, IIC_TIMEOUT_MS) == RET_OK)
        {
            LOG_INFO_P1("I2C device found at address: 0x%x\n", dev_addr);
        }
    }
}
