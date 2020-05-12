/*
 *  swmm_output.i - SWIG interface description file for SWMM Output API
 * 
 *  Created:    11/3/2017
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 *
*/

%module swmm_output
%{
#include "errormanager.h"
#include "messages.h"
#include "swmm_output.h"

#define SWIG_FILE_WITH_INIT
%}

%include "typemaps.i"

/* DEFINE AND TYPEDEF MUST BE INCLUDED */
typedef void* SMO_Handle;

typedef enum {
    SMO_flow_rate,
    SMO_concentration
} SMO_unit;

typedef enum {
    SMO_subcatch,
    SMO_node,
    SMO_link,
    SMO_sys
} SMO_elementType;

typedef enum {
    SMO_reportStep,
    SMO_numPeriods
} SMO_time;

typedef enum {
    SMO_rainfall_subcatch,      // (in/hr or mm/hr),
    SMO_snow_depth_subcatch,    // (in or mm),
    SMO_evap_loss,              // (in/hr or mm/hr),
    SMO_infil_loss,             // (in/hr or mm/hr),
    SMO_runoff_rate,            // (flow units),
    SMO_gwoutflow_rate,         // (flow units),
    SMO_gwtable_elev,           // (ft or m),
    SMO_soil_moisture,          // unsaturated zone moisture content (-),
    SMO_pollutant_conc_subcatch // first pollutant
} SMO_subcatchAttribute;

typedef enum {
    SMO_invert_depth,           // (ft or m),
    SMO_hydraulic_head,         // (ft or m),
    SMO_stored_ponded_volume,   // (ft3 or m3),
    SMO_lateral_inflow,         // (flow units),
    SMO_total_inflow,           // lateral + upstream (flow units),
    SMO_flooding_losses,        // (flow units),
    SMO_pollutant_conc_node     // first pollutant,
} SMO_nodeAttribute;

typedef enum {
    SMO_flow_rate_link,         // (flow units),
    SMO_flow_depth,             // (ft or m),
    SMO_flow_velocity,          // (ft/s or m/s),
    SMO_flow_volume,            // (ft3 or m3),
    SMO_capacity,               // (fraction of conduit filled),
    SMO_pollutant_conc_link     // first pollutant,
} SMO_linkAttribute;

typedef enum {
    SMO_air_temp,               // (deg. F or deg. C),
    SMO_rainfall_system,        // (in/hr or mm/hr),
    SMO_snow_depth_system,      // (in or mm),
    SMO_evap_infil_loss,        // (in/hr or mm/hr),
    SMO_runoff_flow,            // (flow units),
    SMO_dry_weather_inflow,     // (flow units),
    SMO_groundwater_inflow,     // (flow units),
    SMO_RDII_inflow,            // (flow units),
    SMO_direct_inflow,          // user defined (flow units),
    SMO_total_lateral_inflow,   // (sum of variables 4 to 8) //(flow units),
    SMO_flood_losses,           // (flow units),
    SMO_outfall_flows,          // (flow units),
    SMO_volume_stored,          // (ft3 or m3),
    SMO_evap_rate               // (in/day or mm/day),
    //p_evap_rate               // (in/day or mm/day)
} SMO_systemAttribute;


#ifdef WINDOWS
  #ifdef __cplusplus
  #define DLLEXPORT __declspec(dllexport) __cdecl
  #else
  #define DLLEXPORT __declspec(dllexport) __stdcall
  #endif
#else
  #define DLLEXPORT
#endif


/* TYPEMAPS FOR VOID POINTER */
/* Used for functions that output a new opaque pointer */
%typemap(in, numinputs=0) SMO_Handle* p_handle_out (void* retval)
{
 /* OUTPUT in */
    retval = NULL;
    $1 = &retval;
}
/* used for functions that take in an opaque pointer (or NULL)
and return a (possibly) different pointer */
%typemap(argout) SMO_Handle* p_handle_out
{
 /* OUTPUT argout */
    %append_output(SWIG_NewPointerObj(SWIG_as_voidptr(retval$argnum), $1_descriptor, 0));
}

%typemap(in) SMO_Handle* p_handle_inout (SMO_Handle retval)
{
   /* INOUT in */
   SWIG_ConvertPtr($input, SWIG_as_voidptrptr(&retval), 0, 0);
   $1 = &retval;
}

/* No need for special IN typemap for opaque pointers, it works anyway */


/* TYPEMAP FOR IGNORING INT ERROR CODE RETURN VALUE */
%typemap(out) int {
    $result = Py_None;
    Py_INCREF($result);
}


/* TYPEMAPS FOR DOUBLE ARGUMENT AS RETURN VALUE */
%typemap(in, numinputs=0) double* double_out (double temp) {
    $1 = &temp;
}
%typemap(argout) double* double_out {
    %append_output(PyFloat_FromDouble(*$1));
}


/* TYPEMAPS FOR INT ARGUMENT AS RETURN VALUE */
%typemap(in, numinputs=0) int* int_out (int temp) {
    $1 = &temp;
}
%typemap(argout) int* int_out {
    %append_output(PyInt_FromLong(*$1));
}


/* TYPEMAP FOR MEMORY MANAGEMENT AND ENCODING OF STRINGS */
%typemap(in, numinputs=0)char** string_out (char* temp), int* slen (int temp){
   $1 = &temp;
}
%typemap(argout)(char** string_out, int* slen) {
    if (*$1) {
        PyObject* o;
        o = PyUnicode_FromStringAndSize(*$1, *$2);
        
        $result = SWIG_Python_AppendOutput($result, o);
        free(*$1);
    }
}

/* TYPEMAPS FOR MEMORY MANAGEMNET OF FLOAT ARRAYS */
%typemap(in, numinputs=0)float** float_out (float* temp), int* int_dim (int temp){
   $1 = &temp;
}
%typemap(argout) (float** float_out, int* int_dim) {
    if (*$1) {
      PyObject *o = PyList_New(*$2);
      int i;
      float* temp = *$1;
      for(i=0; i<*$2; i++) {
        PyList_SetItem(o, i, PyFloat_FromDouble((double)temp[i]));
      }
      $result = SWIG_Python_AppendOutput($result, o);
      free(*$1);
    }
}


/* TYPEMAPS FOR MEMORY MANAGEMNET OF INT ARRAYS */
%typemap(in, numinputs=0)int** int_out (long* temp), int* int_dim (int temp){
   $1 = &temp;
}
%typemap(argout) (int** int_out, int* int_dim) {
    if (*$1) {
      PyObject *o = PyList_New(*$2);
      int i;
      long* temp = (long*)*$1;
      for(i=0; i<*$2; i++) {
        PyList_SetItem(o, i, PyInt_FromLong(temp[i]));
      }
      $result = SWIG_Python_AppendOutput($result, o);
      free(*$1);
    }
}


/* TYPEMAP FOR ENUMERATED TYPES */
%typemap(in) EnumeratedType (int val, int ecode = 0) {
    if (PyObject_HasAttrString($input,"value")) {
        PyObject* o;
        o = PyObject_GetAttrString($input, "value");
        ecode = SWIG_AsVal_int(o, &val); 
    }   
    else {
        SWIG_exception_fail(SWIG_ArgError(ecode), "in method '" "$symname" "', argument " "$argnum"" of type '" "$ltype""'"); 
    }   
    
    $1 = ($1_type)(val);
}
%apply EnumeratedType {SMO_unit, SMO_elementType, SMO_time, SMO_subcatchAttribute, 
SMO_nodeAttribute, SMO_linkAttribute, SMO_systemAttribute};


/* RENAME FUNCTIONS PYTHON STYLE */
%rename("%(undercase)s") "";


/* INSERTS CUSTOM EXCEPTION HANDLING IN WRAPPER */
%exception
{
    char* err_msg;
    SMO_clearError(arg1);
    $function
    if (SMO_checkError(arg1, &err_msg))
    {
        PyErr_SetString(PyExc_Exception, err_msg);
        SWIG_fail;
    }
}

/* INSERT EXCEPTION HANDLING FOR THESE FUNCTIONS */  

int DLLEXPORT SMO_open(SMO_Handle p_handle, const char* path);

int DLLEXPORT SMO_getVersion(SMO_Handle p_handle, int* int_out);
int DLLEXPORT SMO_getProjectSize(SMO_Handle p_handle, int** int_out, int* int_dim);
int DLLEXPORT SMO_getFlowUnits(SMO_Handle p_handle, int* int_out);
int DLLEXPORT SMO_getPollutantUnits(SMO_Handle p_handle, int** int_out, int* int_dim);
int DLLEXPORT SMO_getStartDate(SMO_Handle p_handle, double* double_out);
int DLLEXPORT SMO_getTimes(SMO_Handle p_handle, SMO_time code, int* int_out);
int DLLEXPORT SMO_getElementName(SMO_Handle p_handle, SMO_elementType type, 
    int elementIndex, char** string_out, int* slen);

int DLLEXPORT SMO_getSubcatchSeries(SMO_Handle p_handle, int subcatchIndex,
    SMO_subcatchAttribute attr, int startPeriod, int endPeriod, float** float_out, int* int_dim);
int DLLEXPORT SMO_getNodeSeries(SMO_Handle p_handle, int nodeIndex, SMO_nodeAttribute attr,
    int startPeriod, int endPeriod, float** float_out, int* int_dim);
int DLLEXPORT SMO_getLinkSeries(SMO_Handle p_handle, int linkIndex, SMO_linkAttribute attr,
    int startPeriod, int endPeriod, float** float_out, int* int_dim);
int DLLEXPORT SMO_getSystemSeries(SMO_Handle p_handle, SMO_systemAttribute attr,
    int startPeriod, int endPeriod, float** float_out, int* int_dim);

int DLLEXPORT SMO_getSubcatchAttribute(SMO_Handle p_handle, int timeIndex,
    SMO_subcatchAttribute attr, float** float_out, int* int_dim);
int DLLEXPORT SMO_getNodeAttribute(SMO_Handle p_handle, int timeIndex,
    SMO_nodeAttribute attr, float** float_out, int* int_dim);
int DLLEXPORT SMO_getLinkAttribute(SMO_Handle p_handle, int timeIndex,
    SMO_linkAttribute attr, float** float_out, int* int_dim);
int DLLEXPORT SMO_getSystemAttribute(SMO_Handle p_handle, int timeIndex,
    SMO_systemAttribute attr, float** float_out, int* int_dim);

int DLLEXPORT SMO_getSubcatchResult(SMO_Handle p_handle, int timeIndex,
    int subcatchIndex, float** float_out, int* int_dim);
int DLLEXPORT SMO_getNodeResult(SMO_Handle p_handle, int timeIndex,
    int nodeIndex, float** float_out, int* int_dim);
int DLLEXPORT SMO_getLinkResult(SMO_Handle p_handle, int timeIndex,
    int linkIndex, float** float_out, int* int_dim);
int DLLEXPORT SMO_getSystemResult(SMO_Handle p_handle, int timeIndex,
    int dummyIndex, float** float_out, int* int_dim);

%exception;        

/* NO EXCEPTION HANDLING FOR THESE FUNCTIONS */        
int DLLEXPORT SMO_init(SMO_Handle* p_handle_out);
int DLLEXPORT SMO_close(SMO_Handle* p_handle_inout);
void DLLEXPORT SMO_free(void** array);

void DLLEXPORT SMO_clearError(SMO_Handle p_handle);
int DLLEXPORT SMO_checkError(SMO_Handle p_handle, char** msg_buffer);


/* CODE ADDED DIRECTLY TO SWIGGED INTERFACE MODULE */
%pythoncode%{
import enum

class Unit(enum.Enum):
    FLOW_UNIT = SMO_flow_rate
    CONC_UNIT = SMO_concentration

class ElementType(enum.Enum):
    SUBCATCH = SMO_subcatch
    NODE = SMO_node
    LINK = SMO_link
    SYSTEM = SMO_sys

class Time(enum.Enum):
    REPORT_STEP = SMO_reportStep
    NUM_PERIODS = SMO_numPeriods

class SubcatchAttribute(enum.Enum):
    RAINFALL = SMO_rainfall_subcatch
    SNOW_DEPTH = SMO_snow_depth_subcatch
    EVAP_LOSS = SMO_evap_loss
    INFIL_LOSS = SMO_infil_loss
    RUNOFF_RATE = SMO_runoff_rate
    GWOUTFLOW_RATE = SMO_gwoutflow_rate
    GETABLE_ELEV = SMO_gwtable_elev
    SOIL_MOISTURE = SMO_soil_moisture
    POLLUT_CONC = SMO_pollutant_conc_subcatch

class NodeAttribute(enum.Enum):
    INVERT_DEPTH = SMO_invert_depth
    HYDRAULIC_HEAD = SMO_hydraulic_head
    PONDED_VOLUME = SMO_stored_ponded_volume
    LATERAL_INFLOW = SMO_lateral_inflow
    TOTAL_INFLOW = SMO_total_inflow
    FLOODING_LOSSES = SMO_flooding_losses
    POLLUT_CONC = SMO_pollutant_conc_node
    
class LinkAttribute(enum.Enum):
    FLOW_RATE = SMO_flow_rate_link
    FLOW_DEPTH = SMO_flow_depth
    FLOW_VELOCITY = SMO_flow_velocity
    FLOW_VOLUME = SMO_flow_volume
    CAPACITY = SMO_capacity
    POLLUT_CONC = SMO_pollutant_conc_link

class SystemAttribute(enum.Enum):
    AIR_TEMP = SMO_air_temp
    RAINFALL = SMO_rainfall_system
    SNOW_DEPTH = SMO_snow_depth_system
    EVAP_INFIL_LOSS = SMO_evap_infil_loss
    RUNOFF_FLOW = SMO_runoff_flow
    DRY_WEATHER_INFLOW = SMO_dry_weather_inflow
    GW_INFLOW = SMO_groundwater_inflow
    RDII_INFLOW = SMO_RDII_inflow
    DIRECT_INFLOW = SMO_direct_inflow
    TOTAL_LATERAL_INFLOW = SMO_total_lateral_inflow
    FLOOD_LOSSES = SMO_flood_losses
    OUTFALL_FLOWS = SMO_outfall_flows
    VOLUME_STORED = SMO_volume_stored
    EVAP_RATE = SMO_evap_rate

%}
