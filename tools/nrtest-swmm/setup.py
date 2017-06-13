# -*- coding: utf-8 -*-

'''
Created on Aug 11, 2016

@author: mtryby
'''

try: 
    from setuptools import setup
except ImportError:
    from distutils.core import setup


entry_points = {
    'nrtest.compare': [
        'swmm allclose = nrtest_swmm:swmm_allclose_compare',
        'swmm report = nrtest_swmm:swmm_report_compare',
        # Add the entry point for new comparison functions here
    ]
}


setup(
    name='nrtest-swmm',
    version='0.2.0',
    description="SWMM extension for nrtest",
    
    author="Michael E. Tryby",
    author_email='tryby.michael@epa.gov',
    url='https://github.com/USEPA',

    packages=['nrtest_swmm',],
    entry_points=entry_points,
    include_package_data=True,
    install_requires=[
        'header_detail_footer>=2.3',
        'nrtest>=0.2.0',
        'numpy>=1.6.0',
        'swmm_reader>=0.2.0',
    ],
    zipsafe=True,  
    keywords='nrtest_swmm'  
)
