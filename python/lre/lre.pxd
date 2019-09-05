cimport cython


# From lre.h
DEF LRE_OK   = 0
DEF LRE_FAIL = 1

cdef extern from 'lre.h':
	ctypedef int int8_t
	ctypedef int int16_t
	ctypedef int int64_t
	
	ctypedef int uint8_t
	ctypedef int uint16_t
	ctypedef int uint64_t

	ctypedef enum lre_error_t:
		LRE_ERROR_NOTHING
		LRE_ERROR_ALLOCATION
		LRE_ERROR_NULLPTR
		LRE_ERROR_RANGE
		LRE_ERROR_NAN
		LRE_ERROR_LENGTH
		LRE_ERROR_TAG
		LRE_ERROR_SIGN
		LRE_ERROR_MOD
		LRE_ERROR_HANDLER

	cdef enum lre_sep_t:
		LRE_SEP_NEGATIVE
		LRE_SEP_POSITIVE

	ctypedef enum lre_tag_t:
		LRE_TAG_NUMBER_NEGATIVE_INF
		LRE_TAG_NUMBER_NEGATIVE_BIG
		LRE_TAG_NUMBER_NEGATIVE_8
		LRE_TAG_NUMBER_NEGATIVE_7
		LRE_TAG_NUMBER_NEGATIVE_6
		LRE_TAG_NUMBER_NEGATIVE_5
		LRE_TAG_NUMBER_NEGATIVE_4
		LRE_TAG_NUMBER_NEGATIVE_3
		LRE_TAG_NUMBER_NEGATIVE_2
		LRE_TAG_NUMBER_NEGATIVE_1
		LRE_TAG_NUMBER_POSITIVE_1
		LRE_TAG_NUMBER_POSITIVE_2
		LRE_TAG_NUMBER_POSITIVE_3
		LRE_TAG_NUMBER_POSITIVE_4
		LRE_TAG_NUMBER_POSITIVE_5
		LRE_TAG_NUMBER_POSITIVE_6
		LRE_TAG_NUMBER_POSITIVE_7
		LRE_TAG_NUMBER_POSITIVE_8
		LRE_TAG_NUMBER_POSITIVE_BIG
		LRE_TAG_NUMBER_POSITIVE_INF
		LRE_TAG_STRING

	ctypedef enum lre_mod_t:
		LRE_MOD_DEFAULT
		LRE_MOD_STRING_RAW
		LRE_MOD_STRING_UTF8

	void     lrex_write_char(uint8_t **dst, uint8_t value)
	void     lrex_write_uint16(uint8_t **dst, uint16_t value)
	void     lrex_write_str(uint8_t **dst, const uint8_t *src, size_t len, uint8_t mask)
	void     lrex_read_str(const uint8_t **src, uint8_t *dst, size_t nbytes, uint8_t mask)
	uint64_t lrex_read_uint64n(const uint8_t **src, size_t nbytes, uint8_t mask)

	ctypedef struct lre_buffer_t:
		uint8_t *data
		size_t   size
		size_t   capacity
		size_t   reserved

	lre_buffer_t *lre_buffer_create(size_t reserve, lre_error_t *error)
	int           lre_buffer_require(lre_buffer_t *buf, size_t required, lre_error_t *error)
	uint8_t      *lre_buffer_end(lre_buffer_t *buf)
	void          lre_buffer_set_size_distance(lre_buffer_t *buf, const uint8_t *end)
	void          lre_buffer_reset_fast(lre_buffer_t *buf)
	int           lre_buffer_reset(lre_buffer_t *buf, lre_error_t *error)
	void          lre_buffer_close(lre_buffer_t *buf)

	int lre_pack_str(lre_buffer_t *buf, const uint8_t *src, size_t len, lre_mod_t mod, lre_error_t *error)
	int lre_pack_int(lre_buffer_t *buf, int64_t value, lre_error_t *error)
	int lre_pack_float(lre_buffer_t *buf, double value, lre_error_t *error)

	ctypedef struct lre_slice_t:
		const uint8_t *src
		const uint8_t *end

	ptrdiff_t lre_slice_len(const lre_slice_t *slice)

	ctypedef struct lre_number_info_t:
		lre_tag_t tag
		uint8_t   mask
		ptrdiff_t nbytes_integral  # Number of encoded bytes
		ptrdiff_t ndigits_fraction # Number of digits after integer part

	ctypedef struct lre_loader_t:
		void *app_private;
		int (*handler_int)     (lre_loader_t *loader, int64_t value) except? LRE_FAIL
		int (*handler_float)   (lre_loader_t *loader, double value)  except? LRE_FAIL
		int (*handler_str)     (lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod) except? LRE_FAIL
		int (*handler_inf)     (lre_loader_t *loader, lre_tag_t tag) except? LRE_FAIL
		int (*handler_bigint)  (lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) except? LRE_FAIL
		int (*handler_bigfloat)(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) except? LRE_FAIL

		ptrdiff_t builtin_nfraction

	void lre_loader_init(lre_loader_t *loader, void *app_private)
	int  lre_tokenize(lre_loader_t *loader, const uint8_t *src, size_t size, lre_error_t *error) except? LRE_FAIL

	const char *lre_strerror(lre_error_t error)


@cython.final
cdef class LRE:
	cdef lre_buffer_t *lrbuffer
	cdef lre_loader_t  lrloader
	cdef list          tmpkey

	cpdef pack(self, key)

	cpdef load(self, key)
	
	cdef write_buffer(self, key)

	cdef write_buffer_bigint(self, pyint)

	cdef write_buffer_decimal(self, pydecimal)

	@staticmethod # Called by lre_tokenize()
	cdef int callback_load_int(lre_loader_t *loader, int64_t value) except? LRE_FAIL

	@staticmethod # Called by lre_tokenize()
	cdef int callback_load_float(lre_loader_t *loader, double value) except? LRE_FAIL

	@staticmethod # Called by lre_tokenize()
	cdef int callback_load_str(lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod) except? LRE_FAIL

	@staticmethod # Called by lre_tokenize()
	cdef int callback_load_bigint(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) except? LRE_FAIL






