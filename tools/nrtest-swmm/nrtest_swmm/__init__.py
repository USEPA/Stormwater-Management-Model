# -*- coding: utf-8 -*-

#
#  __init__.py - nrtest_swmm module
#
#  Author:     Michael E. Tryby
#              US EPA - ORD/NRMRL
#

'''
Numerical regression testing (nrtest) plugin for comparing SWMM binary results
files and SWMM text based report files.
'''

import sys
if sys.version_info < (3,0):
    from itertools import izip as _zip
else:
    _zip = zip

# third party imports
import header_detail_footer as hdf
import numpy as np

# project imports
import nrtest_swmm.output_reader as ordr


__author__ = "Michael E. Tryby"
__copyright__ = "None"
__credits__ = "Colleen Barr, Maurizio Cingi, Mark Gray, David Hall, Bryant McDonnell"
__license__ = "CC0 1.0 Universal"

__version__ = "0.4.0"
__date__ = "November 14, 2017"

__maintainer__ = "Michael E. Tryby"
__email__ = "tryby.michael@epa.gov"
__status  = "Development"


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
    for (test, ref) in _zip(ordr.output_generator(path_test),
                            ordr.output_generator(path_ref)):

        if len(test[0]) != len(ref[0]):
            raise ValueError('Inconsistent lengths')

        # Skip over results if they are equal
        if (np.array_equal(test[0], ref[0])):
            continue

        else:
            np.testing.assert_allclose(test[0], ref[0], rtol, atol)

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

        for (test_line, ref_line) in _zip(hdf.parse(ftest, HEADER, FOOTER)[1],
                                          hdf.parse(fref, HEADER, FOOTER)[1]):

            if test_line != ref_line:
                return False

    return True
