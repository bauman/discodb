"""
for manual testing testing
cd ../python && pip3 uninstall discodb && python3 setup.py bdist_wheel && pip3 install dist/discodb*whl && cd ../cmake-build-debug && LD_LIBRARY_PATH="$(pwd)" python3
"""

from setuptools import setup, Extension


discodb_module = Extension(
    'discodb._discodb',
    sources=['discodbmodule.c'],
    include_dirs=['/usr/include', '../src'],
    library_dirs=['/usr/lib', '/usr/lib64', "../cmake-build-debug"],
    libraries=['discodb']
)

setup(name='discodb',
      version='0.9',
      description='An efficient, immutable, persistent mapping object.',
      author='Nokia Research Center',
      install_requires=[
          'six',
      ],
      ext_modules=[discodb_module],
      packages=['discodb'])

