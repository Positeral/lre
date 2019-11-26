# LRE / Python

Data types:
* String (unicode, bytes)
* Integer
* Float
* +INF and -INF are supported

Limits:
* NaN not supported due to ambiguity
* No difference between 0, -0.0 and +0.0, between 1 and 1.0
* Big integers are limited by 524280 bits (2^524280 - 1)

### Installation from source

First you'll need [Cython](https://cython.org) (e.g. `pip install cython`).
```
pip install git+https://github.com/Positeral/lre.git#subdirectory=python
```

### Usage
```python
>>> lre.dumps(['email', 'home@cern'])
b'XgfgngbgjgmL+XgigpgngfeagdgfhcgoL+'
```

Ordering serialized numbers numerically:
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

Because the LRE format purposely is **flat** by design, it makes no difference between nested lists. Empty lists are ignored:
```python
>>> lre.dumps(['user', 'admin', 1])
b'XhfhdgfhcL+XgbgegngjgoL+Mab+'
>>> lre.dumps(['user', ['admin'], [[1]], []])
b'XhfhdgfhcL+XgbgegngjgoL+Mab+'
```

Also `'abc'` is it the same as `['abc']`:
```python
>>> lre.dumps('key')
b'XglgfhjL+'
>>> lre.dumps(['key'])
b'XglgfhjL+'
```

This "flat behavior" is necessary for easy and unambiguous key concatenation.

### License
Python binding of LRE is licensed under the BSD 2-Clause License.
