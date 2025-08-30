/**
 * \file            lwrb.h
 * \brief           LwRB - Lightweight ring buffer
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
#ifndef SU_RB_HDR_H
#define SU_RB_HDR_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define SU_RB_DISABLE_ATOMIC
    /**
     * \defgroup        LWRB Lightweight ring buffer manager
     * \brief           Lightweight ring buffer manager
     * \{
     */

#if !defined(SU_RB_DISABLE_ATOMIC) || __DOXYGEN__
#include <stdatomic.h>

    /**
     * \brief           Atomic type for size variable.
     * Default value is set to be `unsigned 32-bits` type
     */
    typedef atomic_ulong su_rb_sz_atomic_t;

    /**
     * \brief           Size variable for all library operations.
     * Default value is set to be `unsigned 32-bits` type
     */
    typedef unsigned long su_rb_sz_t;
#else
typedef uint32_t su_rb_sz_atomic_t;
typedef uint32_t su_rb_sz_t;
#endif

    /**
     * \brief           Event type for buffer operations
     */
    typedef enum
    {
        SU_RB_EVT_READ,  /*!< Read event */
        SU_RB_EVT_WRITE, /*!< Write event */
        SU_RB_EVT_RESET, /*!< Reset event */
    } su_rb_evt_type_t;

    /**
     * \brief           Buffer structure forward declaration
     */
    struct st_lwrb;

    /**
     * \brief           Event callback function type
     * \param[in]       buff: Buffer handle for event
     * \param[in]       evt: Event type
     * \param[in]       bp: Number of bytes written or read (when used), depends on event type
     */
    typedef void (*su_rb_evt_fn)(struct st_lwrb* ppt_buff, su_rb_evt_type_t p_evt, su_rb_sz_t p_bp);

/* List of flags */
#define SU_RB_FLAG_READ_ALL  ((uint16_t)0x0001)
#define SU_RB_FLAG_WRITE_ALL ((uint16_t)0x0001)

    /**
     * \brief           Buffer structure
     */
    typedef struct st_lwrb
    {
        uint8_t* buff; /*!< Pointer to buffer data. Buffer is considered initialized when `buff !=
                          NULL` and `size > 0` */
        su_rb_sz_t
          size; /*!< Size of buffer data. Size of actual buffer is `1` byte less than value holds */
        su_rb_sz_atomic_t
          r_ptr; /*!< Next read pointer.
                    Buffer is considered empty when `r == w` and full when `w == r - 1` */
        su_rb_sz_atomic_t
          w_ptr;             /*!< Next write pointer.
                                Buffer is considered empty when `r == w` and full when `w == r - 1` */
        su_rb_evt_fn evt_fn; /*!< Pointer to event callback function */
        void*        arg;    /*!< Event custom user argument */
    } su_rb_t;

    uint8_t su_rb_init(su_rb_t* ppt_buff, void* ppt_buffdata, su_rb_sz_t p_size);
    uint8_t su_rb_is_ready(su_rb_t* ppt_buff);
    void    su_rb_free(su_rb_t* ppt_buff);
    void    su_rb_reset(su_rb_t* ppt_buff);
    void    su_rb_set_evt_fn(su_rb_t* ppt_buff, su_rb_evt_fn ppt_evt_fn);
    void    su_rb_set_arg(su_rb_t* ppt_buff, void* ppt_arg);
    void*   su_rb_get_arg(su_rb_t* ppt_buff);

    /* Read/Write functions */
    su_rb_sz_t su_rb_write(su_rb_t* ppt_buff, const void* ppt_data, su_rb_sz_t p_btw);
    su_rb_sz_t su_rb_read(su_rb_t* ppt_buff, void* ppt_data, su_rb_sz_t p_btr);
    su_rb_sz_t su_rb_peek(const su_rb_t* ppt_buff, su_rb_sz_t p_skip_count, void* ppt_data, su_rb_sz_t p_btp);

    /* Extended read/write functions */
    uint8_t su_rb_write_ex(su_rb_t* ppt_buff, const void* ppt_data, su_rb_sz_t p_btw, su_rb_sz_t* ppt_bwritten,
                           uint16_t p_flags);
    uint8_t su_rb_read_ex(su_rb_t* ppt_buff, void* ppt_data, su_rb_sz_t p_btr, su_rb_sz_t* ppt_bread,
                          uint16_t p_flags);

    /* Buffer size information */
    su_rb_sz_t su_rb_get_free(const su_rb_t* ppt_buff);
    su_rb_sz_t su_rb_get_full(const su_rb_t* ppt_buff);

    /* Read data block management */
    void*      su_rb_get_linear_block_read_address(const su_rb_t* ppt_buff);
    su_rb_sz_t su_rb_get_linear_block_read_length(const su_rb_t* ppt_buff);
    su_rb_sz_t su_rb_skip(su_rb_t* ppt_buff, su_rb_sz_t p_len);

    /* Write data block management */
    void*      su_rb_get_linear_block_write_address(const su_rb_t* ppt_buff);
    su_rb_sz_t su_rb_get_linear_block_write_length(const su_rb_t* ppt_buff);
    su_rb_sz_t su_rb_advance(su_rb_t* ppt_buff, su_rb_sz_t p_len);

    /* Search in buffer */
    uint8_t    su_rb_find(const su_rb_t* ppt_buff, const void* ppt_bts, su_rb_sz_t p_len,
                          su_rb_sz_t p_start_offset, su_rb_sz_t* ppt_found_idx);
    su_rb_sz_t su_rb_overwrite(su_rb_t* ppt_buff, const void* ppt_data, su_rb_sz_t p_btw);
    su_rb_sz_t su_rb_move(su_rb_t* ppt_dest, su_rb_t* ppt_src);

    /**
     * \}
     */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SU_RB_HDR_H */
