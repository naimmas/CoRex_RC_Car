/**
 * \file            lwrb.c
 * \brief           Lightweight ring buffer
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwRB - Lightweight ring buffer library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v3.2.0
 */
#include "su_ring_buffer.h"

/* Memory set and copy functions */
#define BUF_MEMSET      memset
#define BUF_MEMCPY      memcpy

#define BUF_IS_VALID(b) ((b) != NULL && (b)->buff != NULL && (b)->size > 0)
#define BUF_MIN(x, y)   ((x) < (y) ? (x) : (y))
#define BUF_MAX(x, y)   ((x) > (y) ? (x) : (y))
#define BUF_SEND_EVT(b, type, bp)                                                                                      \
    do {                                                                                                               \
        if ((b)->evt_fn != NULL) {                                                                                     \
            (b)->evt_fn((void*)(b), (type), (bp));                                                                     \
        }                                                                                                              \
    } while (0)

/* Optional atomic opeartions */
#ifdef SU_RB_DISABLE_ATOMIC
#define SU_RB_INIT(var, val)        (var) = (val)
#define SU_RB_LOAD(var, type)       (var)
#define SU_RB_STORE(var, val, type) (var) = (val)
#else
#define SU_RB_INIT(var, val)        atomic_init(&(var), (val))
#define SU_RB_LOAD(var, type)       atomic_load_explicit(&(var), (type))
#define SU_RB_STORE(var, val, type) atomic_store_explicit(&(var), (val), (type))
#endif

/**
 * \brief           Initialize buffer handle to default values with size and buffer data array
 * \param[in]       buff: Ring buffer instance
 * \param[in]       buffdata: Pointer to memory to use as buffer data
 * \param[in]       size: Size of `buffdata` in units of bytes
 *                      Maximum number of bytes buffer can hold is `size - 1`
 * \return          `1` on success, `0` otherwise
 */
uint8_t su_rb_init(su_rb_t* ppt_buff, void* ppt_buffdata, su_rb_sz_t p_size)
{
    if (ppt_buff == NULL || ppt_buffdata == NULL || p_size == 0)
    {
        return 0;
    }

    ppt_buff->evt_fn = NULL;
    ppt_buff->size   = p_size;
    ppt_buff->buff   = ppt_buffdata;
    SU_RB_INIT(ppt_buff->w_ptr, 0);
    SU_RB_INIT(ppt_buff->r_ptr, 0);
    return 1;
}

/**
 * \brief           Check if buff is initialized and ready to use
 * \param[in]       buff: Ring buffer instance
 * \return          `1` if ready, `0` otherwise
 */
uint8_t su_rb_is_ready(su_rb_t* ppt_buff)
{
    return BUF_IS_VALID(ppt_buff);
}

/**
 * \brief           Free buffer memory
 * \note            Since implementation does not use dynamic allocation,
 *                  it just sets buffer handle to `NULL`
 * \param[in]       buff: Ring buffer instance
 */
void su_rb_free(su_rb_t* ppt_buff)
{
    if (BUF_IS_VALID(ppt_buff))
    {
        ppt_buff->buff = NULL;
    }
}

/**
 * \brief           Set event function callback for different buffer operations
 * \param[in]       buff: Ring buffer instance
 * \param[in]       evt_fn: Callback function
 */
void su_rb_set_evt_fn(su_rb_t* ppt_buff, su_rb_evt_fn ppt_evt_fn)
{
    if (BUF_IS_VALID(ppt_buff))
    {
        ppt_buff->evt_fn = ppt_evt_fn;
    }
}

/**
 * \brief           Set custom buffer argument, that can be retrieved in the event function
 * \param[in]       buff: Ring buffer instance
 * \param[in]       arg: Custom user argument
 */
void su_rb_set_arg(su_rb_t* ppt_buff, void* ppt_arg)
{
    if (BUF_IS_VALID(ppt_buff))
    {
        ppt_buff->arg = ppt_arg;
    }
}

/**
 * \brief           Get custom buffer argument, previously set with \ref su_rb_set_arg
 * \param[in]       buff: Ring buffer instance
 * \return          User argument, previously set with \ref su_rb_set_arg
 */
void* su_rb_get_arg(su_rb_t* ppt_buff)
{
    return ppt_buff != NULL ? ppt_buff->arg : NULL;
}

/**
 * \brief           Write data to buffer.
 *                  Copies data from `data` array to buffer and advances the write pointer for a
 * maximum of `btw` number of bytes.
 *
 *                  It copies less if there is less memory available in the buffer.
 *                  User must check the return value of the function and compare it to
 *                  the requested write length, to determine if everything has been written
 *
 * \note            Use \ref su_rb_write_ex for more advanced usage
 *
 * \param[in]       buff: Ring buffer instance
 * \param[in]       data: Pointer to data to write into buffer
 * \param[in]       btw: Number of bytes to write
 * \return          Number of bytes written to buffer.
 *                      When returned value is less than `btw`, there was no enough memory available
 *                      to copy full data array.
 */
su_rb_sz_t su_rb_write(su_rb_t* ppt_buff, const void* ppt_data, su_rb_sz_t p_btw)
{
    su_rb_sz_t written = 0;

    if (su_rb_write_ex(ppt_buff, ppt_data, p_btw, &written, 0))
    {
        return written;
    }
    return 0;
}

/**
 * \brief           Write extended functionality
 *
 * \param           buff: Ring buffer instance
 * \param           data: Pointer to data to write into buffer
 * \param           btw: Number of bytes to write
 * \param           bwritten: Output pointer to write number of bytes written into the buffer
 * \param           flags: Optional flags.
 *                      \ref SU_RB_FLAG_WRITE_ALL: Request to write all data (up to btw).
 *                          Will early return if no memory available
 * \return          `1` if write operation OK, `0` otherwise
 */
uint8_t su_rb_write_ex(su_rb_t* ppt_buff, const void* ppt_data, su_rb_sz_t p_btw, su_rb_sz_t* ppt_bwritten,
                       uint16_t p_flags)
{
    su_rb_sz_t     tocopy = 0;
    su_rb_sz_t     free = 0;
    su_rb_sz_t     w_ptr = 0;
    const uint8_t* pt_d_ptr = ppt_data;

    if (!BUF_IS_VALID(ppt_buff) || ppt_data == NULL || p_btw == 0)
    {
        return 0;
    }

    /* Calculate maximum number of bytes available to write */
    free = su_rb_get_free(ppt_buff);
    /* If no memory, or if user wants to write ALL data but no enough space, exit early */
    if (free == 0 || (free < p_btw && (p_flags & SU_RB_FLAG_WRITE_ALL)))
    {
        return 0;
    }
    p_btw   = BUF_MIN(free, p_btw);
    w_ptr = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_acquire);

    /* Step 1: Write data to linear part of buffer */
    tocopy = BUF_MIN(ppt_buff->size - w_ptr, p_btw);
    BUF_MEMCPY(&ppt_buff->buff[w_ptr], pt_d_ptr, tocopy);
    pt_d_ptr += tocopy;
    w_ptr += tocopy;
    p_btw   -= tocopy;

    /* Step 2: Write data to beginning of buffer (overflow part) */
    if (p_btw > 0)
    {
        BUF_MEMCPY(ppt_buff->buff, pt_d_ptr, p_btw);
        w_ptr = p_btw;
    }

    /* Step 3: Check end of buffer */
    if (w_ptr >= ppt_buff->size)
    {
        w_ptr = 0;
    }

    /*
     * Write final value to the actual running variable.
     * This is to ensure no read operation can access intermediate data
     */
    SU_RB_STORE(ppt_buff->w_ptr, w_ptr, memory_order_release);

    BUF_SEND_EVT(ppt_buff, SU_RB_EVT_WRITE, tocopy + p_btw);
    if (ppt_bwritten != NULL)
    {
        *ppt_bwritten = tocopy + p_btw;
    }
    return 1;
}

/**
 * \brief           Read data from buffer.
 *                  Copies data from `data` array to buffer and advances the read pointer for a
 * maximum of `btr` number of bytes.
 *
 *                  It copies less if there is less data available in the buffer.
 *
 * \note            Use \ref su_rb_read_ex for more advanced usage
 *
 * \param[in]       buff: Ring buffer instance
 * \param[out]      data: Pointer to output memory to copy buffer data to
 * \param[in]       btr: Number of bytes to read
 * \return          Number of bytes read and copied to data array
 */
su_rb_sz_t su_rb_read(su_rb_t* ppt_buff, void* ppt_data, su_rb_sz_t p_btr)
{
    su_rb_sz_t read = 0;

    if (su_rb_read_ex(ppt_buff, ppt_data, p_btr, &read, 0))
    {
        return read;
    }
    return 0;
}

/**
 * \brief           Read extended functionality
 *
 * \param           buff: Ring buffer instance
 * \param           data: Pointer to memory to write read data from buffer
 * \param           btr: Number of bytes to read
 * \param           bread: Output pointer to write number of bytes read from buffer and written to
 * the output `data` variable
 * \param           flags: Optional flags
 *                      \ref SU_RB_FLAG_READ_ALL: Request to read all data (up to btr).
 *                          Will early return if no enough bytes in the buffer
 * \return          `1` if read operation OK, `0` otherwise
 */
uint8_t su_rb_read_ex(su_rb_t* ppt_buff, void* ppt_data, su_rb_sz_t p_btr, su_rb_sz_t* ppt_bread, uint16_t p_flags)
{
    su_rb_sz_t tocopy = 0;
    su_rb_sz_t full = 0;
    su_rb_sz_t r_ptr = 0;
    uint8_t*   pt_d_ptr = ppt_data;

    if (!BUF_IS_VALID(ppt_buff) || ppt_data == NULL || p_btr == 0)
    {
        return 0;
    }

    /* Calculate maximum number of bytes available to read */
    full = su_rb_get_full(ppt_buff);
    if (full == 0 || (full < p_btr && (p_flags & SU_RB_FLAG_READ_ALL)))
    {
        return 0;
    }
    p_btr   = BUF_MIN(full, p_btr);
    r_ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_acquire);

    /* Step 1: Read data from linear part of buffer */
    tocopy = BUF_MIN(ppt_buff->size - r_ptr, p_btr);
    BUF_MEMCPY(pt_d_ptr, &ppt_buff->buff[r_ptr], tocopy);
    pt_d_ptr += tocopy;
    r_ptr += tocopy;
    p_btr   -= tocopy;

    /* Step 2: Read data from beginning of buffer (overflow part) */
    if (p_btr > 0)
    {
        BUF_MEMCPY(pt_d_ptr, ppt_buff->buff, p_btr);
        r_ptr = p_btr;
    }

    /* Step 3: Check end of buffer */
    if (r_ptr >= ppt_buff->size)
    {
        r_ptr = 0;
    }

    /*
     * Write final value to the actual running variable.
     * This is to ensure no write operation can access intermediate data
     */
    SU_RB_STORE(ppt_buff->r_ptr, r_ptr, memory_order_release);

    BUF_SEND_EVT(ppt_buff, SU_RB_EVT_READ, tocopy + p_btr);
    if (ppt_bread != NULL)
    {
        *ppt_bread = tocopy + p_btr;
    }
    return 1;
}

/**
 * \brief           Read from buffer without changing read pointer (peek only)
 * \param[in]       buff: Ring buffer instance
 * \param[in]       skip_count: Number of bytes to skip before reading data
 * \param[out]      data: Pointer to output memory to copy buffer data to
 * \param[in]       btp: Number of bytes to peek
 * \return          Number of bytes peeked and written to output array
 */
su_rb_sz_t su_rb_peek(const su_rb_t* ppt_buff, su_rb_sz_t p_skip_count, void* ppt_data, su_rb_sz_t p_btp)
{
    su_rb_sz_t full = 0;
    su_rb_sz_t tocopy = 0;
    su_rb_sz_t r_ptr = 0;
    uint8_t*   pt_d_ptr = ppt_data;

    if (!BUF_IS_VALID(ppt_buff) || ppt_data == NULL || p_btp == 0)
    {
        return 0;
    }

    /*
     * Calculate maximum number of bytes available to read
     * and check if we can even fit to it
     */
    full = su_rb_get_full(ppt_buff);
    if (p_skip_count >= full)
    {
        return 0;
    }
    r_ptr  = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);
    r_ptr += p_skip_count;
    full  -= p_skip_count;
    if (r_ptr >= ppt_buff->size)
    {
        r_ptr -= ppt_buff->size;
    }

    /* Check maximum number of bytes available to read after skip */
    p_btp = BUF_MIN(full, p_btp);
    if (p_btp == 0)
    {
        return 0;
    }

    /* Step 1: Read data from linear part of buffer */
    tocopy = BUF_MIN(ppt_buff->size - r_ptr, p_btp);
    BUF_MEMCPY(pt_d_ptr, &ppt_buff->buff[r_ptr], tocopy);
    pt_d_ptr += tocopy;
    p_btp   -= tocopy;

    /* Step 2: Read data from beginning of buffer (overflow part) */
    if (p_btp > 0)
    {
        BUF_MEMCPY(pt_d_ptr, ppt_buff->buff, p_btp);
    }
    return tocopy + p_btp;
}

/**
 * \brief           Get available size in buffer for write operation
 * \param[in]       buff: Ring buffer instance
 * \return          Number of free bytes in memory
 */
su_rb_sz_t su_rb_get_free(const su_rb_t* ppt_buff)
{
    su_rb_sz_t size = 0;
    su_rb_sz_t w_ptr = 0;
    su_rb_sz_t r_ptr = 0;

    if (!BUF_IS_VALID(ppt_buff))
    {
        return 0;
    }

    /*
     * Copy buffer pointers to local variables with atomic access.
     *
     * To ensure thread safety (only when in single-entry, single-exit FIFO mode use case),
     * it is important to write buffer r and w values to local w and r variables.
     *
     * Local variables will ensure below if statements will always use the same value,
     * even if buff->w or buff->r get changed during interrupt processing.
     *
     * They may change during load operation, important is that
     * they do not change during if-else operations following these assignments.
     *
     * su_rb_get_free is only called for write purpose, and when in FIFO mode, then:
     * - buff->w pointer will not change by another process/interrupt because we are in write mode
     * just now
     * - buff->r pointer may change by another process. If it gets changed after buff->r has been
     * loaded to local variable, buffer will see "free size" less than it actually is. This is not a
     * problem, application can always try again to write more data to remaining free memory that
     * was read just during copy operation
     */
    w_ptr = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_relaxed);
    r_ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);

    if (w_ptr >= r_ptr)
    {
        size = ppt_buff->size - (w_ptr - r_ptr);
    }
    else
    {
        size = r_ptr - w_ptr;
    }

    /* Buffer free size is always 1 less than actual size */
    return size - 1;
}

/**
 * \brief           Get number of bytes currently available in buffer
 * \param[in]       buff: Ring buffer instance
 * \return          Number of bytes ready to be read
 */
su_rb_sz_t su_rb_get_full(const su_rb_t* ppt_buff)
{
    su_rb_sz_t size = 0;
    su_rb_sz_t w_ptr = 0;
    su_rb_sz_t r_ptr = 0;

    if (!BUF_IS_VALID(ppt_buff))
    {
        return 0;
    }

    /*
     * Copy buffer pointers to local variables.
     *
     * To ensure thread safety (only when in single-entry, single-exit FIFO mode use case),
     * it is important to write buffer r and w values to local w and r variables.
     *
     * Local variables will ensure below if statements will always use the same value,
     * even if buff->w or buff->r get changed during interrupt processing.
     *
     * They may change during load operation, important is that
     * they do not change during if-else operations following these assignments.
     *
     * su_rb_get_full is only called for read purpose, and when in FIFO mode, then:
     * - buff->r pointer will not change by another process/interrupt because we are in read mode
     * just now
     * - buff->w pointer may change by another process. If it gets changed after buff->w has been
     * loaded to local variable, buffer will see "full size" less than it really is. This is not a
     * problem, application can always try again to read more data from remaining full memory that
     * was written just during copy operation
     */
    w_ptr = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_relaxed);
    r_ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);

    if (w_ptr >= r_ptr)
    {
        size = w_ptr - r_ptr;
    }
    else
    {
        size = ppt_buff->size - (r_ptr - w_ptr);
    }
    return size;
}

/**
 * \brief           Resets buffer to default values. Buffer size is not modified
 * \note            This function is not thread safe.
 *                      When used, application must ensure there is no active read/write operation
 * \param[in]       buff: Ring buffer instance
 */
void su_rb_reset(su_rb_t* ppt_buff)
{
    if (BUF_IS_VALID(ppt_buff))
    {
        SU_RB_STORE(ppt_buff->w_ptr, 0, memory_order_release);
        SU_RB_STORE(ppt_buff->r_ptr, 0, memory_order_release);
        BUF_SEND_EVT(ppt_buff, SU_RB_EVT_RESET, 0);
    }
}

/**
 * \brief           Get linear address for buffer for fast read
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer start address
 */
void* su_rb_get_linear_block_read_address(const su_rb_t* ppt_buff)
{
    su_rb_sz_t ptr = 0;

    if (!BUF_IS_VALID(ppt_buff))
    {
        return NULL;
    }
    ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);
    return &ppt_buff->buff[ptr];
}

/**
 * \brief           Get length of linear block address before it overflows for read operation
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer size in units of bytes for read operation
 */
su_rb_sz_t su_rb_get_linear_block_read_length(const su_rb_t* ppt_buff)
{
    su_rb_sz_t len = 0;
    su_rb_sz_t w_ptr = 0;
    su_rb_sz_t r_ptr = 0;

    if (!BUF_IS_VALID(ppt_buff))
    {
        return 0;
    }

    /*
     * Use temporary values in case they are changed during operations.
     * See su_rb_buff_free or su_rb_buff_full functions for more information why this is OK.
     */
    w_ptr = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_relaxed);
    r_ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);

    if (w_ptr > r_ptr)
    {
        len = w_ptr - r_ptr;
    }
    else if (r_ptr > w_ptr)
    {
        len = ppt_buff->size - r_ptr;
    }
    else
    {
        len = 0;
    }
    return len;
}

/**
 * \brief           Skip (ignore; advance read pointer) buffer data
 * Marks data as read in the buffer and increases free memory for up to `len` bytes
 *
 * \note            Useful at the end of streaming transfer such as DMA
 * \param[in]       buff: Ring buffer instance
 * \param[in]       len: Number of bytes to skip and mark as read
 * \return          Number of bytes skipped
 */
su_rb_sz_t su_rb_skip(su_rb_t* ppt_buff, su_rb_sz_t p_len)
{
    su_rb_sz_t full = 0;
    su_rb_sz_t r_ptr = 0;

    if (!BUF_IS_VALID(ppt_buff) || p_len == 0)
    {
        return 0;
    }

    full   = su_rb_get_full(ppt_buff);
    p_len    = BUF_MIN(p_len, full);
    r_ptr  = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_acquire);
    r_ptr += p_len;
    if (r_ptr >= ppt_buff->size)
    {
        r_ptr -= ppt_buff->size;
    }
    SU_RB_STORE(ppt_buff->r_ptr, r_ptr, memory_order_release);
    BUF_SEND_EVT(ppt_buff, SU_RB_EVT_READ, p_len);
    return p_len;
}

/**
 * \brief           Get linear address for buffer for fast write
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer start address
 */
void* su_rb_get_linear_block_write_address(const su_rb_t* ppt_buff)
{
    su_rb_sz_t ptr = 0;

    if (!BUF_IS_VALID(ppt_buff))
    {
        return NULL;
    }
    ptr = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_relaxed);
    return &ppt_buff->buff[ptr];
}

/**
 * \brief           Get length of linear block address before it overflows for write operation
 * \param[in]       buff: Ring buffer instance
 * \return          Linear buffer size in units of bytes for write operation
 */
su_rb_sz_t su_rb_get_linear_block_write_length(const su_rb_t* ppt_buff)
{
    su_rb_sz_t len = 0;
    su_rb_sz_t w_ptr = 0;
    su_rb_sz_t r_ptr = 0;

    if (!BUF_IS_VALID(ppt_buff))
    {
        return 0;
    }

    /*
     * Use temporary values in case they are changed during operations.
     * See su_rb_buff_free or su_rb_buff_full functions for more information why this is OK.
     */
    w_ptr = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_relaxed);
    r_ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);

    if (w_ptr >= r_ptr)
    {
        len = ppt_buff->size - w_ptr;
        /*
         * When read pointer is 0,
         * maximal length is one less as if too many bytes
         * are written, buffer would be considered empty again (r == w)
         */
        if (r_ptr == 0)
        {
            /*
             * Cannot overflow:
             * - If r is not 0, statement does not get called
             * - buff->size cannot be 0 and if r is 0, len is greater 0
             */
            --len;
        }
    }
    else
    {
        len = r_ptr - w_ptr - 1;
    }
    return len;
}

/**
 * \brief           Advance write pointer in the buffer.
 * Similar to skip function but modifies write pointer instead of read
 *
 * \note            Useful when hardware is writing to buffer and application needs to increase
 * number of bytes written to buffer by hardware
 * \param[in]       buff: Ring buffer instance
 * \param[in]       len: Number of bytes to advance
 * \return          Number of bytes advanced for write operation
 */
su_rb_sz_t su_rb_advance(su_rb_t* ppt_buff, su_rb_sz_t p_len)
{
    su_rb_sz_t free = 0;
    su_rb_sz_t w_ptr = 0;

    if (!BUF_IS_VALID(ppt_buff) || p_len == 0)
    {
        return 0;
    }

    /* Use local variables before writing back to main structure */
    free   = su_rb_get_free(ppt_buff);
    p_len    = BUF_MIN(p_len, free);
    w_ptr  = SU_RB_LOAD(ppt_buff->w_ptr, memory_order_acquire);
    w_ptr += p_len;
    if (w_ptr >= ppt_buff->size)
    {
        w_ptr -= ppt_buff->size;
    }
    SU_RB_STORE(ppt_buff->w_ptr, w_ptr, memory_order_release);
    BUF_SEND_EVT(ppt_buff, SU_RB_EVT_WRITE, p_len);
    return p_len;
}

/**
 * \brief           Searches for a *needle* in an array, starting from given offset.
 *
 * \note            This function is not thread-safe.
 *
 * \param           buff: Ring buffer to search for needle in
 * \param           bts: Constant byte array sequence to search for in a buffer
 * \param           len: Length of the \arg bts array
 * \param           start_offset: Start offset in the buffer
 * \param           found_idx: Pointer to variable to write index in array where bts has been found
 *                      Must not be set to `NULL`
 * \return          `1` if \arg bts found, `0` otherwise
 */
uint8_t su_rb_find(const su_rb_t* ppt_buff, const void* ppt_bts, su_rb_sz_t p_len, su_rb_sz_t p_start_offset,
                   su_rb_sz_t* ppt_found_idx)
{
    su_rb_sz_t     full = 0;
    su_rb_sz_t     r_ptr = 0;
    su_rb_sz_t     buff_r_ptr = 0;
    su_rb_sz_t     max_x = 0;
    uint8_t        found  = 0;
    const uint8_t* pt_needle = ppt_bts;

    if (!BUF_IS_VALID(ppt_buff) || pt_needle == NULL || p_len == 0 || ppt_found_idx == NULL)
    {
        return 0;
    }
    *ppt_found_idx = 0;

    full = su_rb_get_full(ppt_buff);
    /* Verify initial conditions */
    if (full < (p_len + p_start_offset))
    {
        return 0;
    }

    /* Get actual buffer read pointer for this search */
    buff_r_ptr = SU_RB_LOAD(ppt_buff->r_ptr, memory_order_relaxed);

    /* Max number of for loops is buff_full - input_len - start_offset of buffer length */
    max_x = full - p_len;
    for (su_rb_sz_t skip_x = p_start_offset; !found && skip_x <= max_x; ++skip_x)
    {
        found = 1; /* Found by default */

        /* Prepare the starting point for reading */
        r_ptr = buff_r_ptr + skip_x;
        if (r_ptr >= ppt_buff->size)
        {
            r_ptr -= ppt_buff->size;
        }

        /* Search in the buffer */
        for (su_rb_sz_t idx = 0; idx < p_len; ++idx)
        {
            if (ppt_buff->buff[r_ptr] != pt_needle[idx])
            {
                found = 0;
                break;
            }
            if (++r_ptr >= ppt_buff->size)
            {
                r_ptr = 0;
            }
        }
        if (found)
        {
            *ppt_found_idx = skip_x;
        }
    }
    return found;
}
