from cython cimport final
from libc.stdint cimport uint8_t, int64_t
from cpython.bytes cimport PyBytes_FromStringAndSize

# From lre.h
DEF LRE_OK   = 0
DEF LRE_FAIL = 1

cdef extern from '../../lre.h':
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

	void lrex_read_str(const uint8_t **src, uint8_t *dst, size_t nbytes, uint8_t mask)

	ctypedef struct lre_buffer_t:
		uint8_t *data
		size_t   size
		size_t   capacity
		size_t   reserved

	lre_buffer_t *lre_buffer_create(size_t reserve, lre_error_t *error)
	int lre_buffer_require(lre_buffer_t *buf, size_t required, lre_error_t *error)
	int lre_buffer_reset(lre_buffer_t *buf, lre_error_t *error)
	void lre_buffer_close(lre_buffer_t *buf)

	int lre_pack_str(lre_buffer_t *buf, const uint8_t *src, size_t len, lre_mod_t mod, lre_error_t *error)
	int lre_pack_int(lre_buffer_t *buf, int64_t value, lre_error_t *error)
	int lre_pack_float(lre_buffer_t *buf, double value, lre_error_t *error)

	ctypedef struct lre_slice_t:
		const uint8_t *src
		const uint8_t *end

	ptrdiff_t lre_slice_len(const lre_slice_t *slice)

	ctypedef struct lre_number_info_t:
		lre_tag_t tag
		ptrdiff_t nbytes_integral  # Number of encoded bytes
		ptrdiff_t ndigits_fraction # Number of digits after integer part

	ctypedef struct lre_loader_t:
		void *app_private;
		int (*handler_int)     (lre_loader_t *loader, int64_t value) except? LRE_FAIL
		int (*handler_float)   (lre_loader_t *loader, double value)  except? LRE_FAIL
		int (*handler_str)     (lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod) except? LRE_FAIL
		int (*handler_inf)     (lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) except? LRE_FAIL
		int (*handler_bigint)  (lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) except? LRE_FAIL
		int (*handler_bigfloat)(lre_loader_t *loader, lre_slice_t *slice, lre_number_info_t *info) except? LRE_FAIL

	void lre_loader_init(lre_loader_t *loader, void *app_private)
	int lre_tokenize(lre_loader_t *loader, const uint8_t *src, size_t size, lre_error_t *error) except? LRE_FAIL

	const char *lre_strerror(lre_error_t error)


cdef class Error(Exception):
	pass


cdef int loader_handler_int(lre_loader_t *loader, int64_t value) except? LRE_FAIL:
	cdef LRE lre = <LRE> loader.app_private
	lre.hndl_key.append(value)
	return LRE_OK


cdef int loader_handler_float(lre_loader_t *loader, double value) except? LRE_FAIL:
	cdef LRE lre = <LRE> loader.app_private
	lre.hndl_key.append(value)
	return LRE_OK


cdef int loader_handler_str(lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod) except? LRE_FAIL:
	cdef LRE lre = <LRE> loader.app_private
	cdef ptrdiff_t nbytes = lre_slice_len(slice) / 2
	cdef bytes s = PyBytes_FromStringAndSize(<char *> 0, nbytes)

	lrex_read_str(&slice.src, <uint8_t *> s, nbytes, 0)

	if mod == LRE_MOD_STRING_UTF8:
		lre.hndl_key.append(s.decode('utf8'))
	else:
		lre.hndl_key.append(s)

	return LRE_OK


@final
cdef class LRE:
	cdef lre_error_t   lrerror
	cdef lre_buffer_t *lrbuf
	cdef lre_loader_t  lrloader
	cdef list          hndl_key

	def __cinit__(self, int reserve):
		self.lrbuf = lre_buffer_create(reserve, &self.lrerror)

		lre_loader_init(&self.lrloader, <void *> self)
		self.lrloader.handler_int   = &loader_handler_int
		self.lrloader.handler_float = &loader_handler_float
		self.lrloader.handler_str   = &loader_handler_str

	def __init__(self, int reserve):
		if not self.lrbuf:
			raise MemoryError(lre_strerror(self.error).decode('utf8'))

	cpdef bytes pack(self, key):
		cdef lre_error_t error = LRE_ERROR_NOTHING

		try:
			if not isinstance(key, list):
				key = [key]

			for i in key:
				if isinstance(i, unicode):
					i = i.encode('utf8')
					lre_pack_str(self.lrbuf, i, len(i), LRE_MOD_STRING_UTF8, &error)

				elif isinstance(i, int):
					lre_pack_int(self.lrbuf, i, &error)

				elif isinstance(i, bytes):
					lre_pack_str(self.lrbuf, i, len(i), LRE_MOD_STRING_RAW, &error)

				elif isinstance(i, float):
					lre_pack_float(self.lrbuf, i, &error)

				else:
					raise ValueError('type <%s> is unsupported' % type(i).__name__)

				if error:
					raise ValueError(lre_strerror(error).decode('utf8'))

			return self.lrbuf.data[:self.lrbuf.size]
		finally:
			if lre_buffer_reset(self.lrbuf, &error) != LRE_OK:
				raise MemoryError(lre_strerror(error).decode('utf8'))

	cpdef object load(self, key):
		cdef const uint8_t *src
		cdef lre_error_t error = LRE_ERROR_NOTHING

		self.hndl_key = []

		if isinstance(key, bytes):
			src = key
		elif isinstance(key, unicode):
			key = key.encode('utf8')
			src = key
		else:
			raise TypeError("a bytes or unicode object is required, not '%s'" % type(key).__name__)

		if lre_tokenize(&self.lrloader, src, len(key), &error) != LRE_OK:
			raise ValueError(lre_strerror(error).decode('utf8'))

		return self.hndl_key

lre = LRE(1)
pack = lre.pack
load = lre.load
dumps = pack
loads = load
