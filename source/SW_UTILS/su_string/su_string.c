/***************************************************************************************************
 * Header files.
 ***************************************************************************************************/

#include "su_string.h"

/***************************************************************************************************
 * Macro definitions.
 ***************************************************************************************************/

#define MAX_STRING_LEN (1000U)

/***************************************************************************************************
 * Local type definitions.
 ***************************************************************************************************/

/***************************************************************************************************
 * Local data definitions.
 ***************************************************************************************************/

/***************************************************************************************************
 * Local function definitions.
 ***************************************************************************************************/

/***************************************************************************************************
 * External data definitions.
 ***************************************************************************************************/

/***************************************************************************************************
 * External function definitions.
 ***************************************************************************************************/

/**
 * @brief This function reverses a string in place.
 * @param[in,out] ppt_str Pointer to the string to be reversed.
 * @param[in] p_str_len Length of the string to be reversed.
 */
void string_reverse(char* ppt_str, uint32_t p_str_len)
{
    ASSERT_AND_RETURN(ppt_str == NULL, );

    uint32_t start = 0U;
    uint32_t end   = p_str_len - 1;
    char     temp  = (char)0;

    if (p_str_len > 1 && p_str_len <= MAX_STRING_LEN)
    {
        while (start < end)
        {
            temp           = ppt_str[start];
            ppt_str[start] = ppt_str[end];
            ppt_str[end]   = temp;
            start++;
            end--;
        }
    }
}

/**
 * @brief This function converts an integer to a string representation in the
 * specified base.
 * @param[in] p_num The integer number to convert.
 * @param[out] ppt_str Pointer to the output string buffer.
 * @param[in] p_digit Minimum number of digits in the output string.
 * @param[in] p_base The base for conversion.
 * @return The length of the resulting string.
 */
uint32_t string_itoa(int32_t p_num, char* ppt_str, uint32_t p_digit,
                     number_base_t p_base)
{
    ASSERT_AND_RETURN(ppt_str == NULL, 0U);

    uint32_t idx         = 0U;
    bool_t   is_negative = FALSE;
    uint32_t num_pos     = 0U;
    uint32_t remainder   = 0U;

    if (p_num == 0)
    {
        ppt_str[idx++] = '0';
        ppt_str[idx]   = '\0';
    }
    else
    {
        /// If the number is negative, we convert it to positive for processing
        /// and mark it as negative to append the minus sign at the end.
        if (p_num < 0 && p_base == NUMBER_BASE_DECIMAL)
        {
            is_negative = TRUE;
            p_num       = -p_num;
        }

        num_pos = (uint32_t)p_num;

        /**
         * @brief This loop converts the number to the specified base.
         * It extracts digits from the least significant to the most
         * significant. example: For p_num = 12 and p_base = 10, the conversion
         * steps are as follows:
         * 1. remainder = 12 % 10 = 2, ppt_str[0] = '2'
         * 2. num_pos = 12 / 10 = 1
         * 3. remainder = 1 % 10 = 1, ppt_str[1] = '1'
         * 4. num_pos = 1 / 10 = 0
         * The final string will be "12".
         */
        while (num_pos != 0)
        {
            remainder    = num_pos % p_base;
            ppt_str[idx] = (remainder > 9) ? (remainder - 10) + 'a'
                                           : remainder + '0'; // NOLINT
            idx++;
            num_pos = num_pos / p_base;
        }

        /// Append leading zeros until the string reaches the specified digit
        /// length
        while (idx < p_digit)
        {
            ppt_str[idx] = '0';
            idx++;
        }

        /// If the number is negative, append the minus sign
        if (is_negative)
        {
            ppt_str[idx] = '-';
            idx++;
        }

        string_reverse(ppt_str, idx);

        ppt_str[idx] = '\0';
    }

    return idx;
}

/**
 * @brief This function converts a floating-point number to a string
 * representation.
 * @param[in] p_fnum The floating-point number to convert.
 * @param[out] ppt_str Pointer to the output string buffer.
 * @param[in] p_after_point Number of digits after the decimal point.
 * @return The length of the resulting string.
 */
uint32_t string_ftoa(float p_fnum, char* ppt_str, uint32_t p_after_point)
{
    ASSERT_AND_RETURN(ppt_str == NULL, 0U);

    int32_t  i_part = 0;
    uint32_t idx    = 0U;
    float    f_part = 0.0F;

    /// Extract the integer part
    i_part = (int32_t)p_fnum;
    /// Conver the integer part to string
    idx            = string_itoa(i_part, ppt_str, 0, NUMBER_BASE_DECIMAL);
    ppt_str[idx++] = '.';
    /// Extract the fractional part
    f_part = p_fnum - (float)i_part;

    if (f_part < 0)
    {
        f_part = -f_part;
    }

    /// Appen the fractional part to string digit by digit
    for (uint32_t j = 0; j < p_after_point; j++)
    {
        f_part        *= 10; // NOLINT
        i_part         = (int32_t)f_part;
        ppt_str[idx++] = (char)(i_part + '0');
        f_part        -= (float)i_part;
    }

    ppt_str[idx] = '\0';

    return idx;
}
