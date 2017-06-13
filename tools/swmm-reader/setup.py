# -*- coding: utf-8 -*-

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup
    

setup(
    name = "swmm-reader",
    version = "0.2.0",
    description = "Tools for reading SWMM binary results file",
    
    author="Michael E. Tryby",
    author_email='tryby.michael@epa.gov',
    url='https://github.com/USEPA',
    
    packages = ['swmm_reader'],
    
    install_requires = ['numpy', 'enum34'],
    
    package_data = {
        'swmm_reader':['*.dll']
    },
    include_package_data = True,  
      
    zip_safe = False,
    keywords='swmm_reader'
)
