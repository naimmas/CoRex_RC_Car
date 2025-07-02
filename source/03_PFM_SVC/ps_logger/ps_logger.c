/***************************************************************************************************
 * Header files.
 ***************************************************************************************************/

#include "ps_logger.h"

#include "ha_uart/ha_uart.h"
#include "string.h"
#include "su_string/su_string.h"
#include "stdio.h"

/***************************************************************************************************
 * Macro definitions.
 ***************************************************************************************************/

#define MAX_PARAMETER_COUNT (3U)
#define DEFAUL_UART_SEND_TIMEOUT (1000U)
#define FLOAT_NUMBER_PRECISION (3U)
#define LOGGER_UART_PORT (UART_PORT1)

#ifdef LOGGER_USE_COLOR

#define DBG_LOG_COLOR_E "\033[0;31m[ERROR]\033[0m\t" // RED
#define DBG_LOG_COLOR_W "\033[0;33m[ WARN]\033[0m\t" // YELLOW
#define DBG_LOG_COLOR_I "\033[0;32m[ INFO]\033[0m\t" // GREEN
#define DBG_LOG_COLOR_D "\033[0;36m[DEBUG]\033[0m\t" // CYAN
#define DBG_LOG_COLOR_P "\033[0;35m[PRDIC]\033[0m\t" // Mageenta

#define DBG_LOG_FUNC "\033[1m[FUNC: "
#define DBG_LOG_RESET "]\033[0m ->"

#else

#define DBG_LOG_COLOR_E "[ERROR]\t"
#define DBG_LOG_COLOR_W "[ WARN]\t"
#define DBG_LOG_COLOR_I "[ INFO]\t"
#define DBG_LOG_COLOR_D "[DEBUG]\t"
#define DBG_LOG_COLOR_P "[PRDIC]\t"

#define DBG_LOG_FUNC "[FUNC: "
#define DBG_LOG_RESET "] ->"

#endif

/***************************************************************************************************
 * Local type definitions.
 ***************************************************************************************************/

/***************************************************************************************************
 * Local data definitions.
 ***************************************************************************************************/

static char          g_debug_msg[MAX_DBG_MSG_LEN] = { '\0' };
static debug_level_t g_debug_thld                 = DBG_LVL_DEBUG;

/***************************************************************************************************
 * Local function definitions.
 ***************************************************************************************************/

/**
 * @brief This function looks for the first occurrence of a valid qualifier (%f,
 * %d, %x) and return its index.
 * @param[in] ppt_msg string buffer to search for the qualifier.
 * @return The index of the first valid qualifier in the string, or 0 if no
 * valid qualifier is found.
 * @note The function assumes that the first character of the string is '%'. If
 * it is not, the function returns 0.
 */
static uint16_t get_param_qualifier(const char* ppt_msg)
{
    const char qualifiers[] = "fdx";
    uint16_t   msg_len      = 0U;

    if (ppt_msg[0] == '%')
    {
        /// Iterate through the message to find the first valid qualifier
        for (uint8_t i = 1;
             ((ppt_msg[i] != '\0' && msg_len == 0) && ppt_msg[i] != ' ');
             i++)
        {
            if (ppt_msg[i] == qualifiers[0] || ppt_msg[i] == qualifiers[1]
                || ppt_msg[i] == qualifiers[2])
            {
                msg_len = i;
                break;
            }
        }
    }
    return msg_len;
}

static uint16_t add_function_name(const char* ppt_func_name, char* ppt_msg)
{
    uint16_t bytes_written = 0U;
    uint16_t func_name_len = strlen(ppt_func_name);

    memcpy(&ppt_msg[bytes_written], DBG_LOG_FUNC, sizeof(DBG_LOG_FUNC) - 1);

    bytes_written = sizeof(DBG_LOG_FUNC) - 1;

    for (uint16_t i = 0; i < func_name_len; i++)
    {
        ppt_msg[bytes_written++] = ppt_func_name[i];
    }

    memcpy(&ppt_msg[bytes_written], DBG_LOG_RESET, sizeof(DBG_LOG_RESET) - 1);

    bytes_written += sizeof(DBG_LOG_RESET) - 1;

    return bytes_written;
}

/**
 * @brief This function go through the message and replaces the parameter
 * qualifiers (%f, %d, %x) with the corresponding parameter values.
 * @param[in] ppt_msg Pointer to the message string.
 * @param[in,out] ppt_bytes_written Pointer to the variable that will hold the
 * number of bytes written to `g_debug_msg`.
 * @param[in] ppt_params_list Pointer to the list of parameters to replace in
 * the message.
 */
static void process_message(const char* ppt_msg, uint16_t* ppt_bytes_written,
                            const float* ppt_params_list)
{
    uint16_t qualifier_idx = 0U;
    uint8_t  param_count   = 0U;
    float    current_param = 0.0F;

    /// Iterate through the message
    for (uint16_t i = 0; (i < MAX_DBG_MSG_LEN) && (ppt_msg[i] != '\0'); i++)
    {
        /// If the current character is a parameter qualifier
        if (ppt_msg[i] == '%' && param_count < MAX_PARAMETER_COUNT)
        {
            current_param = ppt_params_list[param_count++];
            qualifier_idx = get_param_qualifier(&ppt_msg[i]);

            /// If a valid qualifier is found
            if (qualifier_idx > 0)
            {
                switch (ppt_msg[i + qualifier_idx])
                {
                    case 'f':
                        qualifier_idx =
                          string_ftoa(current_param,
                                      &g_debug_msg[*ppt_bytes_written],
                                      FLOAT_NUMBER_PRECISION);
                        break;
                    case 'd':
                        qualifier_idx =
                          string_itoa((int32_t)current_param,
                                      &g_debug_msg[*ppt_bytes_written],
                                      0,
                                      NUMBER_BASE_DECIMAL);
                        break;
                    case 'x':
                        qualifier_idx =
                          string_itoa((int32_t)current_param,
                                      &g_debug_msg[*ppt_bytes_written],
                                      0,
                                      NUMBER_BASE_HEX);
                        break;
                    default:
                        qualifier_idx = 0U; // Unknown qualifier, do not write
                        break;
                }

                /// Update the bytes written count
                *ppt_bytes_written += qualifier_idx;

                /// Move the index to the next character after the qualifier
                i++;
            }
            else
            {
                // Do nothing
            }
        }
        else
        {
            /// If the current character is not a parameter qualifier, just copy
            g_debug_msg[(*ppt_bytes_written)++] = ppt_msg[i];
        }
    }
}

/**
 * @brief This function adds the log level prefix and function name to the debug
 * message.
 * @param p_lvl Log level
 * @param ppt_func_name Optional function name to include in the message. Can be
 * NULL.
 * @param ppt_bytes_written Pointer to the variable that will hold the number of
 * bytes written to `g_debug_msg`.
 */
static void add_log_prefix(uint8_t p_lvl, const char* ppt_func_name,
                           uint16_t* ppt_bytes_written)
{
    const char* log_strings[] = { "",
                                  DBG_LOG_COLOR_E,
                                  DBG_LOG_COLOR_W,
                                  DBG_LOG_COLOR_I,
                                  DBG_LOG_COLOR_P,
                                  DBG_LOG_COLOR_D };

    /// Copy the log level prefix to the debug message
    strlcpy(&g_debug_msg[*ppt_bytes_written],
            log_strings[p_lvl],
            MAX_DBG_MSG_LEN);

    *ppt_bytes_written = strlen(log_strings[p_lvl]);

    if (ppt_func_name != NULL)
    {
        *ppt_bytes_written +=
          add_function_name(ppt_func_name, &g_debug_msg[*ppt_bytes_written]);
    }
}

/***************************************************************************************************
 * External data definitions.
 ***************************************************************************************************/

/***************************************************************************************************
 * External function definitions.
 ***************************************************************************************************/

/**
 * @brief This function initializes the logger module. It initializes the UART
 * interface and prints test message.
 *
 */
void ps_logger_init(void)
{
#ifdef LOGGER_ENABLED
    ha_uart_init();
    LOG_INFO("logger is ready\n");
#endif /* LOGGER_ENABLED */
}

/**
 * @brief This function sets the debug level threshold for the logger.
 *
 * @param p_lvl The debug level to set as the threshold. If DBG_LVL_EXT is
 * passed, the threshold will not be changed.
 */
void ps_logger_set_threshold(debug_level_t p_lvl)
{
    if (p_lvl != DBG_LVL_EXT)
    {
        g_debug_thld = p_lvl;
    }
}

/**
 * @brief This function prints the log message with the specified log level.
 * Supported qualifiers are %d, %f and %x . If the format contains no
 * qualifiers, the parameters are ignored.
 *
 * @param p_lvl Log level
 * @param[in] ppt_func_name Optional function name to include in the message.
 * Can be NULL.
 * @param[in] ppt_msg       Log message
 * @param[in] p_param_1     if format contains %d, %f or %x the value of the
 * first parameter. 0 otherwise
 * @param[in] p_param_2     if format contains %d, %f or %x the value of the
 * second parameter. 0 otherwise
 * @param[in] p_param_3     if format contains %d, %f or %x the value of the
 * third parameter. 0 otherwise
 */
void ps_logger_send(debug_level_t p_lvl, const char* ppt_func_name,
                    const char* ppt_msg, float p_param_1, float p_param_2,
                    float p_param_3)
{
    uint16_t    bytes_written                      = 0U;
    const float p_params_list[MAX_PARAMETER_COUNT] = { p_param_1,
                                                       p_param_2,
                                                       p_param_3 };

    /// Filter the log level based on the threshold
    if (p_lvl <= g_debug_thld)
    {
        memset(g_debug_msg, '\0', MAX_DBG_MSG_LEN);

        /// If the message is only a newline character, just send it
        if (ppt_msg[0] == '\n' && ppt_msg[1] == '\0')
        {
            g_debug_msg[bytes_written++] = '\n';
        }
        else
        {
            if (ppt_msg[0] == '\n')
            {
                g_debug_msg[bytes_written++] = '\n';
            }

            add_log_prefix(p_lvl, ppt_func_name, &bytes_written);
            process_message(ppt_msg, &bytes_written, (float*)p_params_list);
        }
#if (LOGGER_OUTPUT_CHANNEL == LOGGER_CHNL_UART)
        ha_uart_transmit(LOGGER_UART_PORT,
                         (uint8_t*)g_debug_msg,
                         bytes_written,
                         DEFAUL_UART_SEND_TIMEOUT);
#elif (LOGGER_OUTPUT_CHANNEL == LOGGER_CHNL_DEBUG)                         
        printf("%s", g_debug_msg);
#else
#error "Define logger channel"
#endif
    }
}
