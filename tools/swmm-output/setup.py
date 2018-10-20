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
    version = "0.1.0-alpha",
    ext_modules = [
        Extension("_swmm_output",
            define_macros = [('swmm_output_EXPORTS', None)], 
            include_dirs = ['include'],
            sources = ['src/swmm_output.i', 'src/swmm_output.c', 'src/errormanager.c'],
            swig_opts=['-py3'],
            language='C'
        )
    ],
    package_dir = {'':'src'},  
    py_modules = ['swmm_output'],
      
    install_requires = [
        'enum34'
    ]
)
