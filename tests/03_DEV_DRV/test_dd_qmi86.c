

#include "unity.h"
#include "mock_ha_iic.h"
#include "mock_ha_timer.h"
#include "dd_qmi86.h"
#include "dd_qmi86_defs.h"

#define QMI_CLR_BITS(reg_data, bitname) ((reg_data) & (~bitname##_MSK))
#define QMI_SET_BITS(bitname, data) ((data) << (bitname##_POS))
#define QMI_GET_BITS(reg_data, bitname) (((reg_data) & (bitname##_MSK)) >> (bitname##_POS))
#define QMI_OWR_BITS(reg_data, bitname, data) ((QMI_CLR_BITS(reg_data, bitname)) | (QMI_SET_BITS(bitname, data)))

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bit_manip(void)
{
    uint8_t reg_data = 0b01010100;
    
    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AODR, QMI86_ACC_ODR_1000_HZ);
    reg_data = QMI_OWR_BITS(reg_data, QMI86_REG_CTRL2_AST, 0x01);
    
    TEST_ASSERT_EQUAL(0b11010011, reg_data);
}

void test_dd_qmi86_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement dd_qmi86");
}
