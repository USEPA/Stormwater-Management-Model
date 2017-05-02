# -*- coding: utf-8 -*-
'''
The function swmm_output_generator is used to iterate over a swmm binary file. 
'''

# project import
from .swmm_reader import *


__author__ = "Michael Tryby"
__copyright = "None"
__credits__ = "Colleen Barr, Maurizio Cingi, Mark Gray, David Hall, Bryant McDonnell"
__license__ = "CC0 1.0 Universal"

__version__ = "0.2.0"
__maintainer__ = "Michael Tryby"
__email__= "tryby.michael@epa.gov"
__status = "Development"


def swmm_output_generator(path_ref): 
    '''
    swmm_output_generator is designed to iterate over a swmm binary file and 
    yield element results. It is useful for comparing contents of binary files 
    for numerical regression testing. 
    
    The generator yields a numpy array containing the SWMM element result. 
    
    Arguments: 
        path_ref - path to result file
        
    Raises: 
        SWMM_OutputReaderError()
        ...
    '''    
    with SWMM_OutputReader(path_ref) as sor:
        
        for period_index in range(0, sor.report_periods()):
            for element_type in ElementType:
                for element_index in range(0, sor.element_count(element_type)):

                    yield sor.element_result(element_type, period_index, element_index)
                    