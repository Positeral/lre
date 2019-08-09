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
	#define lre_isnan(x) (_isnan(x))
	#define lre_isinf(x) (!_finite(x))
#else
	#define lre_isnan(x) (isnan(x))
	#define lre_isinf(x) (isinf(x))
#endif


/* Detalized error code is in lre_error_t */
#define LRE_OK   0
#define LRE_FAIL 1


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


/* For decimal inversion */
static const int64_t lrex_max10[16] = {
	INT64_C(0),
	INT64_C(9),
	INT64_C(99),
	INT64_C(999),
	INT64_C(9999),
	INT64_C(99999),
	INT64_C(999999),
	INT64_C(9999999),
	INT64_C(99999999),
	INT64_C(999999999),
	INT64_C(9999999999),
	INT64_C(99999999999),
	INT64_C(999999999999),
	INT64_C(9999999999999),
	INT64_C(99999999999999),
	INT64_C(999999999999999)
};

/* Precalculate powers of 10 */
static const double lrex_pow10[16] = {
	1.0,
	10.0,
	100.0,
	1000.0,
	10000.0,
	100000.0,
	1000000.0,
	10000000.0,
	100000000.0,
	1000000000.0,
	10000000000.0,
	100000000000.0,
	1000000000000.0,
	10000000000000.0,
	100000000000000.0,
	1000000000000000.0
};

typedef enum {
	LRE_SEP_NEGATIVE = '~',
	LRE_SEP_POSITIVE = '+'
} lre_sep_t;


/* Do not change value of existing numeric tags */
typedef enum {
	LRE_TAG_NUMBER_NEGATIVE_INF = 'C',
	LRE_TAG_NUMBER_NEGATIVE_BIG = 'D',
	LRE_TAG_NUMBER_NEGATIVE_8   = 'E',
	LRE_TAG_NUMBER_NEGATIVE_7   = 'F',
	LRE_TAG_NUMBER_NEGATIVE_6   = 'G',
	LRE_TAG_NUMBER_NEGATIVE_5   = 'H',
	LRE_TAG_NUMBER_NEGATIVE_4   = 'I',
	LRE_TAG_NUMBER_NEGATIVE_3   = 'J',
	LRE_TAG_NUMBER_NEGATIVE_2   = 'K',
	LRE_TAG_NUMBER_NEGATIVE_1   = 'L',
	
	LRE_TAG_NUMBER_POSITIVE_1   = 'M',
	LRE_TAG_NUMBER_POSITIVE_2   = 'N',
	LRE_TAG_NUMBER_POSITIVE_3   = 'O',
	LRE_TAG_NUMBER_POSITIVE_4   = 'P',
	LRE_TAG_NUMBER_POSITIVE_5   = 'Q',
	LRE_TAG_NUMBER_POSITIVE_6   = 'R',
	LRE_TAG_NUMBER_POSITIVE_7   = 'S',
	LRE_TAG_NUMBER_POSITIVE_8   = 'T',
	LRE_TAG_NUMBER_POSITIVE_BIG = 'U',
	LRE_TAG_NUMBER_POSITIVE_INF = 'V',
	
	LRE_TAG_STRING              = 'X'
} lre_tag_t;


/* String modifiers */
typedef enum {
	LRE_MOD_DEFAULT     = 0, /* Will never be packed */
	LRE_MOD_STRING_RAW  = 'H',
	LRE_MOD_STRING_UTF8 = 'L'
} lre_mod_t;


typedef enum {
	LRE_ERROR_NOTHING = 0,
	LRE_ERROR_ALLOCATION,
	LRE_ERROR_NULLPTR,
	LRE_ERROR_RANGE,
	LRE_ERROR_NAN,
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
		case LRE_ERROR_NAN:        return "Value is not a number";
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

/**
 * @brief Returns negative value of positive value. Avoids signed overflow.
 */
lre_decl
int64_t lrex_negate_positive(uint64_t value) {
	return -((int64_t) (value - 1)) - 1;
}


/**
 * @brief Returns positive value of negative value. Avoids signed overflow.
 */
lre_decl
uint64_t lrex_negate_negative(int64_t value) {
	return (uint64_t) (-1 - value) + 1;
}


/**
 * @brief Returns tag for positive number according to number of bytes
 * @param nbytes Number of bytes (lrex_count_nbytes). Strictly from 1 to 8.
 */
lre_decl
lre_tag_t lrex_tag_by_nbytes_positive(int nbytes) {
	return (LRE_TAG_NUMBER_POSITIVE_1 - 1) + nbytes;
}


/**
 * @brief Returns tag for negative number according to number of bytes
 * @param nbytes Number of bytes (lrex_count_nbytes). Strictly from 1 to 8.
 */
lre_decl
lre_tag_t lrex_tag_by_nbytes_negative(int nbytes) {
	return (LRE_TAG_NUMBER_NEGATIVE_1 + 1) - nbytes;
}


/**
 * @brief Returns number of bytes according to positive numeric tag
 * @param 
 */
lre_decl
int lrex_nbytes_by_tag_positive(lre_tag_t numeric_tag) {
	return (int) numeric_tag - ((int) LRE_TAG_NUMBER_POSITIVE_1 - 1);
}


/**
 * @brief Returns number of bytes according to negative numeric tag
 * @param 
 */
lre_decl
int lrex_nbytes_by_tag_negative(lre_tag_t numeric_tag) {
	return ((int) LRE_TAG_NUMBER_NEGATIVE_1 + 1) - (int) numeric_tag;
}


/* */
lre_decl
int lrex_tag_is_string(lre_tag_t tag) {
	return tag == LRE_TAG_STRING;
}


/* */
lre_decl
int lrex_tag_is_number(lre_tag_t tag) {
	return
	(tag >= LRE_TAG_NUMBER_NEGATIVE_INF) &&
	(tag <= LRE_TAG_NUMBER_POSITIVE_INF);
}


/* */
lre_decl
int lrex_tag_is_negative(lre_tag_t tag) {
	return tag < LRE_TAG_NUMBER_POSITIVE_1;
}

/* */
lre_decl
int lrex_tag_is_positive(lre_tag_t tag) {
	return tag > LRE_TAG_NUMBER_NEGATIVE_1;
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
		int byte = (value >> (nbytes * 8)) & 0xff;
		lrex_write_uint8(dst, byte);
	}
}


lre_decl
void lrex_write_decimal(uint8_t **dst, uint64_t value, size_t ndigits) {
	uint8_t *ptr = (*dst += ndigits);

	while (ndigits--) {
		*--ptr = '0' + (value % 10);
		value /= 10;
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
uint64_t lrex_read_decimal(const uint8_t **src, size_t ndigits) {
	uint64_t value = 0;

	while (ndigits--) {
		value *= 10;
		value += lrex_read_char(src) - '0';
	}

	return value;
}


lre_decl
void lrex_read_str(const uint8_t **src, uint8_t *dst, size_t nbytes, uint8_t mask) {
	while (nbytes--) {
		*dst++ = lrex_read_uint8(src, mask);
	}
}


lre_decl
const uint8_t *lrex_memsep(const uint8_t *src, size_t size) {
	for (; size; size--, src++) {
		if (*src == LRE_SEP_POSITIVE || *src == LRE_SEP_NEGATIVE) {
			return src;
		}
	}
	
	return 0;
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
	
	reserve++;
	buf->data = lre_std_malloc(reserve);
		
	if (!buf->data) {
		lre_fail(LRE_ERROR_ALLOCATION, error);
		lre_std_free(buf);
		return 0;
	}
	
	buf->data[0] = '\0';
	buf->reserved = reserve;
	buf->capacity = reserve;

	return buf;
}


lre_decl
int lre_buffer_require(lre_buffer_t *buf, size_t required, lre_error_t *error) {
	if (lre_unlikely(buf->size + required > buf->capacity)) {
		size_t capacity = (buf->size + required + 1) * 10 / 8; /* +25% */
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
void lre_buffer_set_size_distance(lre_buffer_t *buf, const uint8_t *end) {
	if (lre_likely(end >= buf->data)) {
		buf->size = end - buf->data;
		buf->data[buf->size] = '\0';
	}
}


lre_decl
int lre_buffer_reset(lre_buffer_t *buf, lre_error_t *error) {
	buf->size = 0;
	buf->data[0] = '\0';

	if (lre_unlikely(buf->capacity != buf->reserved)) {
		uint8_t *data = lre_std_realloc(buf->data, buf->reserved);
		
		if (lre_unlikely(!data)) {
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
		lrex_write_char(&dst, (int) mod);
		lrex_write_char(&dst, LRE_SEP_POSITIVE);
		lre_buffer_set_size_distance(buf, dst);
		return LRE_OK;
	}
	
	return LRE_FAIL;
}


lre_decl
int lre_pack_int(lre_buffer_t *buf, int64_t value, lre_error_t *error) {
	/* tag(1) + value(16) + separator(1) */
	if (lre_likely(lre_buffer_require(buf, (1+16+1), error) == LRE_OK)) {
		uint8_t *dst = lre_buffer_end(buf);
		
		if (value < 0) {
			uint64_t uvalue = lrex_negate_negative(value);
			uint8_t  nbytes = lrex_count_nbytes(uvalue);
			
			lrex_write_char   (&dst, (int) lrex_tag_by_nbytes_negative(nbytes));
			lrex_write_uint64n(&dst, ~uvalue, nbytes);
			lrex_write_char   (&dst, LRE_SEP_NEGATIVE);
		}
		else {
			uint8_t nbytes = lrex_count_nbytes(value);
			
			lrex_write_char   (&dst, (int) lrex_tag_by_nbytes_positive(nbytes));
			lrex_write_uint64n(&dst, value, nbytes);
			lrex_write_char   (&dst, LRE_SEP_POSITIVE);
		}
		
		lre_buffer_set_size_distance(buf, dst);
		return LRE_OK;
	}
	
	return LRE_FAIL;
}


lre_decl
int lre_pack_float(lre_buffer_t *buf, double value, lre_error_t *error) {
	if (lre_unlikely(lre_isnan(value))) {
		return lre_fail(LRE_ERROR_NAN, error);
	}
	
	/* tag(1) + integral(16) + fraction(15) + separator(1) */
	if (lre_likely(lre_buffer_require(buf, (1+16+15+1), error) == LRE_OK)) {
		uint8_t *dst = lre_buffer_end(buf);
		
		if (lre_unlikely(lre_isinf(value))) {
			if (value < 0) {
				lrex_write_char(&dst, LRE_TAG_NUMBER_NEGATIVE_INF);
				lrex_write_char(&dst, LRE_SEP_NEGATIVE);
			}
			else {
				lrex_write_char(&dst, LRE_TAG_NUMBER_POSITIVE_INF);
				lrex_write_char(&dst, LRE_SEP_POSITIVE);
			}
			
			lre_buffer_set_size_distance(buf, dst);
			return LRE_OK;
		}
		
		if (lre_unlikely(value > 281474976710655.0 || value < -281474976710655.0)) {
			return lre_fail(LRE_ERROR_RANGE, error);
		}
		
		if (value < 0) {
			uint64_t integral = lrex_negate_negative(value);
			uint64_t fraction = lrex_negate_negative((value + integral) * 1e15 - 0.5);
			uint8_t  nbytes   = lrex_count_nbytes(integral);
			
			/* Decimal inversion */
			fraction = lrex_max10[15] - fraction;
			
			lrex_write_char   (&dst, (int) lrex_tag_by_nbytes_negative(nbytes));
			lrex_write_uint64n(&dst, ~integral, nbytes);
			lrex_write_decimal(&dst, fraction, 15);
			lrex_write_char   (&dst, LRE_SEP_NEGATIVE);
		}
		else {
			uint64_t integral = value;
			uint64_t fraction = (value - integral) * 1e15 + 0.5;
			uint8_t  nbytes   = lrex_count_nbytes(integral);
			
			lrex_write_char   (&dst, (int) lrex_tag_by_nbytes_positive(nbytes));
			lrex_write_uint64n(&dst, integral, nbytes);
			lrex_write_decimal(&dst, fraction, 15);
			lrex_write_char   (&dst, LRE_SEP_POSITIVE);
		}
		
		lre_buffer_set_size_distance(buf, dst);
		return LRE_OK;
	}
	
	return LRE_FAIL;
}


/*
 * */
typedef struct {
	lre_tag_t tag;
	ptrdiff_t nbytes_integral;  /* Number of encoded bytes */
	ptrdiff_t ndigits_fraction; /* Number of digits after integer part */
} lre_number_info_t;


typedef struct lre_loader_t lre_loader_t;

/* LRE end handlers for unpack.
 * Normally, it is long-lived objects. */
typedef struct lre_loader_t {
	void *app_private;
	int (*handler_int)     (lre_loader_t *loader, int64_t value);
	int (*handler_float)   (lre_loader_t *loader, double value);
	int (*handler_str)     (lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod);
	int (*handler_inf)     (lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info);
	int (*handler_bigint)  (lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info);
	int (*handler_bigfloat)(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info);
} lre_loader_t;


lre_decl
int lre_loader_default_handler_int(lre_loader_t *loader, int64_t value) {
	return LRE_OK;
}


lre_decl
int lre_loader_default_handler_float(lre_loader_t *loader, double value) {
	return LRE_OK;
}


lre_decl
int lre_loader_default_handler_str(lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod) {
	return LRE_OK;
}


lre_decl
int lre_loader_default_handler_inf(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) {
	double value = lrex_tag_is_negative(info->tag) ? -INFINITY : INFINITY;

	if (lre_likely(loader->handler_float)) {
		return loader->handler_float(loader, value);
	}

	return LRE_OK;
}


lre_decl
int lre_loader_default_handler_bigint(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) {
	return LRE_OK;
}


lre_decl
int lre_loader_default_handler_bigfloat(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) {
	return LRE_OK;
}


lre_decl
void lre_loader_init(lre_loader_t *loader) {
	loader->app_private      = 0;

	loader->handler_int      = &lre_loader_default_handler_int;
	loader->handler_float    = &lre_loader_default_handler_float;
	loader->handler_str      = &lre_loader_default_handler_str;
	loader->handler_inf      = &lre_loader_default_handler_inf;
	loader->handler_bigint   = &lre_loader_default_handler_bigint;
	loader->handler_bigfloat = &lre_loader_default_handler_bigfloat;
}


lre_decl // TODO
int lre_handle_string(lre_loader_t *loader, lre_tag_t tag, lre_slice_t *slice, lre_error_t *error) {
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
	
	if (lre_unlikely(loader->handler_str(loader, slice, modifier) != LRE_OK)) {
		return lre_fail(LRE_ERROR_HANDLER, error);
	}
	
	return LRE_OK;
}


lre_decl
int lre_handle_number_bignum(lre_loader_t *loader,
                            lre_slice_t *slice,
						    lre_number_info_t *info,
						    lre_error_t *error) {
	return lre_fail(LRE_ERROR_RANGE, error);
}


lre_decl // TODO
int lre_handle_number(lre_loader_t *loader, lre_tag_t tag, lre_slice_t *slice, lre_error_t *error) {
	uint8_t mask;
	uint64_t integral;
	uint64_t fraction;
	lre_number_info_t info = {tag, 0, 0};
	lre_slice_t srcslice = *slice;
	
	switch (tag) {
		case LRE_TAG_NUMBER_POSITIVE_BIG:
		case LRE_TAG_NUMBER_NEGATIVE_BIG:
			return lre_handle_number_bignum(loader, &srcslice, &info, error);

		case LRE_TAG_NUMBER_POSITIVE_INF:
		case LRE_TAG_NUMBER_NEGATIVE_INF:
			if (lre_unlikely(loader->handler_inf(loader, &srcslice, &info) != LRE_OK)) {
				return lre_fail(LRE_ERROR_HANDLER, error);
			}

			return LRE_OK;
	}
	
	if (lrex_tag_is_negative(tag)) {
		mask = 0xff;
		info.nbytes_integral = lrex_nbytes_by_tag_negative(tag);
	}
	else {
		mask = 0x00;
		info.nbytes_integral = lrex_nbytes_by_tag_positive(tag);
	}

	info.ndigits_fraction = lre_slice_len(slice) - (info.nbytes_integral * 2);
	
	if (lre_unlikely(info.ndigits_fraction < 0)) {
		return lre_fail(LRE_ERROR_LENGTH, error);
	}

	/* Float-point */
	if (info.ndigits_fraction) {
		double value;

		if (lre_unlikely(info.nbytes_integral > 6 || info.ndigits_fraction > 15)) {
			return lre_handle_number_bignum(loader, &srcslice, &info, error);
		}

		integral = lrex_read_uint64n(&slice->src, info.nbytes_integral, mask);
		fraction = lrex_read_decimal(&slice->src, info.ndigits_fraction);

		if (lrex_tag_is_negative(tag)) {
			fraction = lrex_max10[info.ndigits_fraction] - fraction;
			value = -(integral + (fraction / lrex_pow10[info.ndigits_fraction]));
		}
		else {
			value = integral + (fraction / lrex_pow10[info.ndigits_fraction]);
		}

		if (lre_unlikely(loader->handler_float(loader, value) != LRE_OK)) {
			return lre_fail(LRE_ERROR_HANDLER, error);
		}
	}
	/* Integer */
	else {
		if (lre_unlikely(info.nbytes_integral > 8)) {
			return lre_handle_number_bignum(loader, &srcslice, &info, error);
		}

		integral = lrex_read_uint64n(&slice->src, info.nbytes_integral, mask);

		if (lrex_tag_is_negative(tag)) {
			if (lre_unlikely(integral > UINT64_C(9223372036854775808))) {
				return lre_handle_number_bignum(loader, &srcslice, &info, error);
			}

			if (lre_unlikely(loader->handler_int(loader, lrex_negate_positive(integral)) != LRE_OK)) {
				return lre_fail(LRE_ERROR_HANDLER, error);
			}
		}
		else {
			if (lre_unlikely(integral > UINT64_C(9223372036854775807))) {
				return lre_handle_number_bignum(loader, &srcslice, &info, error);
			}

			if (lre_unlikely(loader->handler_int(loader, integral) != LRE_OK)) {
				return lre_fail(LRE_ERROR_HANDLER, error);
			}
		}
	}
	
	return LRE_OK;
}


lre_decl
int lre_tokenize(lre_loader_t *loader, const uint8_t *src, size_t size, lre_error_t *error) {
	const uint8_t *sep = src;
	const uint8_t *end = src + size;
	
	while ((sep = lrex_memsep(src, end - sep))) {
		lre_tag_t   tag   = lrex_read_char(&src);
		lre_slice_t slice = {src, sep};

		src = sep + 1;
		
		if (lre_unlikely(lre_slice_len(&slice) < 0)) {
			return lre_fail(LRE_ERROR_LENGTH, error);
		}
		
		if (lrex_tag_is_string(tag)) {
			if (lre_unlikely(lre_handle_string(loader, tag, &slice, error) != LRE_OK)) {
				return LRE_FAIL;
			}
			
			continue;
		}
		
		if (lrex_tag_is_number(tag)) {
			if (lre_unlikely(lre_handle_number(loader, tag, &slice, error) != LRE_OK)) {
				return LRE_FAIL;
			}
			
			continue;
		}
		
		return lre_fail(LRE_ERROR_TAG, error);
	}

	return LRE_OK;
}


/* extern "C" */
#if __cplusplus
}
#endif

/* _LRE_H */
#endif

