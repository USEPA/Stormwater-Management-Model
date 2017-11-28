# -*- coding: utf-8 -*-

#
#  setup.py 
# 
#  Author:     Michael E. Tryby
#              US EPA - ORD/NRMRL
#
#

'''Setup up script for SWMM Output API python extension'''

try:
    from setuptools import setup, Extension
    from setuptools.command.build_ext import build_ext
except ImportError:
    from distutils.core import setup, Extension
    from distutils.command.build_ext import build_ext


setup(
    name = "swmm-output",
    version = "1.0",
    ext_modules = [
        Extension("_swmm_output", 
            sources = ['src/swmm_output.i', 'src/swmm_output.c', 'src/errormanager.c'],
            swig_opts=['-modern'],
            language='C'
        )
    ],
    package_dir = {'':'src'},  
    py_modules = ['swmm_output'],
      
    install_requires = [
        'enum34'
    ]
)
