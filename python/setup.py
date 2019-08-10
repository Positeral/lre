import os
from distutils.extension import Extension
from distutils.core import setup
from Cython.Build import cythonize

scriptdir = os.path.dirname(os.path.abspath(__file__))
includedir = os.path.normpath(os.path.join(scriptdir, '..'))

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
      classifiers=classifiers,
      ext_modules=cythonize(Extension('lre', ['lre.pyx'],
                                      include_dirs=[includedir]))
)
