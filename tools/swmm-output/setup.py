# -*- coding: utf-8 -*-
#
#  setup.py 
# 
#  Author:     Michael E. Tryby
#              US EPA - ORD/NRMRL
#
# Setup up script for SWMM Output API python extension
#
# Requires:
#   Platform C language compiler   
#   Python packages: numpy
#

try:
    from setuptools import setup, Extension
    from setuptools.command.build_ext import build_ext
except ImportError:
    from distutils.core import setup, Extension
    from distutils.command.build_ext import build_ext

class CustomBuildExtCommand(build_ext):
    """build_ext command for use when numpy headers are needed."""
    def run(self):
        # Import numpy here, only when headers are needed
        import numpy

        # Add numpy headers to include_dirs
        try:
            numpy_include = numpy.get_include()
        except AttributeError:
            numpy_include = numpy.get_numpy_include()

        self.include_dirs.append(numpy_include)
        
        # Call original build_ext command
        build_ext.run(self)    

setup(
    name = "swmm-output", 
    cmdclass = {'build_ext': CustomBuildExtCommand}, 
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
        'numpy>=1.7.0'
    ]
)
