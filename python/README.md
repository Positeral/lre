# LRE / Python

LRE (Lexicographic REpresentation) - is a rapid serializer of numbers and strings for composite keys. It converts a set of values to special ASCII-string providing a natural order for serialized numbers in lexicographically comparing, so deserialization for every key comparing is not required. This is very helpful for key-value databases like LMDB or Berkeley DB which stores arbitrary keys as byte arrays in lexicographical order by default.

Data types:
* String (unicode, bytes)
* Integer
* Float
* Decimal
* +INF and -INF are supported

Limits:
* NaN not supported due to ambiguity
* No difference between 0, -0.0 and +0.0, between 1 and 1.0
* Big integers are limited by 524280 bits (2^524280 - 1)

### Usage
```python
>>> lre.dumps(['email', 'home@cern'])
b'X656d61696cL+X686f6d65406365726eL+'
```

Natural comparing of numbers:
```python
>>> lre.dumps(123) == lre.dumps(123.0)
True
>>> lre.dumps(-1) < lre.dumps(0) < lre.dumps(1)
True
>>> lre.dumps(float('inf')) > lre.dumps(2**100)
True
>>> sorted([10, -1, -10, 2, 100, -10.5, 0], key=lre.dumps)
[-10.5, -10, -1, 0, 2, 10, 100]
```

Because LRE format purposely is **flat** by design, it makes no difference between nested lists. Empty lists are ignored:
```python
>>> lre.dumps(['user', 'admin', 1])
b'X75736572L+X61646d696eL+M01+'
>>> lre.dumps(['user', ['admin'], [[1]], []])
b'X75736572L+X61646d696eL+M01+'
```

Also `'abc'` is it the same as `['abc']`:
```python
>>> lre.dumps('key')
b'X6b6579L+'
>>> lre.dumps(['key'])
b'X6b6579L+'
```

This "flat behavior" is necessary for easy and unambiguous key concatenation.

### License
Python binding of LRE is licensed under BSD 2-Clause License.