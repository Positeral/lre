import os
import sys
from setuptools.extension import Extension
from setuptools import setup, find_packages

try:
    from Cython.Build import cythonize
except ImportError:
    print('Cython not installed.')
    print('Please, install Cython from http://www.cython.org')
    sys.exit(1)

scriptdir = os.path.dirname(os.path.abspath(__file__))
includedir = os.path.normpath(os.path.join(scriptdir, '..'))

with open(os.path.join(scriptdir, 'README.md')) as f:
    long_description = f.read()

classifiers = [
    'Development Status :: 3 - Alpha',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: BSD License',
    'Programming Language :: Cython',
    'Programming Language :: Python'
]

ext = Extension('lre.clre', ['lre/lre.pyx'], include_dirs=[includedir])

setup(name='lre',
      packages=find_packages(),
      version='0.0.1',
      license='BSD License',
      author='Arthur Goncharuk',
      description='Fast (de)serializer for lexicographical composite keys',
      long_description=long_description,
      long_description_content_type="text/markdown",
      url="https://github.com/Positeral/lre/python",
      classifiers=classifiers,
      ext_modules=cythonize(ext, build_dir='build'),
      package_data={'lre': ['*.pyx', '*.pxd']}
)
