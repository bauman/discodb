"""
for manual testing testing
cd ../python && pip3 uninstall discodb && python3 setup.py bdist_wheel && pip3 install dist/discodb*whl && cd ../cmake-build-debug && LD_LIBRARY_PATH="$(pwd)" python3
"""
from os import getcwd

from setuptools import setup, Extension
import glob
from pathlib import Path
setup_args = dict(
    ext_modules = [
        Extension(
            'discodb._discodb',
            sources=['python/discodbmodule.c'] + glob.glob("src/*.c"),
            libraries=["cmph"],
            library_dirs=[],
            include_dirs=[
                Path(__file__).parent / "src",
                "/opt/homebrew/include"
            ]
        )
    ]
)
setup(**setup_args)

# setup(name='discodb',
#       version='0.9.1',
#       description='An efficient, immutable, persistent mapping object.',
#       author='Nokia Research Center',
#       install_requires=[
#       ],
#       ext_modules=[discodb_module],
#       packages=['discodb'])
#
