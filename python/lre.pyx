from cython cimport final
from libc.stdint cimport uint8_t, uint16_t, int64_t
from cpython.bytes cimport PyBytes_FromStringAndSize


cdef extern from *:
	const char *PyUnicode_AsUTF8AndSize(object unicode, Py_ssize_t *size) except? NULL
	int PyBytes_AsStringAndSize(object obj, char **buffer, Py_ssize_t *length) except? -1
	long long PyLong_AsLongLongAndOverflow(object obj, int *overflow) except? -1


# From lre.h
DEF LRE_OK   = 0
DEF LRE_FAIL = 1

cdef extern from 'lre.h':
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

	void lrex_write_char(uint8_t **dst, uint8_t value)
	void lrex_write_uint16(uint8_t **dst, uint16_t value)
	void lrex_write_str(uint8_t **dst, const uint8_t *src, size_t len, uint8_t mask)
	void lrex_read_str(const uint8_t **src, uint8_t *dst, size_t nbytes, uint8_t mask)

	ctypedef struct lre_buffer_t:
		uint8_t *data
		size_t   size
		size_t   capacity
		size_t   reserved

	lre_buffer_t *lre_buffer_create(size_t reserve, lre_error_t *error)
	int lre_buffer_require(lre_buffer_t *buf, size_t required, lre_error_t *error)
	uint8_t *lre_buffer_end(lre_buffer_t *buf)
	void lre_buffer_set_size_distance(lre_buffer_t *buf, const uint8_t *end)
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


cdef packbufferbigint(lre_buffer_t *lrbuf, object intobj):
	cdef lre_error_t error = LRE_ERROR_NOTHING
	cdef uint8_t *dst
	cdef int nbytes = (intobj.bit_length() + 7) // 8
	cdef bytes s

	if nbytes > 0xffff:
		raise OverflowError('big int out of range')

	# tag(1) + nbytes(4) + value(nbytes*2) + separator(1)
	if lre_buffer_require(lrbuf, (1+4+(nbytes*2)+1), &error) != LRE_OK:
		raise MemoryError(lre_strerror(error).decode('utf8'))

	dst = lre_buffer_end(lrbuf)

	if intobj < 0:
		s = (-intobj).to_bytes(nbytes, 'big')

		lrex_write_char  (&dst, LRE_TAG_NUMBER_NEGATIVE_BIG)
		lrex_write_uint16(&dst, ~nbytes)
		lrex_write_str   (&dst, s, nbytes, 0xff)
		lrex_write_char  (&dst, LRE_SEP_NEGATIVE)
	else:
		s = intobj.to_bytes(nbytes, 'big')

		lrex_write_char  (&dst, LRE_TAG_NUMBER_POSITIVE_BIG)
		lrex_write_uint16(&dst, nbytes)
		lrex_write_str   (&dst, s, nbytes, 0)
		lrex_write_char  (&dst, LRE_SEP_POSITIVE)

	lre_buffer_set_size_distance(lrbuf, dst)


cdef packbuffer(lre_buffer_t *lrbuf, object key):
	cdef lre_error_t error = LRE_ERROR_NOTHING

	cdef const uint8_t *str_value
	cdef Py_ssize_t     str_size

	cdef int     int_overflow
	cdef int64_t int_value

	if not isinstance(key, list):
		key = [key]

	for i in key:
		if isinstance(i, unicode):
			str_value = <const uint8_t *> PyUnicode_AsUTF8AndSize(i, &str_size)
			lre_pack_str(lrbuf, str_value, str_size, LRE_MOD_STRING_UTF8, &error)

		elif isinstance(i, int):
			int_value = PyLong_AsLongLongAndOverflow(i, &int_overflow)

			if not int_overflow:
				lre_pack_int(lrbuf, int_value, &error)
			else:
				return packbufferbigint(lrbuf, i)

		elif isinstance(i, bytes):
			PyBytes_AsStringAndSize(i, <char **> &str_value, &str_size)
			lre_pack_str(lrbuf, str_value, str_size, LRE_MOD_STRING_RAW, &error)

		elif isinstance(i, float):
			lre_pack_float(lrbuf, i, &error)

		elif isinstance(i, list):
			return packbuffer(lrbuf, i)

		else:
			raise ValueError('type <%s> is unsupported' % type(i).__name__)

		if error:
			raise ValueError(lre_strerror(error).decode('utf8'))


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
			packbuffer(self.lrbuf, key)
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

lre = LRE(4096)
pack = lre.pack
load = lre.load
dumps = pack
loads = load
