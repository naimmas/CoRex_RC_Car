#ifndef SU_BYTE_UTILS_H
#define SU_BYTE_UTILS_H

/* Single-bit helpers */
#define BIT_GET(n)          (1u << (n))
#define BIT_SET(v, n)    ((v) |=  BIT_GET(n))
#define BIT_CLR(v, n)    ((v) &= ~BIT_GET(n))
#define BIT_TOGGLE(v, n) ((v) ^=  BIT_GET(n))

/* Extract bytes */
#define BYTE_LOW(x)    ((uint8_t)((x) & 0xFF))
#define BYTE_HIGH(x)   ((uint8_t)(((x) >> 8) & 0xFF))
#define BYTE_N(x, n)    ((uint8_t)(((x) >> (8*(n))) & 0xFF))

/* Combine bytes */
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define BYTES_TO_WORD(sign, lo, hi)           ((sign short)(BYTE_LOW(lo) | ((sign short)BYTE_LOW(hi) << 8))) //NOLINT

// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define BYTES_TO_DWORD(sign, b0, b1, b2, b3)    ((sign int)(BYTE_LOW(b0) | ((sign int)BYTE_LOW(b1) << 8) | \
                                    ((sign int)BYTE_LOW(b2) << 16) | ((sign int)BYTE_LOW(b3) << 24))) //NOLINT

/*  byte-swap */
#define BYTE_SWAP16(x)  ((uint16_t)(((x) >> 8) | ((x) << 8)))
#define BYTE_SWAP32(x)  ((uint32_t)(((x) >> 24) | (((x) & 0x00FF0000) >> 8) | \
                                 (((x) & 0x0000FF00) << 8)  | ((x) << 24)))
#define BYTE_SWAP64(x)  ((uint64_t)(((uint64_t)BYTE_SWAP32((uint32_t)(x)) << 32) | BYTE_SWAP32((uint32_t)((x) >> 32))))

#endif // SU_BYTE_UTILS_H
