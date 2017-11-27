# -*- coding: utf-8 -*-

#
#  main.py 
# 
#  Author:     Michael E. Tryby
#              US EPA - ORD/NRMRL
#
'''
Provides entry point main. Useful for development and testing purposes. 
'''

import cStringIO
import itertools as it
import time

import header_detail_footer as hdf
import numpy as np

import nrtest_swmm.output_reader as ordr

def result_compare(path_test, path_ref, comp_args):

    isclose = True
    close = 0
    notclose = 0
    equal = 0
    total = 0
    output = cStringIO.StringIO()
    eps =  np.finfo(float).eps
    
    start = time.time()
    
    test_reader = ordr.output_generator(path_test)
    ref_reader = ordr.output_generator(path_ref)
    
    for test, ref in it.izip(test_reader, ref_reader):
        total += 1
        if total%100000 == 0:
        print(total)
        
        if len(test) != len(ref):
            raise ValueError('Inconsistent lengths')
        
        # Skip results if they are zero or equal
        if np.array_equal(test, ref):
            equal += 1
            continue
        else:
            try:
                np.testing.assert_allclose(test, ref, comp_args[0], comp_args[1])
                close += 1
                
            except AssertionError as ae:
                notclose += 1
                output.write(str(ae))
                output.write('\n\n')
                continue       
        
    stop = time.time()
    
    print(output.getvalue())
    output.close()
    
    print('equal: %d  close: %d  notclose: %d  total: %d  in %f (sec)\n' % 
          (equal, close, notclose, total, (stop - start)))

    if notclose > 0:
        print('%d differences found\n' % notclose)
        isclose = False

    return isclose

def array_zero(test, ref):
    if not test.any() and not ref.any():
        return True
    return False

def report_compare(path_test, path_ref, (comp_args)):
    '''
    Compares results in two report files ignoring contents of header and footer. 
    '''
    with open(path_test ,'r') as ftest, open(path_ref, 'r') as fref:
        for (test_line, ref_line) in it.izip(hdf.parse(ftest, 4, 4)[1], 
                                             hdf.parse(fref, 4, 4)[1]): 
            if test_line != ref_line: 
                return False
              
    return True 


import logging    
from os import listdir
from os.path import exists, isfile, isdir, join

from nrtest.testsuite import TestSuite
from nrtest.compare import compare_testsuite, validate_testsuite
from nrtest.execute import execute_testsuite
    
def nrtest_compare(path_test, path_ref, rtol, atol): 

    ts_new = TestSuite.read_benchmark(path_test)
    ts_old = TestSuite.read_benchmark(path_ref)

    if not validate_testsuite(ts_new) or not validate_testsuite(ts_old):
        exit(1)

    try:
        logging.info('Found %i tests' % len(ts_new.tests))
        compatible = compare_testsuite(ts_new, ts_old, rtol, atol)
    except KeyboardInterrupt:
        logging.warning('Process interrupted by user')
        compatible = False
    else:
        logging.info('Finished')

    # Non-zero exit code indicates failure
    exit(not compatible)

def nrtest_execute(app_path, test_path, output_path):


    for p in test_path + [app_path]:
        if not exists(p):
            logging.error('Could not find path: "%s"' % p)

    test_dirs = [p for p in test_path if isdir(p)]
    test_files = [p for p in test_path if isfile(p)]
    test_files += [join(d, p) for d in test_dirs for p in listdir(d)
                   if p.endswith('.json')]

    test_files = list(set(test_files))  # remove duplicates

    ts = TestSuite.read_config(app_path, test_files, output_path)

    if not validate_testsuite(ts):
        exit(1)

    try:
        logging.info('Found %i tests' % len(test_files))
        success = execute_testsuite(ts)
        ts.write_manifest()
    except KeyboardInterrupt:
        logging.warning('Process interrupted by user')
        success = False
    else:
        logging.info('Finished')

    # Non-zero exit code indicates failure
    exit(not success)


if __name__ == "__main__":

    import sys
    
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)
    
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    root.addHandler(ch)   
    
#    app_path = "C:\\Users\\mtryby\\Workspace\\GitRepo\\michaeltryby\\Stormwater-Management-Model\\tests\\swmm-nrtestsuite\\apps\\swmm-517_x86.json"
#    test_path = "C:\\Users\\mtryby\\Workspace\\GitRepo\\michaeltryby\\Stormwater-Management-Model\\tests\\swmm-nrtestsuite\\tests\\examples\\example1.json"
#    output_path = "C:\\Users\\mtryby\\Workspace\\GitRepo\\michaeltryby\\Stormwater-Management-Model\\tests\\swmm-nrtestsuite\\benchmarks\\test"
#    nrtest_execute(app_path, [test_path], output_path)
 
#    test_path = "C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\swmm-testsuite\\benchmarks\\v5111_x64"
#    ref_path  = "C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\swmm-testsuite\\benchmarks\\v5111_x86"
#    nrtest_compare(test_path, ref_path, 0.001, 0.0)

    
#    path_test = "C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\swmm-testsuite\\benchmarks\\v5111\\Example_4\\Example4.out"
#    path_ref  = "C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\swmm-testsuite\\benchmarks\\v5110\\Example_4\\Example4.out"    
#    result_compare(path_test, path_ref, (0.001, 0.0))

    path_test = "C:\\Users\\mtryby\\Workspace\\GitRepo\\michaeltryby\\Stormwater-Management-Model\\tests\\swmm-nrtestsuite\\benchmark\\swmm-5112"
    path_ref  = "C:\\Users\\mtryby\\Workspace\\GitRepo\\michaeltryby\\Stormwater-Management-Model\\tests\\swmm-nrtestsuite\\benchmark\\swmm-520dev1"
    nrtest_compare(path_test, path_ref, 0.0, 1.0)
#    print(result_compare(path_test, path_ref, (0.0, 0.1)))
