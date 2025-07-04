

#ifndef SU_COMMON_H
#define SU_COMMON_H

#include "stddef.h"
#include "stdint.h"
#include "su_byte_utils.h"

#if !defined(TEST)
#define SW_BREAK() \
    do { \
        __asm__ __volatile__("bkpt #0\n\t" : : : "memory"); \
    } while (0);
#else
#define SW_BREAK() ;
#endif

#define ASSERT_AND_RETURN(expr, rv) { \
    if (expr) \
    { \
        SW_BREAK(); \
        return rv; \
    } }

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ARRAY_EQUAL_LENGTHS(a, b) \
    _Static_assert(ARRAY_SIZE(a) == ARRAY_SIZE(b), "Arrays must be same length")

typedef enum
{
    FALSE = 0x00,
    TRUE  = 0x01,
} bool_t;

typedef bool_t BOOL;

typedef enum
{
    RET_OK              = 0x00,
    RET_ERROR           = 0x01,
    RET_BUSY            = 0x02,
    RET_TIMEOUT         = 0x04,
    RET_PARAM_ERROR     = 0x08,
    RET_NOT_SUPPORTED   = 0x10,
    RET_NOT_INITIALIZED = 0x20,
    RET_NOT_FOUND       = 0x40,
} response_status_t;

typedef uint16_t timeout_t;
#endif /* SU_COMMON_H */
