cimport cython


cdef extern from *:
	ctypedef struct PyObject
	ctypedef struct PyLongObject

	object PyBytes_FromStringAndSize(const char *v, Py_ssize_t len)
	const char *PyUnicode_AsUTF8AndSize(object unicode, Py_ssize_t *size) except? NULL
	int PyBytes_AsStringAndSize(object obj, char **buffer, Py_ssize_t *length) except? -1
	long long PyLong_AsLongLongAndOverflow(object obj, int *overflow) except? -1

	# https://stackoverflow.com/questions/18290507/python-extension-construct-and-inspect-large-integers-efficiently
	# https://mail.python.org/pipermail/python-list/2006-August/372367.html
	# https://github.com/python/cpython/blob/3.7/Include/longobject.h#L127
	object _PyLong_FromByteArray(const unsigned char* bytes, size_t n, int little_endian, int is_signed)

	# https://github.com/python/cpython/blob/3.7/Include/longobject.h#L144
	int _PyLong_AsByteArray(PyLongObject *v, unsigned char *bytes, size_t n, int little_endian, int is_signed) except? -1

	# https://github.com/python/cpython/blob/3.7/Include/longobject.h#L110
	size_t _PyLong_NumBits(object v) except *


# From lre.h
DEF LRE_OK   = 0
DEF LRE_FAIL = 1

# Thanks to GIL we can avoid memory allocation
cdef uint8_t TMP65535[65535]


@cython.final
cdef class LRE:
	def __cinit__(self, int reserve):
		cdef lre_error_t error = LRE_ERROR_NOTHING

		self.lrbuffer = lre_buffer_create(reserve, &error)

		if error:
			raise MemoryError(lre_strerror(error).decode('utf8'))

		lre_loader_init(&self.lrloader, <void *> self)
		self.lrloader.handler_int    = &self.callback_load_int
		self.lrloader.handler_float  = &self.callback_load_float
		self.lrloader.handler_str    = &self.callback_load_str
		self.lrloader.handler_bigint = &self.callback_load_bigint

	cpdef pack(self, key):
		cdef lre_error_t error = LRE_ERROR_NOTHING
		lre_buffer_reset_fast(self.lrbuffer)

		try:
			self.buffer_write(key, 0)
			return (<char *> self.lrbuffer.data)[:self.lrbuffer.size]
		finally:
			if lre_buffer_reset(self.lrbuffer, &error) != LRE_OK:
				raise MemoryError(lre_strerror(error).decode('utf8'))

	cpdef load(self, key):
		cdef lre_error_t error = LRE_ERROR_NOTHING
		cdef const char *src

		try:
			self.tmpkey = []

			if isinstance(key, bytes):
				src = key
			elif isinstance(key, unicode):
				key = key.encode('utf8')
				src = key
			else:
				raise TypeError("a bytes or unicode object is required, not '%s'" % type(key).__name__)

			if lre_tokenize(&self.lrloader, <uint8_t *> src, len(key), &error) != LRE_OK:
				raise ValueError(lre_strerror(error).decode('utf8'))

			return self.tmpkey
		finally:
			self.tmpkey = []

	cdef buffer_write(self, key, int depth):
		cdef lre_error_t    error = LRE_ERROR_NOTHING
		cdef const uint8_t *str_value
		cdef Py_ssize_t     str_size

		if depth > 32:
			raise ValueError('maximum depth exceeded')

		if not isinstance(key, list):
			key = [key]
	
		for i in key:
			if isinstance(i, unicode):
				str_value = <const uint8_t *> PyUnicode_AsUTF8AndSize(i, &str_size)
				lre_pack_str(self.lrbuffer, str_value, str_size, LRE_ENC_UTF8, &error)

			elif isinstance(i, float):
				lre_pack_float(self.lrbuffer, i, &error)

			elif isinstance(i, int):
				self.buffer_write_int(i)
	
			elif isinstance(i, bytes):
				PyBytes_AsStringAndSize(i, <char **> &str_value, &str_size)
				lre_pack_str(self.lrbuffer, str_value, str_size, LRE_ENC_RAW, &error)
	
			elif isinstance(i, list):
				self.buffer_write(i, depth + 1)
	
			else:
				raise ValueError('type <%s> is unsupported' % type(i).__name__)
	
			if error:
				raise ValueError(lre_strerror(error).decode('utf8'))

	cdef buffer_write_int(self, pyint):
		cdef lre_error_t error        = LRE_ERROR_NOTHING
		cdef int         int_overflow = 0
		cdef int64_t     int_value    = PyLong_AsLongLongAndOverflow(pyint, &int_overflow)

		if not int_overflow:
			if lre_pack_int(self.lrbuffer, int_value, &error) != LRE_OK:
				raise ValueError(lre_strerror(error).decode('utf8'))
			else:
				return LRE_OK

		cdef uint8_t *dst
		cdef size_t   nbytes = (_PyLong_NumBits(pyint) + 7) >> 3

		if nbytes > 65535:
			raise OverflowError('big int out of range')
	
		# tag(1) + nbytes(4) + value(nbytes*2) + separator(1)
		if lre_buffer_require(self.lrbuffer, (1+4+(nbytes*2)+1), &error) != LRE_OK:
			raise MemoryError(lre_strerror(error).decode('utf8'))

		dst = lre_buffer_end(self.lrbuffer)

		if pyint < 0:
			pyint = -pyint
			_PyLong_AsByteArray(<PyLongObject *> pyint, <unsigned char *> TMP65535, nbytes, 0, 0)

			lrex_write_char  (&dst, LRE_TAG_NUMBER_NEGATIVE_BIG)
			lrex_write_uint16(&dst, ~nbytes)
			lrex_write_str   (&dst, TMP65535, nbytes, 0xff)
			lrex_write_char  (&dst, LRE_SEP_NEGATIVE)
		else:
			_PyLong_AsByteArray(<PyLongObject *> pyint, <unsigned char *> TMP65535, nbytes, 0, 0)

			lrex_write_char  (&dst, LRE_TAG_NUMBER_POSITIVE_BIG)
			lrex_write_uint16(&dst, nbytes)
			lrex_write_str   (&dst, TMP65535, nbytes, 0)
			lrex_write_char  (&dst, LRE_SEP_POSITIVE)
	
		lre_buffer_set_size_distance(self.lrbuffer, dst)

	@staticmethod # Call by lre_tokenize()
	cdef int callback_load_int(lre_loader_t *loader, int64_t value) except? LRE_FAIL:
		cdef LRE self = <LRE> loader.app_private

		self.tmpkey.append(value)
		return LRE_OK

	@staticmethod # Call by lre_tokenize()
	cdef int callback_load_float(lre_loader_t *loader, double value) except? LRE_FAIL:
		cdef LRE self = <LRE> loader.app_private

		self.tmpkey.append(value)
		return LRE_OK

	@staticmethod # Call by lre_tokenize()
	cdef int callback_load_str(lre_loader_t *loader, lre_slice_t *slice, lre_enc_t enc) except? LRE_FAIL:
		cdef LRE       self   = <LRE> loader.app_private
		cdef ptrdiff_t nbytes = lre_slice_len(slice) >> 1
		cdef bytes     s      = PyBytes_FromStringAndSize(NULL, nbytes)

		lrex_read_str(&slice.src, <uint8_t *> (<char *> s), nbytes, 0)

		if enc == LRE_ENC_UTF8:
			self.tmpkey.append(s.decode('utf8'))
		else:
			self.tmpkey.append(s)

		return LRE_OK

	@staticmethod # Call by lre_tokenize()
	cdef int callback_load_bigint(lre_loader_t *loader, const lre_metanumber_t *num) except? LRE_FAIL:
		cdef LRE            self = <LRE> loader.app_private
		cdef const uint8_t *src  = num.integral_data
		cdef object         value

		if num.integral_nbytes > 65535:
			raise OverflowError('big int out of range')

		if num.negative_mask:
			lrex_read_str(&src, TMP65535, num.integral_nbytes, 0xff)
			value = _PyLong_FromByteArray(<unsigned char *> TMP65535, num.integral_nbytes, 0, 0)
			value = -value
		else:
			lrex_read_str(&src, TMP65535, num.integral_nbytes, 0x00)
			value = _PyLong_FromByteArray(<unsigned char *> TMP65535, num.integral_nbytes, 0, 0)

		self.tmpkey.append(value)

		return LRE_OK


