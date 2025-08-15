#ifndef PS_LOGGER_H
#define PS_LOGGER_H

/***************************************************************************************************
 * Header files.
 ***************************************************************************************************/

#include "su_common.h"

/***************************************************************************************************
 * Macro definitions.
 ***************************************************************************************************/
#define LOGGER_ENABLED 1

#define LOGGER_MSG_MAX_LENGTH 1024

#ifdef LOGGER_ENABLED

#define LOG_INFO(p_msg) ps_logger_send(DBG_LVL_INFO, NULL, (p_msg), 0, 0, 0)
#define LOG_WARN(p_msg) ps_logger_send(DBG_LVL_WARN, NULL, (p_msg), 0, 0, 0)
#define LOG_ERR(p_msg) ps_logger_send(DBG_LVL_ERR, NULL, (p_msg), 0, 0, 0)
#define LOG_DEBUG(p_msg) ps_logger_send(DBG_LVL_DEBUG, __func__, (p_msg), 0, 0, 0)
#define LOG_PRDIC(p_msg) ps_logger_send(DBG_LVL_PERIODIC, NULL, (p_msg), 0, 0, 0)
/**
 * @brief This macro is used to extend the message.
 * Meaning that it will not change the color if enabled and will not print the
 * level of the message.
 *
 */
#define LOG_EXTEND(p_msg) ps_logger_send(DBG_LVL_EXT, NULL, (p_msg), 0, 0, 0)

#define LOG_INFO_P1(p_msg, p_param_1) ps_logger_send(DBG_LVL_INFO, NULL, (p_msg), (p_param_1), 0U, 0U)
#define LOG_WARN_P1(p_msg, p_param_1) ps_logger_send(DBG_LVL_WARN, NULL, (p_msg), (p_param_1), 0U, 0U)
#define LOG_ERR_P1(p_msg, p_param_1) ps_logger_send(DBG_LVL_ERR, NULL, (p_msg), (p_param_1), 0U, 0U)
#define LOG_DEBUG_P1(p_msg, p_param_1) ps_logger_send(DBG_LVL_DEBUG, __func__, (p_msg), (p_param_1), 0U, 0U)
#define LOG_PRDIC_P1(p_msg, p_param_1) ps_logger_send(DBG_LVL_PERIODIC, NULL, (p_msg), (p_param_1), 0U, 0U)
#define LOG_EXTEND_P1(p_msg, p_param_1) ps_logger_send(DBG_LVL_EXT, NULL, (p_msg), (p_param_1), 0U, 0U)

#define LOG_INFO_P2(p_msg, p_param_1, p_param_2) ps_logger_send(DBG_LVL_INFO, NULL, (p_msg), (p_param_1), (p_param_2), 0U)
#define LOG_WARN_P2(p_msg, p_param_1, p_param_2) ps_logger_send(DBG_LVL_WARN, NULL, (p_msg), (p_param_1), (p_param_2), 0U)
#define LOG_ERR_P2(p_msg, p_param_1, p_param_2) ps_logger_send(DBG_LVL_ERR, NULL, (p_msg), (p_param_1), (p_param_2), 0U)
#define LOG_DEBUG_P2(p_msg, p_param_1, p_param_2) ps_logger_send(DBG_LVL_DEBUG, __func__, (p_msg), (p_param_1), (p_param_2), 0U)
#define LOG_PRDIC_P2(p_msg, p_param_1, p_param_2) ps_logger_send(DBG_LVL_PERIODIC, NULL, (p_msg), (p_param_1), (p_param_2), 0U)
#define LOG_EXTEND_P2(p_msg, p_param_1, p_param_2) ps_logger_send(DBG_LVL_EXT, NULL, (p_msg), (p_param_1), (p_param_2), 0U)

#define LOG_INFO_P3(p_msg, p_param_1, p_param_2, p_param_3) ps_logger_send(DBG_LVL_INFO, NULL, (p_msg), (p_param_1), (p_param_2), p_param_3)
#define LOG_WARN_P3(p_msg, p_param_1, p_param_2, p_param_3) ps_logger_send(DBG_LVL_WARN, NULL, (p_msg), (p_param_1), (p_param_2), p_param_3)
#define LOG_ERR_P3(p_msg, p_param_1, p_param_2, p_param_3) ps_logger_send(DBG_LVL_ERR, NULL, (p_msg), (p_param_1), (p_param_2), p_param_3)
#define LOG_DEBUG_P3(p_msg, p_param_1, p_param_2, p_param_3) ps_logger_send(DBG_LVL_DEBUG, __func__, (p_msg), (p_param_1), (p_param_2), p_param_3)
#define LOG_PRDIC_P3(p_msg, p_param_1, p_param_2, p_param_3) ps_logger_send(DBG_LVL_PERIODIC, NULL, (p_msg), (p_param_1), (p_param_2), p_param_3)
#define LOG_EXTEND_P3(p_msg, p_param_1, p_param_2, p_param_3) ps_logger_send(DBG_LVL_EXT, NULL, (p_msg), (p_param_1), (p_param_2), p_param_3)

#else
#define LOG_INFO(p_msg) ((void)0)
#define LOG_WARN(p_msg) ((void)0)
#define LOG_ERR(p_msg) ((void)0)
#define LOG_DEBUG(p_msg) ((void)0)
#define LOG_PRDIC(p_msg) ((void)0)
#define LOG_EXTEND(p_msg) ((void)0)

#define LOG_INFO_P1(p_msg, p_param_1) ((void)0)
#define LOG_WARN_P1(p_msg, p_param_1) ((void)0)
#define LOG_ERR_P1(p_msg, p_param_1) ((void)0)
#define LOG_DEBUG_P1(p_msg, p_param_1) ((void)0)
#define LOG_PRDIC_P1(p_msg, p_param_1) ((void)0)
#define LOG_EXTEND_P1(p_msg, p_param_1) ((void)0)

#define LOG_INFO_P2(p_msg, p_param_1, p_param_2) ((void)0)
#define LOG_WARN_P2(p_msg, p_param_1, p_param_2) ((void)0)
#define LOG_ERR_P2(p_msg, p_param_1, p_param_2) ((void)0)
#define LOG_DEBUG_P2(p_msg, p_param_1, p_param_2) ((void)0)
#define LOG_PRDIC_P2(p_msg, p_param_1, p_param_2) ((void)0)
#define LOG_EXTEND_P2(p_msg, p_param_1, p_param_2) ((void)0)

#define LOG_INFO_P3(p_msg, p_param_1, p_param_2, p_param_3) ((void)0)
#define LOG_WARN_P3(p_msg, p_param_1, p_param_2, p_param_3) ((void)0)
#define LOG_ERR_P3(p_msg, p_param_1, p_param_2, p_param_3) ((void)0)
#define LOG_DEBUG_P3(p_msg, p_param_1, p_param_2, p_param_3) ((void)0)
#define LOG_PRDIC_P3(p_msg, p_param_1, p_param_2, p_param_3) ((void)0)
#define LOG_EXTEND_P3(p_msg, p_param_1, p_param_2, p_param_3) ((void)0)

#endif /* LOGGER_ENABLED */

/***************************************************************************************************
 * External type declarations.
 ***************************************************************************************************/

typedef enum en_debug_level
{
    DBG_LVL_EXT = 0,
    DBG_LVL_ERR,
    DBG_LVL_WARN,
    DBG_LVL_INFO,
    DBG_LVL_PERIODIC,
    DBG_LVL_DEBUG,
} debug_level_t;

/***************************************************************************************************
 * External data declarations.
 ***************************************************************************************************/

/***************************************************************************************************
 * External function declarations.
 ***************************************************************************************************/

response_status_t ps_logger_init(void);
void              ps_logger_set_threshold(debug_level_t p_lvl);

void ps_logger_send(debug_level_t p_lvl, const char* ppt_func_name, const char* ppt_msg,
                    float p_param_1, float p_param_2, float p_param_3);

#endif /* PS_LOGGER_H */
