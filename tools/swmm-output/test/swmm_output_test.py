# -*- coding: utf-8 -*-

#   swmm_output_test.py
#
#   Created: 11/8/2017
#   Author: Michael E. Tryby
#           US EPA - ORD/NRMRL
#
#   Unit testing for SWMM outputapi using pytest.
#

import pytest
import numpy as np

import swmm_output as oapi

from data import OUTPUT_FILE_EXAMPLE1
    
@pytest.fixture()
def smo_open(request):    
    _handle = oapi.smo_init()
    oapi.smo_open(_handle, OUTPUT_FILE_EXAMPLE1)
    
    def smo_close():
        oapi.smo_close()
        _handle = None
    
    request.addfinalizer(smo_close)    
    return _handle    


def test_get_version(smo_open):
    version = oapi.smo_get_version(smo_open)
    assert version == 51000
 
def test_get_project_size(smo_open):
    count = oapi.smo_get_project_size(smo_open)
    assert count == [8, 14, 13, 2]
     
def test_get_flow_units(smo_open):
    units = oapi.smo_get_flow_units(smo_open)
    assert units == 0
     
def test_get_pollutant_units(smo_open):
    units = oapi.smo_get_pollutant_units(smo_open)    
    assert units == [0, 1]
     
def test_get_element_name(smo_open):
    name = oapi.smo_get_element_name(smo_open, oapi.ElementType.SUBCATCH, 2)
    assert name == "3"
     
    name = oapi.smo_get_element_name(smo_open, oapi.ElementType.NODE, 7)
    assert name == "19"
 
def test_get_subcatch_result(smo_open):
     ref = np.array([0.5, 0.0, 0.0, 0.125, 1.24382424, 0.0, 
         0.0, 0.0, 33.48199081, 6.69639826])
     
     result = oapi.smo_get_subcatch_result(smo_open, 1, 1)
     assert len(result) == 10
     assert np.allclose(result, ref)

def test_get_node_result(smo_open):
    ref = np.array([0.296234, 995.296204, 0.0, 1.302650, 1.302650,
        0.0, 15.361463, 3.072293])
    
    result = oapi.smo_get_node_result(smo_open, 2, 2)
    assert len(result) == 8
    assert np.allclose(result, ref)
                
def test_get_link_result(smo_open):
    ref = np.array([4.631762, 1.0, 5.8973422, 314.15927,
        1.0, 19.070757, 3.8141515])
    result = oapi.smo_get_link_result(smo_open, 3, 3)
    assert len(result) == 7
    assert np.allclose(result, ref)
        
def test_get_system_result(smo_open): 
    ref = np.array([70.0, 0.1, 0.0, 0.19042271, 14.172027, 0.0,
        0.0, 0.0, 0.0, 14.172027, 0.55517411, 13.622702, 2913.0793, 0.0])
    
    result = oapi.smo_get_system_result(smo_open, 4, 4)
    assert len(result) == 14
    assert np.allclose(result, ref)       
