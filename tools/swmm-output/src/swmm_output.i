/*
 *  sm_outputapi.i - SWIG interface description file for SWMM Output API
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
%include "numpy.i"

%init %{
    import_array();
%}

/* DEFINE AND TYPEDEF MUST BE INCLUDED */
typedef void* SMO_Handle;

typedef enum {
    flow_rate,
    concentration
} SMO_unit;

typedef enum {
    subcatch,
    node,
    link,
    sys
} SMO_elementType;

typedef enum {
//  reportStart,
    reportStep,
    numPeriods
} SMO_time;

typedef enum {
    rainfall_subcatch,      // (in/hr or mm/hr),
    snow_depth_subcatch,    // (in or mm),
    evap_loss,              // (in/hr or mm/hr),
    infil_loss,             // (in/hr or mm/hr),
    runoff_rate,            // (flow units),
    gwoutflow_rate,         // (flow units),
    gwtable_elev,           // (ft or m),
    soil_moisture,          // unsaturated zone moisture content (-),
    pollutant_conc_subcatch // first pollutant
} SMO_subcatchAttribute;

typedef enum {
    invert_depth,           // (ft or m),
    hydraulic_head,         // (ft or m),
    stored_ponded_volume,   // (ft3 or m3),
    lateral_inflow,         // (flow units),
    total_inflow,           // lateral + upstream (flow units),
    flooding_losses,        // (flow units),
    pollutant_conc_node     // first pollutant,
} SMO_nodeAttribute;

typedef enum {
    flow_rate_link,         // (flow units),
    flow_depth,             // (ft or m),
    flow_velocity,          // (ft/s or m/s),
    flow_volume,            // (ft3 or m3),
    capacity,               // (fraction of conduit filled),
    pollutant_conc_link     // first pollutant,
} SMO_linkAttribute;

typedef enum {
    air_temp,               // (deg. F or deg. C),
    rainfall_system,        // (in/hr or mm/hr),
    snow_depth_system,      // (in or mm),
    evap_infil_loss,        // (in/hr or mm/hr),
    runoff_flow,            // (flow units),
    dry_weather_inflow,     // (flow units),
    groundwater_inflow,     // (flow units),
    RDII_inflow,            // (flow units),
    direct_inflow,          // user defined (flow units),
    total_lateral_inflow,   // (sum of variables 4 to 8) //(flow units),
    flood_losses,           // (flow units),
    outfall_flows,          // (flow units),
    volume_stored,          // (ft3 or m3),
    evap_rate               // (in/day or mm/day),
    //p_evap_rate           // (in/day or mm/day)
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
/* No need for special IN typemap for opaque pointers, it works anyway */


/* TYPEMAP FOR IGNORING INT ERROR CODE RETURN VALUE */
%typemap(out) int {
    $result = Py_None;
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
    int subcatchIndex, float** ARGOUTVIEWM_ARRAY1, int* DIM1);
int DLLEXPORT SMO_getNodeResult(SMO_Handle p_handle, int timeIndex,
    int nodeIndex, float** ARGOUTVIEWM_ARRAY1, int* DIM1);
int DLLEXPORT SMO_getLinkResult(SMO_Handle p_handle, int timeIndex,
    int linkIndex, float** ARGOUTVIEWM_ARRAY1, int* DIM1);
int DLLEXPORT SMO_getSystemResult(SMO_Handle p_handle, int timeIndex,
    int dummyIndex, float** ARGOUTVIEWM_ARRAY1, int* DIM1);

%exception;        

/* NO EXCEPTION HANDLING FOR THESE FUNCTIONS */        
int DLLEXPORT SMO_init(SMO_Handle* p_handle_out);
int DLLEXPORT SMO_close(SMO_Handle* p_handle_out);
void DLLEXPORT SMO_free(void** array);

void DLLEXPORT SMO_clearError(SMO_Handle p_handle);
int DLLEXPORT SMO_checkError(SMO_Handle p_handle, char** msg_buffer);


/* CODE ADDED DIRECTLY TO SWIGGED INTERFACE MODULE */
%pythoncode%{
import enum

class Unit(enum.Enum):
    FLOW_UNIT = flow_rate
    CONC_UNIT = concentration

class ElementType(enum.Enum):
    SUBCATCH = subcatch
    NODE = node
    LINK = link
    SYSTEM = sys

class Time(enum.Enum):
    REPORT_STEP = reportStep
    NUM_PERIODS = numPeriods

class SubcatchAttribute(enum.Enum):
    RAINFALL = rainfall_subcatch
    SNOW_DEPTH = snow_depth_subcatch
    EVAP_LOSS = evap_loss
    INFIL_LOSS = infil_loss
    RUNOFF_RATE = runoff_rate
    GWOUTFLOW_RATE = gwoutflow_rate
    GETABLE_ELEV = gwtable_elev
    SOIL_MOISTURE = soil_moisture
    POLLUT_CONC = pollutant_conc_subcatch

class NodeAttribute(enum.Enum):
    INVERT_DEPTH = invert_depth
    HYDRAULIC_HEAD = hydraulic_head
    PONDED_VOLUME = stored_ponded_volume
    LATERAL_INFLOW = lateral_inflow
    TOTAL_INFLOW = total_inflow
    FLOODING_LOSSES = flooding_losses
    POLLUT_CONC = pollutant_conc_node
    
class LinkAttribute(enum.Enum):
    FLOW_RATE = flow_rate_link
    FLOW_DEPTH = flow_depth
    FLOW_VELOCITY = flow_velocity
    FLOW_VOLUME = flow_volume
    CAPACITY = capacity
    POLLUT_CONC = pollutant_conc_link

class SystemAttribute(enum.Enum):
    AIR_TEMP = air_temp
    RAINFALL = rainfall_system
    SNOW_DEPTH = snow_depth_system
    EVAP_INFIL_LOSS = evap_infil_loss
    RUNOFF_FLOW = runoff_flow
    DRY_WEATHER_INFLOW = dry_weather_inflow
    GW_INFLOW = groundwater_inflow
    RDII_INFLOW = RDII_inflow
    DIRECT_INFLOW = direct_inflow
    TOTAL_LATERAL_INFLOW = total_lateral_inflow
    FLOOD_LOSSES = flood_losses
    OUTFALL_FLOWS = outfall_flows
    VOLUME_STORED = volume_stored
    EVAP_RATE = evap_rate

%}
