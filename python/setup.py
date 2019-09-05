import os
import sys
import shutil
from setuptools.extension import Extension
from setuptools import setup, find_packages

try:
    from Cython.Build import cythonize
except ImportError:
    print('Cython not installed.')
    print('Please, install Cython from http://www.cython.org')
    sys.exit(1)

scriptdir = os.path.dirname(os.path.abspath(__file__))

for fname in ['lre.h', 'pstdint.h']:
    src = os.path.normpath(os.path.join(scriptdir, '..', fname))
    dst = os.path.normpath(os.path.join(scriptdir, 'lre', fname))

    if os.path.exists(src):
        shutil.copy(src, dst)

with open(os.path.join(scriptdir, 'README.md')) as f:
    long_description = f.read()

ext = Extension('*', ['lre/*.pyx'], include_dirs=['.'])

classifiers = [
    'Development Status :: 3 - Alpha',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: BSD License',
    'Programming Language :: Cython',
    'Programming Language :: Python',
    'Programming Language :: C',
    'Topic :: Software Development :: Libraries',
    'Topic :: Database'
]

setup(name='lre',
      packages=find_packages(),
      version='0.0.2rc5',
      license='BSD License',
      author='Arthur Goncharuk',
      author_email='af3.inet@gmail.com',
      description='Fast (de)serializer for lexicographical composite keys',
      long_description=long_description,
      long_description_content_type="text/markdown",
      url="https://github.com/Positeral/lre/python",
      classifiers=classifiers,
      keywords='lre lexicographical serializer composite-keys binding',
      ext_modules=cythonize(ext, build_dir='build'),
      package_data={'lre': ['*.pyx', '*.pxd', '*.h']},
      headers=['lre/lre.h', 'lre/pstdint.h'],
      zip_safe=False
)
