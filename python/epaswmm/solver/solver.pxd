# Description: Cython module for epaswmm solver
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# cython: language_level=3

# cython imports
from cpython.datetime cimport datetime as cython_datetime

# third-party imports

# project imports

# Define the number of days since 01/01/00
cpdef double encode_swmm_datetime(cython_datetime pdatetime)

# Define the number of days since 01/01/00
cpdef cython_datetime decode_swmm_datetime(double swmm_datetime)

cdef extern from "Python.h":
    object PyObject_CallObject(object, object)

cdef extern from "time.h":
    ctypedef long clock_t
    clock_t clock()

cdef extern from "swmm5.h":
    # SWMM object type enumeration
    ctypedef enum swmm_Object:
        swmm_GAGE      # Rain gage
        swmm_SUBCATCH         # Subcatchment
        swmm_NODE             # Junction
        swmm_LINK             # Conduit
        swmm_AQUIFER         # Aquifers
        swmm_SNOWPACK         # Snowpack
        swmm_UNIT_HYDROGRAPH # Unit hydrographs
        swmm_LID              # Low impact development
        swmm_STREET          # Streets
        swmm_INLET           # Inlets
        swmm_TRANSECT        # Transects
        smmm_XSECTION_SHAPE   # Cross-section shape
        swmm_CONTROL_RULE    # Control rules
        swmm_POLLUTANT       # Pollutants
        swmm_LANDUSE         # Land uses
        swmm_CURVE            # Curve
        swmm_TIMESERIES       # Time series
        swmm_TIME_PATTERN     # Time pattern
        swmm_SYSTEM           # General

    # SWMM node type enumeration
    ctypedef enum swmm_NodeType:
        swmm_JUNCTION  # Junction node
        swmm_OUTFALL   # Outfall node
        swmm_STORAGE   # Storage node
        swmm_DIVIDER   # Divider node

    # SWMM link type enumeration
    ctypedef enum swmm_LinkType:
        swmm_CONDUIT   # Conduit link
        swmm_PUMP      # Pump link
        swmm_ORIFICE   # Orifice link
        swmm_WEIR      # Weir link
        swmm_OUTLET    # Outlet link

    # SWMM Rain Gage properties
    ctypedef enum swmm_GageProperty:
        swmm_GAGE_TOTAL_PRECIPITATION # Total precipitation
        swmm_GAGE_RAINFALL          # Snow depth
        swmm_GAGE_SNOWFALL            # Snowfall
    
    # SWMM Subcatchment properties
    ctypedef enum swmm_SubcatchProperty:
        swmm_SUBCATCH_AREA      # Area
        swmm_SUBCATCH_RAINGAGE  # Rain gage
        swmm_SUBCATCH_RAINFALL  # Rainfall
        swmm_SUBCATCH_EVAP      # Evaporation
        swmm_SUBCATCH_INFIL     # Infiltration
        swmm_SUBCATCH_RUNOFF    # Runoff
        swmm_SUBCATCH_RPTFLAG   # Reporting flag
        swmm_SUBCATCH_WIDTH     # Width 
        swmm_SUBCATCH_SLOPE     # Slope
        swmm_SUBCATCH_CURB_LENGTH # Curb length
        swmm_SUBCATCH_API_RAINFALL # API rainfall
        swmm_SUBCATCH_API_SNOWFALL # API snowfall
        swmm_SUBCATCH_POLLUTANT_BUILDUP # Pollutant buildup
        swmm_SUBCATCH_EXTERNAL_POLLUTANT_BUILDUP # External pollutant buildup
        swmm_SUBCATCH_POLLUTANT_RUNOFF_CONCENTRATION # Pollutant ponded concentration
        swmm_SUBCATCH_POLLUTANT_PONDED_CONCENTRATION # Pollutant runoff concentration
        swmm_SUBCATCH_POLLUTANT_TOTAL_LOAD # Pollutant total load

    # SWMM Node properties
    ctypedef enum swmm_NodeProperty:
        swmm_NODE_TYPE     # Node type
        swmm_NODE_ELEV     # Elevation
        swmm_NODE_MAXDEPTH # Max. depth
        swmm_NODE_DEPTH    # Depth
        swmm_NODE_HEAD     # Hydraulic head
        swmm_NODE_VOLUME   # Volume
        swmm_NODE_LATFLOW  # Lateral inflow
        swmm_NODE_INFLOW   # Total inflow
        swmm_NODE_OVERFLOW # Flooding
        swmm_NODE_RPTFLAG  # Reporting flag
        swmm_NODE_SURCHARGE_DEPTH # Surcharge depth
        swmm_NODE_PONDED_AREA   # Ponded area
        swmm_NODE_INITIAL_DEPTH # Initial depth
        swmm_NODE_POLLUTANT_CONCENTRATION # Pollutant concentration
        swmm_NODE_POLLUTANT_LATMASS_FLUX  # Pollutant lateral mass flux

    # SWMM Link properties
    ctypedef enum swmm_LinkProperty:
        swmm_LINK_TYPE       # Link type
        swmm_LINK_NODE1      # Start node
        swmm_LINK_NODE2      # End node
        swmm_LINK_LENGTH     # Length
        swmm_LINK_SLOPE      # Slope 
        swmm_LINK_FULLDEPTH  # Full depth
        swmm_LINK_FULLFLOW   # Full flow
        swmm_LINK_SETTING    # Setting
        swmm_LINK_TIMEOPEN   # Time open
        swmm_LINK_TIMECLOSED # Time closed
        swmm_LINK_FLOW       # Flow
        swmm_LINK_DEPTH      # Depth
        swmm_LINK_VELOCITY   # Velocity
        swmm_LINK_TOPWIDTH   # Top width
        swmm_LINK_VOLUME     # Volume
        swmm_LINK_CAPACITY   # Capacity
        swmm_LINK_RPTFLAG    # Reporting flag
        swmm_LINK_OFFSET1    # Inlet offset
        swmm_LINK_OFFSET2    # Outlet offset
        swmm_LINK_INITIAL_FLOW # Initial flow
        swmm_LINK_FLOW_LIMIT # Flow limit
        swmm_LINK_INLET_LOSS # Inlet loss
        swmm_LINK_OUTLET_LOSS # Outlet loss
        swmm_LINK_AVERAGE_LOSS # Average depth
        swmm_LINK_SEEPAGE_RATE # Seepage rate
        swmm_LINK_HAS_FLAPGATE # Flap gate
        swmm_LINK_POLLUTANT_CONCENTRATION  # Pollutant concentration
        swmm_LINK_POLLUTANT_LOAD # Pollutant load
        swmm_LINK_POLLUTANT_LATMASS_FLUX # Pollutant lateral mass flux

    # SWMM System properties
    ctypedef enum swmm_SystemProperty:
        swmm_STARTDATE           # The start date of the simulation.
        swmm_CURRENTDATE         # The current date in the simulation.
        swmm_ELAPSEDTIME         # The elapsed time since the start of the simulation.
        swmm_ROUTESTEP           # The routing step size.
        swmm_MAXROUTESTEP        # The maximum routing step size.
        swmm_REPORTSTEP          # The reporting step size.
        swmm_TOTALSTEPS          # The total number of steps in the simulation.
        swmm_NOREPORT            # Flag indicating whether reporting is disabled.
        swmm_FLOWUNITS           # The flow units used in the simulation.
        swmm_ENDDATE             # The end date of the simulation.
        swmm_REPORTSTART         # The start date of the reporting period.
        swmm_UNITSYSTEM          # The unit system used in the simulation.
        swmm_SURCHARGEMETHOD     # The surcharge method used in the simulation.
        swmm_ALLOWPONDING        # Flag indicating whether ponding is allowed.
        swmm_INERTIADAMPING      # The inertia damping factor used in the simulation.
        swmm_NORMALFLOWLTD       # The normal flow limited flag.
        swmm_SKIPSTEADYSTATE     # Flag indicating whether steady state periods are skipped.
        swmm_IGNORERAINFALL      # Flag indicating whether rainfall is ignored.
        swmm_IGNORERDII          # Flag indicating whether RDII is ignored.
        swmm_IGNORESNOWMELT      # Flag indicating whether snowmelt is ignored.
        swmm_IGNOREGROUNDWATER   # Flag indicating whether groundwater is ignored.
        swmm_IGNOREROUTING       # Flag indicating whether routing is ignored.
        swmm_IGNOREQUALITY       # Flag indicating whether water quality is ignored.
        swmm_ERROR_CODE          # The error code.
        swmm_RULESTEP            # The rule step size.
        swmm_SWEEPSTART          # The start date of the sweep start.
        swmm_SWEEPEND            # The end date of the sweep end.
        swmm_MAXTRIALS           # The maximum number of trials.
        swmm_NUMTHREADS          # The number of threads used in the simulation.
        swmm_MINROUTESTEP        # The minimum routing step size.
        swmm_LENGTHENINGSTEP     # The lengthening step size.
        swmm_STARTDRYDAYS        # The number of start dry days.
        swmm_COURANTFACTOR       # The Courant factor.
        swmm_MINSURFAREA         # The minimum surface area.
        swmm_MINSLOPE            # The minimum slope.
        swmm_RUNOFFERROR         # The runoff error.
        swmm_FLOWERROR           # The flow error.
        swmm_QUALERROR           # The quality error.
        swmm_HEADTOL             # The head tolerance.
        swmm_SYSFLOWTOL          # The system flow tolerance.
        swmm_LATFLOWTOL          # The lateral flow tolerance.

    # SWMM flow units enumeration
    ctypedef enum swmm_FlowUnitsProperty:
        swmm_CFS  # Cubic feet per second
        swmm_GPM  # Gallons per minute
        swmm_MGD  # Million gallons per day
        swmm_CMS  # Cubic meters per second
        swmm_LPS  # Liters per second
        swmm_MLD  # Million liters per day

    # SWMM API function return error codes
    ctypedef enum swmm_API_Errors:
        ERR_API_NOT_OPEN          # API not open
        ERR_API_NOT_STARTED       # API not started
        ERR_API_NOT_ENDED         # API not ended
        ERR_API_OBJECT_TYPE       # Invalid object type
        ERR_API_OBJECT_INDEX      # Invalid object index
        ERR_API_OBJECT_NAME       # Invalid object name
        ERR_API_PROPERTY_TYPE     # Invalid property type
        ERR_API_PROPERTY_VALUE    # Invalid property value
        ERR_API_TIME_PERIOD       # Invalid time period
        ERR_API_HOTSTART_FILE_OPEN # Error opening hotstart file
        ERR_API_HOTSTART_FILE_FORMAT # Invalid hotstart file format
        ERR_API_IS_RUNNING        # Simulation is already running

    # SWMM API function return simulation progress
    ctypedef void (*progress_callback)(double progress); 

    # SWMM API function prototypes
    # param: inp_file: input file name
    # param: rpt_file: report file name
    # param: out_file: output file name
    # Returns: error code (0 if successful)
    cdef int swmm_run(char* inp_file, char* rpt_file, char* out_file)


    # SWMM API function prototypes
    # param: inp_file: input file name
    # param: rpt_file: report file name
    # param: out_file: output file name
    # param: progress: progress callback
    # Returns: error code (0 if successful)
    cdef int swmm_run_with_callback(char* inp_file, char* rpt_file, char* out_file, progress_callback progress)

    # Open a SWMM input file
    # param: inp_file: input file name
    # param: rpt_file: report file name
    # parm: out_file: output file name
    # Returns: error code (0 if successful)
    cdef int swmm_open(char* inp_file, char* rpt_file, char* out_file)

    #  Starts a SWMM simulation
    #  param: save_flag = TRUE if simulation results saved to binary file 
    #  Returns: error code
    cdef int swmm_start(int save_flag)

    # Performs a single time step of a SWMM simulation
    # param: elapsed_time: elapsed time in seconds
    # Returns: error code
    cdef int swmm_step(double* elapsed_time)

    # Performs a single stride of a SWMM simulation
    # param: strideStep: number of steps to perform
    # param: elapsedTime: elapsed time in seconds
    # Returns: error code
    cdef int swmm_stride(int strideStep, double *elapsedTime)

    # Uses provide hot start file to initialize the simulation
    # param: hotStartFile: hot start file name
    # Returns: error code
    cdef int swmm_useHotStart(const char* hotStartFile)

    # Saves the current simulation state to a hot start file
    # param: hotStartFile: hot start file name
    # Returns: error code
    cdef int swmm_saveHotStart(const char* hotStartFile)

    # Ends a SWMM simulation
    # Returns: error code
    cdef int swmm_end()

    # Writes a report to the report file
    # Returns: error code
    cdef int swmm_report()

    # Closes a SWMM simulation
    # Returns: error code
    cdef int swmm_close()

    # Retrieves the mass balance errors
    # param: runoffErr: runoff error
    # param: flowErr: flow error
    # param: qualErr: quality error
    cdef int swmm_getMassBalErr(float *runoffErr, float *flowErr, float *qualErr)

    # Gets the version of the SWMM engine
    # Returns: version number
    cdef int swmm_getVersion()

    # Retrieves the error message from the SWMM engine
    # param: errMsg: error message
    # param: msgLen: length of the error message
    # Returns: error code
    cdef int swmm_getError(char *errMsg, int msgLen)

    # Retrieves the error message from the SWMM engine
    # param: error_code: error code
    # param: outErrMsg: error message
    # Returns: error code
    cdef int swmm_getErrorFromCode(int error_code, char *outErrMsg[1024])

    # Retrieves the number of warnings from the SWMM engine
    cdef int swmm_getWarnings()
    
    # Retrieves the number of objects of a given type
    # param: objType: object type
    cdef int swmm_getCount(int objType)

    # Retrieves the name of an object of a given type and index
    # param: objType: object type
    # param: index: object index
    # param: name: object name
    # param: size: size of the object name
    cdef int swmm_getName(int objType, int index, char *name, int size)

    # Retrieves the index of an object of a given type and name
    # param: objType: object type
    # param: name: object name
    cdef int swmm_getIndex(int objType, const char *name)

    # Retrieves the value of a property for an object of a given type and index
    # param: property: property type
    # param: index: object index
    cdef double swmm_getValue(int property, int index)


    # Retrieves the value of a property for an object of a given type and index
    # param: objType: object type
    # param: property: property type
    # param: index: object index
    # param: subIndex: sub-index
    cdef double swmm_getValueExpanded(int objType, int property, int index, int subIndex)

    # Sets the value of a property for an object of a given type and index
    # param: property: property type
    # param: index: object index
    # param: value: property value
    cdef int swmm_setValue(int property, int index,  double value)
    
    # Sets the value of a property for an object of a given type and index
    # param: objType: object type
    # param: property: property type
    # param: index: object index
    # param: value: property value
    cdef int swmm_setValueExpanded(int objType, int property, int index, int subindex, double value)

    # Retrieves the value of a property for an object of a given type and index
    # param: property: property type
    # param: index: object index
    # param: period: time period
    cdef double swmm_getSavedValue(int property, int index, int period)
    
    # Writes a line to the SWMM report file
    # param: line: line to write
    cdef void swmm_writeLine(const char *line)

    # Decodes a SWMM datetime into a datetime object    
    cdef void swmm_decodeDate(double date, int *year, int *month, int *day, int *hour, int *minute, int *second, int *dayOfWeek)

    # Encodes a datetime object into a SWMM datetime
    cdef double swmm_encodeDate(int year, int month, int day, int hour, int minute, int second)