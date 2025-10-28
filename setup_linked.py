"""
The default system packaged version
links the python lib <discodb._discodb>

against the c library <libdiscodb.so>

this version incorperates discodb without
the need for a system-wide libdiscodb

it still requires system-wide cmph <libcmph0.so>
library

"""

from pathlib import Path
from setuptools import setup, Extension
import sys

setup_args = dict(
    ext_modules = [
        Extension(
            'discodb._discodb',
            sources=['python/discodbmodule.c'],
            libraries=["cmph", "discodb"],
            library_dirs=[
                "/opt/homebrew/lib"
            ],
            include_dirs=[
                "/opt/homebrew/include",
                Path(__file__).parent / "src",
            ]
        )
    ]
)
setup(**setup_args)
