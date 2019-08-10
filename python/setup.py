import os
from setuptools.extension import Extension
from setuptools import setup
from Cython.Build import cythonize

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

setup(name='lre',
      packages=['lre'],
      version='0.0.1',
      license='BSD License',
      author='Arthur Goncharuk',
      long_description=long_description,
      long_description_content_type="text/markdown",
      url="https://github.com/Positeral/lre/python",
      classifiers=classifiers,
      ext_modules=cythonize(Extension('lre', ['lre.pyx'],
                                      include_dirs=[includedir]))
)
