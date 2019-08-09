from cython cimport final
from libc.stdint cimport uint8_t, int64_t

cdef extern from '../../lre.h':
	cdef int LRE_OK
	cdef int LRE_FAIL

	ctypedef enum lre_error_t:
		LRE_ERROR_NOTHING
		LRE_ERROR_ALLOCATION,
		LRE_ERROR_NULLPTR,
		LRE_ERROR_RANGE,
		LRE_ERROR_NAN,
		LRE_ERROR_LENGTH,
		LRE_ERROR_TAG,
		LRE_ERROR_SIGN,
		LRE_ERROR_MOD,
		LRE_ERROR_HANDLER

	ctypedef enum lre_mod_t:
		LRE_MOD_DEFAULT,
		LRE_MOD_STRING_RAW,
		LRE_MOD_STRING_UTF8

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

	const char *lre_strerror(lre_error_t error)


cdef class Error(Exception):
	pass


@final
cdef class LRE:
	cdef lre_error_t   lrerror
	cdef lre_buffer_t *lrbuf

	def __cinit__(self, int reserve):
		self.lrbuf = lre_buffer_create(reserve, &self.lrerror)

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
					lre_pack_str(self.lrbuf, <const uint8_t *> i, len(i), LRE_MOD_STRING_UTF8, &error)

				elif isinstance(i, int):
					lre_pack_int(self.lrbuf, <int64_t> i, &error)

				elif isinstance(i, bytes):
					lre_pack_str(self.lrbuf, <const uint8_t *> i, len(i), LRE_MOD_STRING_RAW, &error)

				elif isinstance(i, float):
					lre_pack_float(self.lrbuf, <double> i, &error)

				else:
					raise ValueError('type <%s> is unsupported' % type(i).__name__)

				if error:
					raise ValueError(lre_strerror(error).decode('utf8'))

			return self.lrbuf.data[:self.lrbuf.size]
		finally:
			if lre_buffer_reset(self.lrbuf, &error) != LRE_OK:
				raise MemoryError(lre_strerror(error).decode('utf8'))

	cpdef load(self, key):
		pass


lre = LRE(1)
pack = lre.pack
load = lre.load
dumps = pack
loads = load
