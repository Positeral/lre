/*
BSD 2-Clause License

Copyright (c) 2019, Arthur Goncharuk
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once
#ifndef _LRE_H
#define _LRE_H

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

/* Portable stdint.h provide cross-compiler
 * type definitions and printf() number modifiers */
 #include "pstdint.h"


#ifdef _MSC_VER
	#define isnan _isnan
	#define isinf(x) (!_finite(x))
#endif


/* Detalized error code is in lre_error_t */
#define LRE_OK   0
#define LRE_FAIL 1


/* Code of delimiter must be must be smaller than all other
 * characters from HEX and tags */
#if !defined(LRE_SEP)
	#define LRE_SEP '+'
#endif


#if !defined(lre_decl)
	#if defined(__cplusplus) || defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
		#define lre_decl static inline
	#elif defined(_MSC_VER)
		#define lre_decl static __inline
	#else
		#define lre_decl static
	#endif
#endif


#if !defined(lre_std_malloc)
	#define lre_std_malloc malloc
#endif

#if !defined(lre_std_calloc)
	#define lre_std_calloc calloc
#endif

#if !defined(lre_std_realloc)
	#define lre_std_realloc realloc
#endif

#if !defined(lre_std_free)
	#define lre_std_free free
#endif


/* Branch prediction macro for if-statements.
 * Almost all branches in this library are well predictable. */
#if defined(__GNUC__) || defined(__clang__)
	#if !defined(lre_likely)
		#define lre_likely(x) __builtin_expect(!!(x), 1)
	#endif

	#if !defined(lre_unlikely)
		#define lre_unlikely(x) __builtin_expect(!!(x), 0)
	#endif
#else
	#if !defined(lre_likely)
		#define lre_likely(x) (x)
	#endif

	#if !defined(lre_unlikely)
		#define lre_unlikely(x) (x)
	#endif
#endif


#if __cplusplus
extern "C" {
#endif


/* Encoding */
static const uint8_t lrex_hextab[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e','f'
};


/* Decoding */
static const uint8_t lrex_hexrev[256] = {
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0,   0,   0,   0,   0,   0,   // 0-9
	0,   0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0,   0,   0,   0,   0,   0,   0,   0,   0,   // a-f
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
	0,   0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0,   0,   0,   0,   0,   0,   0,   0,   0,   // A-F
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};


typedef enum {
	LRE_TAG_NUMBER = 'A',
	LRE_TAG_STRING = 'G'
} lre_tag_t;


typedef enum {
	LRE_MOD_DEFAULT     = 'D',
	LRE_MOD_STRING_RAW  = 'H',
	LRE_MOD_STRING_UTF8 = 'L'
} lre_mod_t;


typedef enum {
	LRE_SIGN_NEGATIVE_INF = 'A',
	LRE_SIGN_NEGATIVE     = 'G',
	LRE_SIGN_POSITIVE     = 'J',
	LRE_SIGN_POSITIVE_INF = 'P',
	LRE_SIGN_POSITIVE_NAN = 'S'
} lre_sign_t;


typedef enum {
	LRE_ERROR_NOTHING = 0,
	LRE_ERROR_ALLOCATION,
	LRE_ERROR_NULLPTR,
	LRE_ERROR_RANGE,
	LRE_ERROR_LENGTH,
	LRE_ERROR_TAG,
	LRE_ERROR_SIGN,
	LRE_ERROR_MOD,
	LRE_ERROR_HANDLER
} lre_error_t;


lre_decl
const char *lre_strerror(lre_error_t error) {
	switch (error) {
		case LRE_ERROR_NOTHING:    return "Successful return";
		case LRE_ERROR_ALLOCATION: return "Memory cannot be allocated";
		case LRE_ERROR_NULLPTR:    return "Null pointer passed";		
		case LRE_ERROR_RANGE:      return "Value out of allowed range";
		case LRE_ERROR_LENGTH:     return "Invalid length of data";
		case LRE_ERROR_TAG:        return "Unknown tag";
		case LRE_ERROR_SIGN:       return "Unknown sign";
		case LRE_ERROR_MOD:        return "Unknown string modifier";
		case LRE_ERROR_HANDLER:    return "Final value cannot be handled";
		default:                   return "Unknown error";
	}
}


lre_decl
int lre_fail(lre_error_t error, lre_error_t *to) {
	if (lre_likely(to)) {
		*to = error;
	}
	
	return LRE_FAIL;
}

/*
 * LREX LOW-LEVEL funtions.
 * This functions are designed to be as fast as possible, 
 * not to provide a coherent interface.
 */


/* @brief Returns positive value of negative value. */
lre_decl
uint64_t lrex_negate_negative(int64_t value) {
	return (uint64_t) (-1 - value) + 1;
}


/* @brief Returns negative value of positive value. */
lre_decl
int64_t lrex_negate_positive(uint64_t value) {
	return -((int64_t) (value - 1)) - 1;
}


/* @brief Counting of significant bytes, always between 1, 8. */
lre_decl
int lrex_count_nbytes(uint64_t value) {
#if defined(__GNUC__) || defined(__clang__)
	return ((__builtin_clzll(value | 1) ^ 63) >> 3) + 1;
#else
	return \
	(value > UINT64_C(0xff)) + \
	(value > UINT64_C(0xffff)) + \
	(value > UINT64_C(0xffffff)) + \
	(value > UINT64_C(0xffffffff)) + \
	(value > UINT64_C(0xffffffffff)) + \
	(value > UINT64_C(0xffffffffffff)) + \
	(value > UINT64_C(0xffffffffffffff)) + 1;
#endif
}


lre_decl
void lrex_write_char(uint8_t **dst, uint8_t value){
	*(*dst)++ = value;
}


lre_decl
void lrex_write_uint8(uint8_t **dst, uint8_t value) {
	lrex_write_char(dst, lrex_hextab[value >> 4]);
	lrex_write_char(dst, lrex_hextab[value & 0xf]);
}


lre_decl
void lrex_write_uint16(uint8_t **dst, uint16_t value) {
	lrex_write_uint8(dst, value >> 8);
	lrex_write_uint8(dst, value & 0xff);
}


lre_decl
void lrex_write_uint64n(uint8_t **dst, uint64_t value, size_t nbytes) {
	while (nbytes--) {
		int byte = (value >> (nbytes << 3)) & 0xff;
		lrex_write_uint8(dst, byte);
	}
}


lre_decl
void lrex_write_str(uint8_t **dst, const uint8_t *src, size_t len, uint8_t mask) {
	while (len--) {
		int byte = *src++ ^ mask;
		lrex_write_uint8(dst, byte);
	}
}


lre_decl
uint8_t lrex_read_char(const uint8_t **src) {
	return *(*src)++;
}


lre_decl
uint8_t lrex_read_uint8(const uint8_t **src, uint8_t mask) {
	int a = lrex_hexrev[lrex_read_char(src)];
	int b = lrex_hexrev[lrex_read_char(src)];
	return ((a << 4) | b) ^ mask;
}


lre_decl
uint16_t lrex_read_uint16(const uint8_t **src, uint8_t mask) {
	int a = lrex_read_uint8(src, mask);
	int b = lrex_read_uint8(src, mask);
	return (a << 8) | b;
}


lre_decl
uint64_t lrex_read_uint64n(const uint8_t **src, size_t nbytes, uint8_t mask) {
	uint64_t value = 0;
	
	while (nbytes--) {
		value = (value << 8) | lrex_read_uint8(src, mask);
	}
	
	return value;
}


lre_decl
void lrex_read_str(const uint8_t **src, uint8_t *dst, size_t nbytes, uint8_t mask) {
	while (nbytes--) {
		*dst++ = lrex_read_uint8(src, mask);
	}
}


/*
 * */
typedef struct {
	const uint8_t *src;
	const uint8_t *end;
} lre_slice_t;


/**
 * @brief Returns length of slice
 * @param slice Pointer to lre_slice_t object
 */
lre_decl
ptrdiff_t lre_slice_len(const lre_slice_t *slice) {
	return slice->end - slice->src;
}


/**
 * @brief Returns last byte and shifts end of slice
 * @param slice Pointer to lre_slice_t object
 */
lre_decl
uint8_t lre_slice_pop(lre_slice_t *slice) {
	return *(--slice->end);
}


/* LRE MEMORY BUFFER.
 * Normally, it is long-lived objects in one thread. */
typedef struct {
	uint8_t *data;     /* Payload */
	size_t   size;     /* Payload length */
	size_t   capacity; /* Total *data allocated space  */
	size_t   reserved; /* Minimal *data allocated space */
} lre_buffer_t;


lre_decl
lre_buffer_t *lre_buffer_create(size_t reserve, lre_error_t *error) {
	lre_buffer_t *buf = lre_std_calloc(1, sizeof(lre_buffer_t));
	
	if (lre_unlikely(!buf)) {
		lre_fail(LRE_ERROR_ALLOCATION, error);
		return 0;
	}
	
	if (reserve) {
		buf->data = lre_std_malloc(reserve);
		
		if (!buf->data) {
			lre_fail(LRE_ERROR_ALLOCATION, error);
			lre_std_free(buf);
			return 0;
		}
		
		buf->reserved = reserve;
		buf->capacity = reserve;
	}
	
	return buf;
}


lre_decl
int lre_buffer_require(lre_buffer_t *buf, size_t required, lre_error_t *error) {	
	if (lre_unlikely(buf->size + required > buf->capacity)) {
		size_t capacity = (buf->size + required) * 1.25;
		uint8_t *data = lre_std_realloc(buf->data, capacity);
		
		if (lre_unlikely(!data)) {
			return lre_fail(LRE_ERROR_ALLOCATION, error);
		}
		
		buf->data = data;
		buf->capacity = capacity;
	}
	
	return LRE_OK;
}


lre_decl
uint8_t *lre_buffer_end(lre_buffer_t *buf) {
	return buf->data + buf->size;
}


lre_decl
void lre_buffer_set_size_distance(lre_buffer_t *buf, uint8_t *end) {
	if (lre_likely(end >= buf->data)) {
		buf->size = end - buf->data;
	}
}


lre_decl
int lre_buffer_reset(lre_buffer_t *buf, lre_error_t *error) {
	buf->size = 0;
	
	if (lre_unlikely(buf->capacity != buf->reserved)) {
		uint8_t *data = lre_std_realloc(buf->data, buf->reserved);
		
		if (lre_unlikely(!data && buf->reserved)) {
			return lre_fail(LRE_ERROR_ALLOCATION, error);
		}
		
		buf->data = data;
		buf->capacity = buf->reserved;
	}
	
	return LRE_OK;
}


lre_decl
void lre_buffer_close(lre_buffer_t *buf) {
	if (buf) {
		lre_std_free(buf->data);
		lre_std_free(buf);
	}
}

/*
 *
 */


lre_decl
int lre_pack_str(lre_buffer_t *buf, const uint8_t *src, size_t len, lre_mod_t mod, lre_error_t *error) {
	/* tag(1) + string(len*2) + modifier(1) + separator(1) */
	if (lre_likely(lre_buffer_require(buf, (1+(len*2)+1+1), error) == LRE_OK)) {
		uint8_t *dst = lre_buffer_end(buf);
		
		lrex_write_char(&dst, LRE_TAG_STRING);
		
		if (lre_unlikely(!mod)) {
			mod = LRE_MOD_STRING_RAW;
		}
		
		lrex_write_str (&dst, src, len, 0);
		lrex_write_char(&dst, mod);
		lrex_write_char(&dst, LRE_SEP);
		lre_buffer_set_size_distance(buf, dst);
		
		return LRE_OK;
	}
	
	return LRE_FAIL;
}


lre_decl
int lre_pack_int(lre_buffer_t *buf, int64_t value, lre_error_t *error) {
	/* tag(1) + sign(1) + nbytes(2) + value(16) + separator(1) */
	if (lre_likely(lre_buffer_require(buf, (1+1+2+16+1), error) == LRE_OK)) {
		uint8_t *dst = lre_buffer_end(buf);
		
		lrex_write_char(&dst, LRE_TAG_NUMBER);
		
		if (value < 0) {
			uint64_t uvalue = lrex_negate_negative(value);
			uint8_t  nbytes = lrex_count_nbytes(uvalue);
			
			lrex_write_char   (&dst, LRE_SIGN_NEGATIVE);
			lrex_write_uint8  (&dst, ~nbytes);
			lrex_write_uint64n(&dst, ~uvalue, nbytes);
		}
		else {
			uint8_t nbytes = lrex_count_nbytes(value);
			
			lrex_write_char   (&dst, LRE_SIGN_POSITIVE);
			lrex_write_uint8  (&dst, nbytes);
			lrex_write_uint64n(&dst, value, nbytes);
		}
		
		lrex_write_char(&dst, LRE_SEP);
		lre_buffer_set_size_distance(buf, dst);
		
		return LRE_OK;
	}
	
	return LRE_FAIL;
}


lre_decl
int lre_pack_double(lre_buffer_t *buf, double value, lre_error_t *error) {
	/* tag(1) + sign(1) + nbytes(2) + integral(16) + fraction(16) + separator(1) */
	if (lre_likely(lre_buffer_require(buf, (1+1+2+16+16+1), error) == LRE_OK)) {
		uint8_t *dst = lre_buffer_end(buf);
		
		lrex_write_char(&dst, LRE_TAG_NUMBER);
		
		if (value < 0) {
			uint64_t integral = lrex_negate_negative(value);
			uint64_t fraction = lrex_negate_negative((value + integral) * 1e15 - 0.5);
			uint8_t  nbytes   = lrex_count_nbytes(integral);
			
			lrex_write_char   (&dst, LRE_SIGN_NEGATIVE);
			lrex_write_uint8  (&dst, ~nbytes);
			lrex_write_uint64n(&dst, ~integral, nbytes);
			lrex_write_uint64n(&dst, ~fraction, 7);
		}
		else {
			uint64_t integral = value;
			uint64_t fraction = (value - integral) * 1e15 + 0.5;
			uint8_t  nbytes   = lrex_count_nbytes(integral);
			
			lrex_write_char   (&dst, LRE_SIGN_POSITIVE);
			lrex_write_uint8  (&dst, nbytes);
			lrex_write_uint64n(&dst, integral, nbytes);
			lrex_write_uint64n(&dst, fraction, 7);
		}
		
		lrex_write_char(&dst, LRE_SEP);
		lre_buffer_set_size_distance(buf, dst);
		
		return LRE_OK;
	}
	
	return LRE_FAIL;
}


/*
 * */
typedef struct {
	lre_sign_t sign;
	ptrdiff_t  len_integral; /* Number of encoded bytes */
	ptrdiff_t  len_fraction; /* Number of encoded bytes afetr integer part */
} lre_number_info_t;


/* LRE end handlers for unpack.
 * Normally, it is long-lived objects. */
typedef struct {
	void *app_private;
	int (*handle_int)   (void *app_private, int64_t value);
	int (*handle_double)(void *app_private, double value);
	int (*handle_str)   (void *app_private, lre_slice_t *slice, lre_mod_t mod);
	int (*handle_number)(void *app_private, lre_slice_t *slice, lre_sign_t sign);
} lre_handlers_t;


lre_decl // TODO
int lre_handle_string(const lre_handlers_t *hns, lre_slice_t *slice, lre_error_t *error) {
	lre_mod_t modifier;
	
	if (lre_unlikely((lre_slice_len(slice) - 1) % 2)) {
		return lre_fail(LRE_ERROR_LENGTH, error);
	}
	
	/* Last character is a modifier value */
	modifier = (lre_mod_t) lre_slice_pop(slice);
	
	switch (modifier) {
		case LRE_MOD_STRING_UTF8: break;
		case LRE_MOD_STRING_RAW:  break;
		case LRE_MOD_DEFAULT:     break;
		default: return lre_fail(LRE_ERROR_MOD, error);
	}
	
	if (lre_unlikely(hns->handle_str(hns->app_private, slice, modifier) != LRE_OK)) {
		return lre_fail(LRE_ERROR_HANDLER, error);
	}
	
	return LRE_OK;
}


lre_decl // TODO
int lre_handle_number(const lre_handlers_t *hns, lre_slice_t *slice, lre_error_t *error) {
	return LRE_OK;
}


/*
 */
lre_decl
int lre_tokenize(const lre_handlers_t *hns, const uint8_t *src, size_t size, lre_error_t *error) {
	const uint8_t *sep = src;
	const uint8_t *end = src + size;
	
	while ((sep = memchr(src, LRE_SEP, end - sep))) {
		lre_tag_t   tag   = lrex_read_char(&src);
		lre_slice_t slice = {src, sep};
		
		if (lre_unlikely(lre_slice_len(&slice) < 1)) {
			return lre_fail(LRE_ERROR_LENGTH, error);
		}
		
		switch (tag) {
			case LRE_TAG_STRING:
				if (lre_unlikely(lre_handle_string(hns, &slice, error) != LRE_OK)) {
					return LRE_FAIL;
				}
				
				break;
			
			case LRE_TAG_NUMBER:
				if (lre_unlikely(lre_handle_number(hns, &slice, error) != LRE_OK)) {
					return LRE_FAIL;
				}
				
				break;

			default:
				return lre_fail(LRE_ERROR_TAG, error);
		}
		
		src = sep + 1;
	}
	
	return LRE_OK;
}


/* extern "C" */
#if __cplusplus
}
#endif

/* _LRE_H */
#endif


