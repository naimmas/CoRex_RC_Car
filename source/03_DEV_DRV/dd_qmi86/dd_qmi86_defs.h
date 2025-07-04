#ifndef DD_QMI86_DEFS_H
#define DD_QMI86_DEFS_H

/** @brief Device Identifier */
#define QMI86_REG_WHO_AM_I (0x0)

/** @brief Device Revision ID */
#define QMI86_REG_REVISION_ID (0x1)

/** @brief SPI Interface and Sensor Enable */
#define QMI86_REG_CTRL1 (0x2)
#define QMI86_REG_CTRL1_SENS_DISABLE_POS (0x00)
#define QMI86_REG_CTRL1_SENS_DISABLE_MSK (0x01)

#define QMI86_REG_CTRL1_FIFO_INT_SEL_POS (0x02)
#define QMI86_REG_CTRL1_FIFO_INT_SEL_MSK (0x04)

#define QMI86_REG_CTRL1_INT1_EN_POS (0x03)
#define QMI86_REG_CTRL1_INT1_EN_MSK (0x08)

#define QMI86_REG_CTRL1_INT2_EN_POS (0x04)
#define QMI86_REG_CTRL1_INT2_EN_MSK (0x10)

#define QMI86_REG_CTRL1_BE_POS (0x05)
#define QMI86_REG_CTRL1_BE_MSK (0x20)

#define QMI86_REG_CTRL1_ADDR_AI_POS (0x06)
#define QMI86_REG_CTRL1_ADDR_AI_MSK (0x40)

#define QMI86_REG_CTRL1_SIM_POS (0x07)
#define QMI86_REG_CTRL1_SIM_MSK (0x80)

/** @brief Accelerometer: Output Data Rate, Full Scale, Self-Test */
#define QMI86_REG_CTRL2 (0x3)
#define QMI86_REG_CTRL2_AODR_POS (0x00)
#define QMI86_REG_CTRL2_AODR_MSK (0x0F)

#define QMI86_REG_CTRL2_AFS_POS (0x04)
#define QMI86_REG_CTRL2_AFS_MSK (0x70)

#define QMI86_REG_CTRL2_AST_POS (0x07)
#define QMI86_REG_CTRL2_AST_MSK (0x80)

/** @brief Gyroscope: Output Data Rate, Full Scale, Self-Test */
#define QMI86_REG_CTRL3 (0x4)
#define QMI86_REG_CTRL3_GODR_POS (0x00)
#define QMI86_REG_CTRL3_GODR_MSK (0x0F)

#define QMI86_REG_CTRL3_GFS_POS (0x04)
#define QMI86_REG_CTRL3_GFS_MSK (0x70)

#define QMI86_REG_CTRL3_GST_POS (0x07)
#define QMI86_REG_CTRL3_GST_MSK (0x80)
/** @brief Low pass filter setting */
#define QMI86_REG_CTRL5 (0x6)
#define QMI86_REG_CTRL5_ALPF_EN_POS (0x00)
#define QMI86_REG_CTRL5_ALPF_EN_MSK (0x01)

#define QMI86_REG_CTRL5_ALPF_MODE_POS (0x01)
#define QMI86_REG_CTRL5_ALPF_MODE_MSK (0x06)

#define QMI86_REG_CTRL5_GLPF_EN_POS (0x04)
#define QMI86_REG_CTRL5_GLPF_EN_MSK (0x10)

#define QMI86_REG_CTRL5_GLPF_MODE_POS (0x05)
#define QMI86_REG_CTRL5_GLPF_MODE_MSK (0x60)
/** @brief Enable Sensors */
#define QMI86_REG_CTRL7 (0x8)
#define QMI86_REG_CTRL7_SYNC_SAMPLE_POS (0x07)
#define QMI86_REG_CTRL7_SYNC_SAMPLE_MSK (0x80)

#define QMI86_REG_CTRL7_DRDY_DIS_POS (0x05)
#define QMI86_REG_CTRL7_DRDY_DIS_MSK (0x20)

#define QMI86_REG_CTRL7_GSN_POS (0x04)
#define QMI86_REG_CTRL7_GSN_MSK (0x10)

#define QMI86_REG_CTRL7_AGEN_POS (0x00)
#define QMI86_REG_CTRL7_AGEN_MSK (0x03)

/** @brief Motion Detection Control */
#define QMI86_REG_CTRL8 (0x9)
#define QMI86_REG_CTRL8_CTRL9_HANDSHAKE_POS (0x07)
#define QMI86_REG_CTRL8_CTRL9_HANDSHAKE_MSK (0x80)

/** @brief Host Commands */
#define QMI86_REG_CTRL9 (0x0A)

/** @brief Calibration Register */
#define QMI86_REG_CAL1_L (0x0B)
#define QMI86_REG_CAL1_H (0x0C)

/** @brief Calibration Register */
#define QMI86_REG_CAL2_L (0x0D)
#define QMI86_REG_CAL2_H (0x0E)

/** @brief Calibration Register */
#define QMI86_REG_CAL3_L (0x0F)
#define QMI86_REG_CAL3_H (0x10)

/** @brief Calibration Register */
#define QMI86_REG_CAL4_L (0x11)
#define QMI86_REG_CAL4_H (0x12)

/** @brief FIFO watermark level, in ODRs */
#define QMI86_REG_FIFO_WTM_TH (0x13)

/** @brief FIFO Setup */
#define QMI86_REG_FIFO_CTRL (0x14)
#define QMI86_REG_FIFO_CTRL_FIFO_MODE_POS (0x00)
#define QMI86_REG_FIFO_CTRL_FIFO_MODE_MSK (0x03)

#define QMI86_REG_FIFO_CTRL_FIFO_SIZE_POS (0x02)
#define QMI86_REG_FIFO_CTRL_FIFO_SIZE_MSK (0x0C)

#define QMI86_REG_FIFO_CTRL_FIFO_RD_MODE_POS (0x07)
#define QMI86_REG_FIFO_CTRL_FIFO_RD_MODE_MSK (0x80)

/** @brief FIFO sample count LSBs */
#define QMI86_REG_FIFO_SMPL_CNT_LSB (0x15)

/** @brief FIFO Status */
#define QMI86_REG_FIFO_STATUS (0x16)
#define QMI86_REG_FIFO_STATUS_FIFO_SMPL_CNT_MSB_POS (0x00)
#define QMI86_REG_FIFO_STATUS_FIFO_SMPL_CNT_MSB_MSK (0x03)

#define QMI86_REG_FIFO_STATUS_FIFO_NOT_EMPTY_POS (0x04)
#define QMI86_REG_FIFO_STATUS_FIFO_NOT_EMPTY_MSK (0x10)

#define QMI86_REG_FIFO_STATUS_FIFO_OVFLOW_POS (0x05)
#define QMI86_REG_FIFO_STATUS_FIFO_OVFLOW_MSK (0x20)

#define QMI86_REG_FIFO_STATUS_FIFO_WTM_POS (0x06)
#define QMI86_REG_FIFO_STATUS_FIFO_WTM_MSK (0x40)

#define QMI86_REG_FIFO_STATUS_FIFO_FULL_POS (0x07)
#define QMI86_REG_FIFO_STATUS_FIFO_FULL_MSK (0x80)

/** @brief FIFO Data */
#define QMI86_REG_FIFO_DATA (0x17)

/** @brief Sensor Data Availability with the Locking mechanism, CmdDone (CTRL9
 * protocol bit). */
#define QMI86_REG_STATUSINT (0x2D)
#define QMI86_REG_STATUSINT_AVAIL_POS (0x00)
#define QMI86_REG_STATUSINT_AVAIL_MSK (0x01)

#define QMI86_REG_STATUSINT_LOCKED_POS (0x01)
#define QMI86_REG_STATUSINT_LOCKED_MSK (0x02)

#define QMI86_REG_STATUSINT_CMD_DONE_POS (0x07)
#define QMI86_REG_STATUSINT_CMD_DONE_MSK (0x80)
/** @brief Output Data Over Run and Data Availability. */
#define QMI86_REG_STATUS0 (0x2E)
#define QMI86_REG_STATUS0_ADA_POS (0x00)
#define QMI86_REG_STATUS0_ADA_MSK (0x01)

#define QMI86_REG_STATUS0_GDA_POS (0x01)
#define QMI86_REG_STATUS0_GDA_MSK (0x02)
/** @brief Miscellaneous Status: Any Motion, No Motion, Significant Motion,
 * Pedometer, Tap. */
#define QMI86_REG_STATUS1 (0x2F)

/** @brief 24 Bits Sample Time Stamp */
#define QMI86_REG_TIMESTAMP_LOW (0x30)
#define QMI86_REG_TIMESTAMP_MID (0x31)
#define QMI86_REG_TIMESTAMP_HIGH (0x32)

/** @brief 16 Bits Temperature Output Data */
#define QMI86_REG_TEMP_L (0x33)
#define QMI86_REG_TEMP_H (0x34)

/** @brief 16 Bits X-axis Acceleration */
#define QMI86_REG_AX_L (0x35)
#define QMI86_REG_AX_H (0x36)

/** @brief 16 Bits Y-axis Acceleration */
#define QMI86_REG_AY_L (0x37)
#define QMI86_REG_AY_H (0x38)

/** @brief 16 Bits Z-axis Acceleration */
#define QMI86_REG_AZ_L (0x39)
#define QMI86_REG_AZ_H (0x3A)

/** @brief 16 Bits X-axis Angular Rate */
#define QMI86_REG_GX_L (0x3B)
#define QMI86_REG_GX_H (0x3C)

/** @brief 16 Bits Y-axis Angular Rate */
#define QMI86_REG_GY_L (0x3D)
#define QMI86_REG_GY_H (0x3E)

/** @brief 16 Bits Z-axis Angular Rate */
#define QMI86_REG_GZ_L (0x3F)
#define QMI86_REG_GZ_H (0x40)

/** @brief Calibration-On-Demand status register */
#define QMI86_REG_COD_STATUS (0x46)
#define QMI86_REG_COD_STATUS_X_LIMIT_L_Fail_POS (0x07)
#define QMI86_REG_COD_STATUS_X_LIMIT_L_Fail_MSK (0x80)

#define QMI86_REG_COD_STATUS_X_LIMIT_H_Fail_POS (0x06)
#define QMI86_REG_COD_STATUS_X_LIMIT_H_Fail_MSK (0x40)

#define QMI86_REG_COD_STATUS_Y_LIMIT_L_Fail_POS (0x05)
#define QMI86_REG_COD_STATUS_Y_LIMIT_L_Fail_MSK (0x20)

#define QMI86_REG_COD_STATUS_Y_LIMIT_H_Fail_POS (0x04)
#define QMI86_REG_COD_STATUS_Y_LIMIT_H_Fail_MSK (0x10)

#define QMI86_REG_COD_STATUS_ACCEL_CHECK_POS (0x03)
#define QMI86_REG_COD_STATUS_ACCEL_CHECK_MSK (0x08)

#define QMI86_REG_COD_STATUS_STARTUP_FAILED_POS (0x02)
#define QMI86_REG_COD_STATUS_STARTUP_FAILED_MSK (0x04)

#define QMI86_REG_COD_STATUS_GYRO_ENABLED_POS (0x01)
#define QMI86_REG_COD_STATUS_GYRO_ENABLED_MSK (0x02)

#define QMI86_REG_COD_STATUS_COD_FAILED_POS (0x00)
#define QMI86_REG_COD_STATUS_COD_FAILED_MSK (0x01)

/** @brief 24 Bits FW Version */
#define QMI86_REG_DQW_L (0x49) // byte 0
#define QMI86_REG_DQW_H (0x4A) // byte 1
#define QMI86_REG_DQX_L (0x4B) // byte 2

/** @brief 48 Bits General purpose registers */
#define QMI86_REG_DVX_L (0x51) // byte 0
#define QMI86_REG_DVX_H (0x52) // byte 1
#define QMI86_REG_DVY_L (0x53) // byte 2
#define QMI86_REG_DVY_H (0x54) // byte 3
#define QMI86_REG_DVZ_L (0x55) // byte 4
#define QMI86_REG_DVZ_H (0x56) // byte 5

/** @brief Axis, direction, number of detected Tap */
#define QMI86_REG_TAP_STATUS (0x59)

/** @brief 24 Bits Step count of Pedometer */
#define QMI86_REG_STEP_CNT_LOW (0x5A)
#define QMI86_REG_STEP_CNT_MIDL (0x5B)
#define QMI86_REG_STEP_CNT_HIGH (0x5C)

/** @brief Soft Reset Register */
#define QMI86_REG_RESET (0x60)
#define QMI86_RESET_CMD (0xB0)

#define QMI86_REG_RESET_RESULT (0x4D)
#define QMI86_RESET_SUCCESSFUL (0x80)

#define QMI86_WHO_AM_I (0x05)
#define QMI86_CHIP_ID (0x7B)

#define QMI86_IIC_ADDR_1 (0x6A)
#define QMI86_IIC_ADDR_2 (0x6B)

#define QMI86_POWER_OFF_TIME (5U)

#define QMI86_SYS_POWER_ON_TIME (20U)
#define QMI86_GYRO_POWER_ON_TIME (160U)
#define QMI86_ACCEL_POWER_ON_TIME (10U)

#define QMI86_ST_ODR_CYCLE_CNT (25U)
#endif // DD_QMI86_DEFS_H
