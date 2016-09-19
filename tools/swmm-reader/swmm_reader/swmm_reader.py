# -*- coding: utf-8 -*-

# system imports
import ctypes

# third party imports
import enum
import numpy as np

# project import
import outputapi as oapi


_err_max_char = 80

class ElementType(enum.Enum):
    subcatch = oapi.subcatch
    node = oapi.node
    link = oapi.link
    system = oapi._sys
   
class SWMM_BinaryReaderError(Exception):
    '''
    Custom exception class for SWMM errors.
    '''
    def __init__(self, error_code, error_message):
        self.warning = False
        self.args = (error_code,)
        self.message = error_message
            
    def __str__(self):
        return self.message
                    
class SWMM_BinaryReader():    
    ''' 
    Provides a minimal API used to implement the SWMM result generator. 
    '''
    def __init__(self, filename):
        self.filepath = filename
        self.ptr_api = ctypes.c_void_p
        self.ptr_resultbuff = ctypes.c_void_p
        self.bufflength = ctypes.c_long()
        self.getElementResult = {ElementType.subcatch: oapi.SMO_getSubcatchResult, 
                                 ElementType.node: oapi.SMO_getNodeResult, 
                                 ElementType.link: oapi.SMO_getLinkResult, 
                                 ElementType.system: oapi.SMO_getSystemResult}

    def __enter__(self):     
        self.ptr_api = oapi.SMO_init()
        self._error_check(oapi.SMO_open(self.ptr_api, ctypes.c_char_p(self.filepath.encode())))

        # max system result is vector with 15 elements so should be adequate for result buffer
        # TODO: What about when there are more than six pollutants defined?
        error = ctypes.c_long()
        self.ptr_resultbuff = oapi.SMO_newOutValueArray(self.ptr_api, ctypes.c_int(oapi.getResult), 
                                                        ctypes.c_int(ElementType.system.value), 
                                                        ctypes.byref(self.bufflength), ctypes.byref(error))
        self._error_check(error.value)
        return self

    def __exit__(self, type, value, traceback):
        oapi.SMO_free(self.ptr_resultbuff)
        self._error_check(oapi.SMO_close(self.ptr_api))

    def _error_message(self, code):
        error_code = ctypes.c_int(code)
        error_message = oapi.String(ctypes.create_string_buffer(_err_max_char))
        oapi.SMO_errMessage(error_code, error_message, _err_max_char)
        return error_message.data

    def _error_check(self, err):
        if err != 0:
            raise SWMM_BinaryReaderError(err, self._error_message(err))
            
    def report_periods(self):
        num_periods = ctypes.c_int()
        self._error_check(oapi.SMO_getTimes(self.ptr_api, oapi.numPeriods, ctypes.byref(num_periods)))
        return num_periods.value
    
    def element_count(self, element_type):
        count = ctypes.c_int()
        self._error_check(oapi.SMO_getProjectSize(self.ptr_api, ctypes.c_int(element_type.value), ctypes.byref(count)))
        return count.value
 
    def element_result(self, element_type, time_index, element_index):
        time_idx = ctypes.c_long(time_index)
        element_idx = ctypes.c_int(element_index)
        count = ctypes.c_int()
        self._error_check(self.getElementResult[element_type](self.ptr_api, time_idx, element_idx, self.ptr_resultbuff, ctypes.byref(count)))
        return np.fromiter((self.ptr_resultbuff[i] for i in range(count.value)), np.float, count.value)
