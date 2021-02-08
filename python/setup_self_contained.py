"""
The default system packaged version
links the python lib <discodb._discodb>

against the c library <libdiscodb.so>

this version incorperates discodb without
the need for a system-wide libdiscodb

it still requires system-wide cmph <libcmph0.so>
library

"""

from setuptools import setup, Extension


discodb_module = Extension(
    'discodb._discodb',
    sources=[
        'discodbmodule.c',
        '../src/ddb_delta.c',
        "../src/ddb_deltalist.c",
        "../src/ddb_membuffer.c",
        "../src/ddb_queue.c",
        "../src/ddb.c",
        "../src/ddb_cmph.c",
        "../src/ddb_cnf.c",
        "../src/ddb_cons.c",
        "../src/ddb_huffman.c",
        "../src/ddb_list.c",
        "../src/ddb_map.c",
        "../src/ddb_view.c"
    ],
    include_dirs=['/usr/include', '../src'],
    libraries=['cmph']
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

