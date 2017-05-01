# -*- coding: utf-8 -*-

# system imports
import itertools as it

# third party imports
import header_detail_footer as hdf
import numpy as np

# project import
import swmm_reader as sr


def swmm_allclose_compare(path_test, path_ref, rtol, atol):
    ''' 
    Compares results in two SWMM binary files. Using the comparison criteria 
    described in the numpy assert_allclose documentation. 
            
        (test_value - ref_value) <= atol + rtol * abs(ref_value) 
    
    Returns true if all of the results in the two binary files meet the 
    comparison criteria; otherwise, an AssertionError is thrown. 
    
    Numpy allclose is quite expensive to evaluate. Test and reference results 
    are checked to see if they are equal before being compared using the 
    allclose criteria. This reduces comparison times significantly. 
    
    Allclose raises the question, "To what values should the relative and 
    absolute tolerances be set?" In theory, this turns out to be a difficult 
    question to answer. 
    
    The choice of the allclose critera itself implies a pragmatic approach to 
    answering this question. In practice, the tolerances values should be 
    selected based on the purpose for running the test. 
    
    Values selected to screen for unanticipated changes in results can be more 
    stringent than those for engine development work that is likely to 
    significantly change benchmark results.   
    
    Arguments: 
        path_test - path to result file being tested
        path_ref  - path to reference result file
        rtol - relative tolerance
        atol - absolute tolerance
    
    Returns: 
        True or raises an error
                
    Raises:
        ValueError()
        AssertionError()
        ...
    '''
    for (test, ref) in it.izip(sr.reader(path_test), sr.reader(path_ref)):
        
        if test.size != ref.size:
            raise ValueError('Inconsistent lengths')
        
        # Skip over results if they are equal
        if (np.array_equal(test, ref)):
            continue
        
        else:
            np.testing.assert_allclose(test, ref, rtol, atol)

    return True

#def swmm_better_compare():
#    '''
#    If for some reason you don't like numpy.testing.assert_allclose() add a 
#    better function here. Be sure to add the entry point to the setup file so 
#    nrtest can find it at runtime. 
#    '''
#    pass

def swmm_report_compare(path_test, path_ref, rtol, atol):
    '''
    Compares results in two report files ignoring contents of header and footer.
    
    Arguments: 
        path_test - path to result file being tested
        path_ref  - path to reference result file
        rtol - ignored
        atol - ignored
    
    Returns: 
        True or False
        
    Raises: 
        HeaderError()
        FooterError()
        RunTimeError()
        ...
    '''
    HEADER = 4
    FOOTER = 4
    
    with open(path_test ,'r') as ftest, open(path_ref, 'r') as fref:
        
        for (test_line, ref_line) in it.izip(hdf.parse(ftest, HEADER, FOOTER)[1], 
                                             hdf.parse(fref, HEADER, FOOTER)[1]): 
        
            if test_line != ref_line: 
                return False

    return True 
