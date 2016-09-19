# -*- coding: utf-8 -*-

# project import
from .swmm_reader import *

def reader(path_ref): 
    '''
    The function reader() is a generator designed to iterate through a swmm 
    binary file and yield element results. It is useful for comparing contents 
    of binary files for numerical regression testing. 
    
    The reader yields a numpy array containing the SWMM element result. 
    
    Arguments: 
        path_ref - path to result file
        
    Raises: 
        SWMM_Binary_Reader_Error()
        ...
    '''    
    with SWMM_BinaryReader(path_ref) as br:
        
        for period_index in range(0, br.report_periods()):
            for element_type in ElementType:
                for element_index in range(0, br.element_count(element_type)):

                    yield br.element_result(element_type, period_index, element_index)
                    