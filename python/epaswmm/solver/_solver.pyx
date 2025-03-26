# cython: language_level=3str
# Description: Cython module for epaswmm solver
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# python and cython imports
from enum import Enum
from warnings import warn
from typing import List, Tuple, Union, Dict, Set, Callable
from cpython.datetime cimport datetime as cython_datetime
from datetime import datetime, timedelta
from libc.stdlib cimport free, malloc
from functools import partialmethod

# external imports
from .solver cimport (
    PyObject_CallObject,
    clock_t,
    clock,
    swmm_Object,
    swmm_NodeType,
    swmm_LinkType,
    swmm_GageProperty,
    swmm_SubcatchProperty,
    swmm_NodeProperty,
    swmm_LinkProperty,
    swmm_SystemProperty,
    swmm_FlowUnitsProperty,
    swmm_API_Errors,
    progress_callback,
    swmm_run,
    swmm_run_with_callback,
    swmm_open,
    swmm_start,
    swmm_step,
    swmm_stride,
    swmm_useHotStart,
    swmm_saveHotStart,
    swmm_end,
    swmm_report,
    swmm_close,
    swmm_getMassBalErr,
    swmm_getVersion,
    swmm_getError,
    swmm_getErrorFromCode,
    swmm_getWarnings,
    swmm_getCount,
    swmm_getName,
    swmm_getIndex,
    swmm_getValue,
    swmm_getValueExpanded,
    swmm_setValue,
    swmm_setValueExpanded,
    swmm_getSavedValue,
    swmm_writeLine,
    swmm_decodeDate,
    swmm_encodeDate
)

class SWMMObjects(Enum):
    """
    Enumeration of SWMM objects.

    :ivar RAIN_GAGE: Raingage object
    :type RAIN_GAGE: int
    :ivar SUBCATCHMENT: Subcatchment object
    :type SUBCATCHMENT: int 
    :ivar NODE: Node object
    :type NODE: int
    :ivar LINK: Link object
    :type LINK: int
    :ivar AQUIFER: Aquifer object
    :type AQUIFER: int
    :ivar SNOWPACK: Snowpack object
    :type SNOWPACK: int
    :ivar UNIT_HYDROGRAPH: Unit hydrograph object
    :type UNIT_HYDROGRAPH: int
    :ivar LID: LID object
    :type LID: int
    :ivar STREET: Street object
    :type STREET: int
    :ivar INLET: Inlet object
    :type INLET: int
    :ivar TRANSECT: Transect object
    :type TRANSECT: int
    :ivar XSECTION_SHAPE: Cross-section shape object
    :type XSECTION_SHAPE: int
    :ivar CONTROL_RULE: Control rule object
    :type CONTROL_RULE: int
    :ivar POLLUTANT: Pollutant object
    :type POLLUTANT: int
    :ivar LANDUSE: Land use object
    :type LANDUSE: int
    :ivar CURVE: Curve object
    :type CURVE: int
    :ivar TIMESERIES: Time series object
    :type TIMESERIES: int
    :ivar TIME_PATTERN: Time pattern object
    :type TIME_PATTERN: int
    :ivar SYSTEM: System object
    :type SYSTEM: int
    """
    RAIN_GAGE = swmm_Object.swmm_GAGE
    SUBCATCHMENT = swmm_Object.swmm_SUBCATCH 
    NODE = swmm_Object.swmm_NODE
    LINK = swmm_Object.swmm_LINK
    AQUIFER = swmm_Object.swmm_AQUIFER
    SNOWPACK = swmm_Object.swmm_SNOWPACK
    UNIT_HYDROGRAPH = swmm_Object.swmm_UNIT_HYDROGRAPH
    LID = swmm_Object.swmm_LID
    STREET = swmm_Object.swmm_STREET
    INLET = swmm_Object.swmm_INLET
    TRANSECT = swmm_Object.swmm_TRANSECT
    XSECTION_SHAPE = swmm_Object.smmm_XSECTION_SHAPE
    CONTROL_RULE = swmm_Object.swmm_CONTROL_RULE
    POLLUTANT = swmm_Object.swmm_POLLUTANT
    LANDUSE = swmm_Object.swmm_LANDUSE
    CURVE = swmm_Object.swmm_CURVE
    TIMESERIES = swmm_Object.swmm_TIMESERIES
    TIME_PATTERN = swmm_Object.swmm_TIME_PATTERN
    SYSTEM = swmm_Object.swmm_SYSTEM

class SWMMNodeTypes(Enum):
    """
    Enumeration of SWMM node types.

    :ivar JUNCTION: Junction node
    :type JUNCTION: int
    :ivar OUTFALL: Outfall node
    :type OUTFALL: int
    :ivar STORAGE: Storage node
    :type STORAGE: int
    :ivar DIVIDER: Divider node
    :type DIVIDER: int
    """
    JUNCTION = swmm_NodeType.swmm_JUNCTION
    OUTFALL = swmm_NodeType.swmm_OUTFALL
    STORAGE = swmm_NodeType.swmm_STORAGE
    DIVIDER = swmm_NodeType.swmm_DIVIDER

class SWMMRainGageProperties(Enum):
    """
    Enumeration of SWMM raingage properties.

    :ivar GAGE_TOTAL_PRECIPITATION: Total precipitation
    :type GAGE_TOTAL_PRECIPITATION: int
    :ivar GAGE_RAINFALL: Rainfall
    :type GAGE_RAINFALL: int
    :ivar GAGE_SNOWFALL: Snowfall
    :type GAGE_SNOWFALL: int
    """
    GAGE_TOTAL_PRECIPITATION = swmm_GageProperty.swmm_GAGE_TOTAL_PRECIPITATION # Total precipitation
    GAGE_RAINFALL = swmm_GageProperty.swmm_GAGE_RAINFALL # Rainfall
    GAGE_SNOWFALL = swmm_GageProperty.swmm_GAGE_SNOWFALL # Snowfall
    
class SWMMSubcatchmentProperties(Enum):
    """
    Enumeration of SWMM subcatchment properties.

    :ivar AREA: Area
    :type AREA: int
    :ivar RAINGAGE: Raingage
    :type RAINGAGE: int
    :ivar RAINFALL: Rainfall
    :type RAINFALL: int
    :ivar EVAPORATION: Evaporation
    :type EVAPORATION: int
    :ivar INFILTRATION: Infiltration
    :type INFILTRATION: int
    :ivar RUNOFF: Runoff
    :type RUNOFF: int
    :ivar REPORT_FLAG: Report flag
    :type REPORT_FLAG: int
    :ivar WIDTH: Width
    :type WIDTH: int
    :ivar SLOPE: Slope
    :type SLOPE: int
    :ivar CURB_LENGTH: Curb length
    :type CURB_LENGTH: int
    :ivar API_RAINFALL: API Rainfall
    :type API_RAINFALL: int
    :ivar API_SNOWFALL: API Snowfall
    :type API_SNOWFALL: int
    :ivar POLLUTANT_BUILDUP: Pollutant buildup
    :type POLLUTANT_BUILDUP: int
    :ivar EXTERNAL_POLLUTANT_BUILDUP: External pollutant buildup
    :type EXTERNAL_POLLUTANT_BUILDUP: int
    :ivar POLLUTANT_RUNOFF_CONCENTRATION: Pollutant runoff concentration
    :type POLLUTANT_RUNOFF_CONCENTRATION: int
    :ivar POLLUTANT_PONDED_CONCENTRATION: Pollutant ponded concentration
    :type POLLUTANT_PONDED_CONCENTRATION: int
    :ivar POLLUTANT_TOTAL_LOAD: Pollutant total load
    :type POLLUTANT_TOTAL_LOAD: int
    """
    AREA = swmm_SubcatchProperty.swmm_SUBCATCH_AREA
    RAINGAGE = swmm_SubcatchProperty.swmm_SUBCATCH_RAINGAGE
    RAINFALL = swmm_SubcatchProperty.swmm_SUBCATCH_RAINFALL
    EVAPORATION = swmm_SubcatchProperty.swmm_SUBCATCH_EVAP
    INFILTRATION = swmm_SubcatchProperty.swmm_SUBCATCH_INFIL
    RUNOFF = swmm_SubcatchProperty.swmm_SUBCATCH_RUNOFF
    REPORT_FLAG = swmm_SubcatchProperty.swmm_SUBCATCH_RPTFLAG
    WIDTH = swmm_SubcatchProperty.swmm_SUBCATCH_WIDTH
    SLOPE = swmm_SubcatchProperty.swmm_SUBCATCH_SLOPE
    CURB_LENGTH = swmm_SubcatchProperty.swmm_SUBCATCH_CURB_LENGTH
    API_RAINFALL = swmm_SubcatchProperty.swmm_SUBCATCH_API_RAINFALL
    API_SNOWFALL = swmm_SubcatchProperty.swmm_SUBCATCH_API_SNOWFALL
    POLLUTANT_BUILDUP = swmm_SubcatchProperty.swmm_SUBCATCH_POLLUTANT_BUILDUP
    EXTERNAL_POLLUTANT_BUILDUP = swmm_SubcatchProperty.swmm_SUBCATCH_EXTERNAL_POLLUTANT_BUILDUP
    POLLUTANT_RUNOFF_CONCENTRATION = swmm_SubcatchProperty.swmm_SUBCATCH_POLLUTANT_RUNOFF_CONCENTRATION 
    POLLUTANT_PONDED_CONCENTRATION = swmm_SubcatchProperty.swmm_SUBCATCH_POLLUTANT_PONDED_CONCENTRATION
    POLLUTANT_TOTAL_LOAD = swmm_SubcatchProperty.swmm_SUBCATCH_POLLUTANT_TOTAL_LOAD
    
class SWMMNodeProperties(Enum):
    """
    Enumeration of SWMM node properties.

    :ivar TYPE: Node type
    :type TYPE: int
    :ivar INVERT_ELEVATION: Invert elevation
    :type INVERT_ELEVATION: int
    :ivar MAX_DEPTH: Maximum depth
    :type MAX_DEPTH: int
    :ivar DEPTH: Depth
    :type DEPTH: int
    :ivar HYDRAULIC_HEAD: Hydraulic head
    :type HYDRAULIC_HEAD: int
    :ivar VOLUME: Volume
    :type VOLUME: int
    :ivar LATERAL_INFLOW: Lateral inflow
    :type LATERAL_INFLOW: int
    :ivar TOTAL_INFLOW: Total inflow
    :type TOTAL_INFLOW: int
    :ivar FLOODING: Flooding
    :type FLOODING: int
    :ivar REPORT_FLAG: Report flag
    :type REPORT_FLAG: int
    :ivar SURCHARGE_DEPTH: Surcharge depth
    :type SURCHARGE_DEPTH: int
    :ivar PONDING_AREA: Ponding area
    :type PONDING_AREA: int
    :ivar INITIAL_DEPTH: Initial depth
    :type INITIAL_DEPTH: int
    :ivar POLLUTANT_CONCENTRATION: Pollutant concentration
    :type POLLUTANT_CONCENTRATION: int
    :ivar POLLUTANT_LATERAL_MASS_FLUX: Pollutant lateral mass flux
    :type POLLUTANT_LATERAL_MASS_FLUX: int
    """
    TYPE = swmm_NodeProperty.swmm_NODE_TYPE
    INVERT_ELEVATION = swmm_NodeProperty.swmm_NODE_ELEV
    MAX_DEPTH = swmm_NodeProperty.swmm_NODE_MAXDEPTH
    DEPTH = swmm_NodeProperty.swmm_NODE_DEPTH
    HYDRAULIC_HEAD = swmm_NodeProperty.swmm_NODE_HEAD
    VOLUME = swmm_NodeProperty.swmm_NODE_VOLUME
    LATERAL_INFLOW = swmm_NodeProperty.swmm_NODE_LATFLOW
    TOTAL_INFLOW = swmm_NodeProperty.swmm_NODE_INFLOW
    FLOODING = swmm_NodeProperty.swmm_NODE_OVERFLOW
    REPORT_FLAG = swmm_NodeProperty.swmm_NODE_RPTFLAG
    SURCHARGE_DEPTH = swmm_NodeProperty.swmm_NODE_SURCHARGE_DEPTH
    PONDING_AREA = swmm_NodeProperty.swmm_NODE_PONDED_AREA
    INITIAL_DEPTH = swmm_NodeProperty.swmm_NODE_INITIAL_DEPTH
    POLLUTANT_CONCENTRATION = swmm_NodeProperty.swmm_NODE_POLLUTANT_CONCENTRATION # Pollutant concentration
    POLLUTANT_LATERAL_MASS_FLUX = swmm_NodeProperty.swmm_NODE_POLLUTANT_LATMASS_FLUX # Pollutant inflow concentration

class SWMMLinkProperties(Enum):
    """
    Enumeration of SWMM link properties.

    :ivar TYPE: Link type
    :type TYPE: int
    :ivar START_NODE: Start node
    :type START_NODE: int
    :ivar END_NODE: End node
    :type END_NODE: int
    :ivar LENGTH: Length
    :type LENGTH: int
    :ivar SLOPE: Slope
    :type SLOPE: int
    :ivar FULL_DEPTH: Full depth
    :type FULL_DEPTH: int
    :ivar FULL_FLOW: Full flow
    :type FULL_FLOW: int
    :ivar SETTING: Setting
    :type SETTING: int
    :ivar TIME_OPEN: Time open
    :type TIME_OPEN: int
    :ivar TIME_CLOSED: Time closed
    :type TIME_CLOSED: int
    :ivar FLOW: Flow
    :type FLOW: int
    :ivar DEPTH: Depth
    :type DEPTH: int
    :ivar VELOCITY: Velocity
    :type VELOCITY: int
    :ivar TOP_WIDTH: Top width
    :type TOP_WIDTH: int
    :ivar REPORT_FLAG: Report flag
    :type REPORT_FLAG: int
    :ivar START_NODE_OFFSET: Start node offset
    :type START_NODE_OFFSET: int
    :ivar END_NODE_OFFSET: End node offset
    :type END_NODE_OFFSET: int
    :ivar INITIAL_FLOW: Initial flow
    :type INITIAL_FLOW: int
    :ivar FLOW_LIMIT: Flow limit
    :type FLOW_LIMIT: int
    :ivar INLET_LOSS: Inlet loss
    :type INLET_LOSS: int
    :ivar OUTLET_LOSS: Outlet loss
    :type OUTLET_LOSS: int
    :ivar AVERAGE_LOSS: Average loss
    :type AVERAGE_LOSS: int
    :ivar SEEPAGE_RATE: Seepage rate
    :type SEEPAGE_RATE: int
    :ivar HAS_FLAPGATE: Has flapgate
    :type HAS_FLAPGATE: int
    :ivar POLLUTANT_CONCENTRATION: Pollutant concentration
    :type POLLUTANT_CONCENTRATION: int
    :ivar POLLUTANT_LOAD: Pollutant load
    :type POLLUTANT_LOAD: int
    :ivar POLLUTANT_LATERAL_MASS_FLUX: Pollutant lateral mass flux
    :type POLLUTANT_LATERAL_MASS_FLUX: int
    """
    TYPE = swmm_LinkProperty.swmm_LINK_TYPE
    START_NODE = swmm_LinkProperty.swmm_LINK_NODE1
    END_NODE = swmm_LinkProperty.swmm_LINK_NODE2
    LENGTH = swmm_LinkProperty.swmm_LINK_LENGTH
    SLOPE = swmm_LinkProperty.swmm_LINK_SLOPE
    FULL_DEPTH = swmm_LinkProperty.swmm_LINK_FULLDEPTH
    FULL_FLOW = swmm_LinkProperty.swmm_LINK_FULLFLOW
    SETTING = swmm_LinkProperty.swmm_LINK_SETTING
    TIME_OPEN = swmm_LinkProperty.swmm_LINK_TIMEOPEN
    TIME_CLOSED = swmm_LinkProperty.swmm_LINK_TIMECLOSED
    FLOW = swmm_LinkProperty.swmm_LINK_FLOW
    DEPTH = swmm_LinkProperty.swmm_LINK_DEPTH
    VELOCITY = swmm_LinkProperty.swmm_LINK_VELOCITY
    TOP_WIDTH = swmm_LinkProperty.swmm_LINK_TOPWIDTH
    VOLUME = swmm_LinkProperty.swmm_LINK_VOLUME
    CAPACITY = swmm_LinkProperty.swmm_LINK_CAPACITY
    REPORT_FLAG = swmm_LinkProperty.swmm_LINK_RPTFLAG
    START_NODE_OFFSET = swmm_LinkProperty.swmm_LINK_OFFSET1
    END_NODE_OFFSET = swmm_LinkProperty.swmm_LINK_OFFSET2
    INITIAL_FLOW = swmm_LinkProperty.swmm_LINK_INITIAL_FLOW
    FLOW_LIMIT = swmm_LinkProperty.swmm_LINK_FLOW_LIMIT
    INLET_LOSS = swmm_LinkProperty.swmm_LINK_INLET_LOSS
    OUTLET_LOSS = swmm_LinkProperty.swmm_LINK_OUTLET_LOSS
    AVERAGE_LOSS = swmm_LinkProperty.swmm_LINK_AVERAGE_LOSS
    SEEPAGE_RATE = swmm_LinkProperty.swmm_LINK_SEEPAGE_RATE
    HAS_FLAPGATE = swmm_LinkProperty.swmm_LINK_HAS_FLAPGATE
    POLLUTANT_CONCENTRATION = swmm_LinkProperty.swmm_LINK_POLLUTANT_CONCENTRATION  # Pollutant concentration
    POLLUTANT_LOAD = swmm_LinkProperty.swmm_LINK_POLLUTANT_LOAD # Pollutant load
    POLLUTANT_LATERAL_MASS_FLUX = swmm_LinkProperty.swmm_LINK_POLLUTANT_LATMASS_FLUX # Pollutant lateral mass flux

class SWMMLinkTypes(Enum):
    """
    Enumeration of SWMM link types.

    :ivar CONDUIT: Conduit link
    :type CONDUIT: int
    :ivar PUMP: Pump link
    :type PUMP: int
    :ivar ORIFICE: Orifice link
    :type ORIFICE: int
    :ivar WEIR: Weir link
    :type WEIR: int
    :ivar OUTLET: Outlet link
    :type OUTLET: int
    """
    CONDUIT = swmm_LinkType.swmm_CONDUIT
    PUMP = swmm_LinkType.swmm_PUMP
    ORIFICE = swmm_LinkType.swmm_ORIFICE
    WEIR = swmm_LinkType.swmm_WEIR
    OUTLET = swmm_LinkType.swmm_OUTLET

class SWMMSystemProperties(Enum):
    """
    Enumeration of SWMM system properties.

    :ivar START_DATE: Start date for the simulation
    :type START_DATE: int
    :ivar CURRENT_DATE: Current date for the simulation
    :type CURRENT_DATE: int
    :ivar ELAPSED_TIME: Elapsed time for the simulation
    :type ELAPSED_TIME: int
    :ivar ROUTING_STEP: Routing time step
    :type ROUTING_STEP: int
    :ivar MAX_ROUTING_STEP: Maximum routing time step
    :type MAX_ROUTING_STEP: int
    :ivar REPORT_STEP: Report time step
    :type REPORT_STEP: int
    :ivar TOTAL_STEPS: Total number of steps
    :type TOTAL_STEPS: int
    :ivar NO_REPORT_FLAG: No report flag
    :type NO_REPORT_FLAG: int
    :ivar FLOW_UNITS: Flow units
    :type FLOW_UNITS: int
    :ivar END_DATE: End date for the simulation
    :type END_DATE: int
    :ivar REPORT_START_DATE: Report start date
    :type REPORT_START_DATE: int
    :ivar UNIT_SYSTEM: Unit system
    :type UNIT_SYSTEM: int
    :ivar SURCHARGE_METHOD: Surcharge method
    :type SURCHARGE_METHOD: int
    :ivar ALLOW_PONDING: Allow ponding
    :type ALLOW_PONDING: int
    :ivar INTERTIAL_DAMPING: Inertial damping
    :type INTERTIAL_DAMPING: int
    :ivar NORMAL_FLOW_LIMITED: Normal flow limited
    :type NORMAL_FLOW_LIMITED: int
    :ivar SKIP_STEADY_STATE: Skip steady state
    :type SKIP_STEADY_STATE: int
    :ivar IGNORE_RAINFALL: Ignore rainfall
    :type IGNORE_RAINFALL: int
    :ivar IGNORE_RDII: Ignore RDII
    :type IGNORE_RDII: int
    :ivar IGNORE_SNOWMELT: Ignore snowmelt
    :type IGNORE_SNOWMELT: int
    :ivar IGNORE_GROUNDWATER: Ignore groundwater
    :type IGNORE_GROUNDWATER: int
    :ivar IGNORE_ROUTING: Ignore routing
    :type IGNORE_ROUTING: int
    :ivar IGNORE_QUALITY: Ignore quality
    :type IGNORE_QUALITY: int
    :ivar RULE_STEP: Rule step
    :type RULE_STEP: int
    :ivar SWEEP_START: Sweep start
    :type SWEEP_START: int
    :ivar SWEEP_END: Sweep end
    :type SWEEP_END: int
    :ivar MAX_TRIALS: Maximum trials
    :type MAX_TRIALS: int
    :ivar NUM_THREADS: Number of threads
    :type NUM_THREADS: int
    :ivar MIN_ROUTE_STEP: Minimum routing step
    :type MIN_ROUTE_STEP: int
    :ivar LENGTHENING_STEP: Lengthening step
    :type LENGTHENING_STEP: int
    :ivar START_DRY_DAYS: Start dry days
    :type START_DRY_DAYS: int
    :ivar COURANT_FACTOR: Courant factor
    :type COURANT_FACTOR: int
    :ivar MIN_SURF_AREA: Minimum surface area
    :type MIN_SURF_AREA: int
    :ivar MIN_SLOPE: Minimum slope
    :type MIN_SLOPE: int
    :ivar RUNOFF_ERROR: Runoff error
    :type RUNOFF_ERROR: int
    :ivar FLOW_ERROR: Flow error
    :type FLOW_ERROR: int
    :ivar QUAL_ERROR: Quality error
    :type QUAL_ERROR: int
    :ivar HEAD_TOL: Head tolerance
    :type HEAD_TOL: int
    :ivar SYS_FLOW_TOL: System flow tolerance
    :type SYS_FLOW_TOL: int
    :ivar LAT_FLOW_TOL: Lateral flow tolerance
    :type LAT_FLOW_TOL: int

    """
    START_DATE = swmm_SystemProperty.swmm_STARTDATE
    CURRENT_DATE = swmm_SystemProperty.swmm_CURRENTDATE
    ELAPSED_TIME = swmm_SystemProperty.swmm_ELAPSEDTIME
    ROUTING_STEP = swmm_SystemProperty.swmm_ROUTESTEP
    MAX_ROUTING_STEP = swmm_SystemProperty.swmm_MAXROUTESTEP
    REPORT_STEP = swmm_SystemProperty.swmm_REPORTSTEP
    TOTAL_STEPS = swmm_SystemProperty.swmm_TOTALSTEPS
    NO_REPORT_FLAG = swmm_SystemProperty.swmm_NOREPORT
    FLOW_UNITS = swmm_SystemProperty.swmm_FLOWUNITS
    END_DATE = swmm_SystemProperty.swmm_ENDDATE
    REPORT_START_DATE = swmm_SystemProperty.swmm_REPORTSTART
    UNIT_SYSTEM = swmm_SystemProperty.swmm_UNITSYSTEM
    SURCHARGE_METHOD = swmm_SystemProperty.swmm_SURCHARGEMETHOD
    ALLOW_PONDING = swmm_SystemProperty.swmm_ALLOWPONDING
    INTERTIAL_DAMPING = swmm_SystemProperty.swmm_INERTIADAMPING
    NORMAL_FLOW_LIMITED = swmm_SystemProperty.swmm_NORMALFLOWLTD
    SKIP_STEADY_STATE = swmm_SystemProperty.swmm_SKIPSTEADYSTATE
    IGNORE_RAINFALL = swmm_SystemProperty.swmm_IGNORERAINFALL
    IGNORE_RDII = swmm_SystemProperty.swmm_IGNORERDII
    IGNORE_SNOWMELT = swmm_SystemProperty.swmm_IGNORESNOWMELT
    IGNORE_GROUNDWATER = swmm_SystemProperty.swmm_IGNOREGROUNDWATER
    IGNORE_ROUTING = swmm_SystemProperty.swmm_IGNOREROUTING
    IGNORE_QUALITY = swmm_SystemProperty.swmm_IGNOREQUALITY
    ERROR_CODE = swmm_SystemProperty.swmm_ERROR_CODE
    RULE_STEP = swmm_SystemProperty.swmm_RULESTEP
    SWEEP_START = swmm_SystemProperty.swmm_SWEEPSTART
    SWEEP_END = swmm_SystemProperty.swmm_SWEEPEND
    MAX_TRIALS = swmm_SystemProperty.swmm_MAXTRIALS
    NUM_THREADS = swmm_SystemProperty.swmm_NUMTHREADS
    MIN_ROUTE_STEP = swmm_SystemProperty.swmm_MINROUTESTEP
    LENGTHENING_STEP = swmm_SystemProperty.swmm_LENGTHENINGSTEP
    START_DRY_DAYS = swmm_SystemProperty.swmm_STARTDRYDAYS
    COURANT_FACTOR = swmm_SystemProperty.swmm_COURANTFACTOR
    MIN_SURF_AREA = swmm_SystemProperty.swmm_MINSURFAREA
    MIN_SLOPE = swmm_SystemProperty.swmm_MINSLOPE
    RUNOFF_ERROR = swmm_SystemProperty.swmm_RUNOFFERROR
    FLOW_ERROR = swmm_SystemProperty.swmm_FLOWERROR
    QUAL_ERROR = swmm_SystemProperty.swmm_QUALERROR
    HEAD_TOL = swmm_SystemProperty.swmm_HEADTOL
    SYS_FLOW_TOL = swmm_SystemProperty.swmm_SYSFLOWTOL
    LAT_FLOW_TOL = swmm_SystemProperty.swmm_LATFLOWTOL

class SWMMFlowUnits(Enum):
    """
    Enumeration of SWMM flow units.

    :ivar CFS: Cubic feet per second
    :type CFS: int
    :ivar GPM: Gallons per minute
    :type GPM: int
    :ivar MGD: Million gallons per day
    :type MGD: int
    :ivar CMS: Cubic meters per second
    :type CMS: int
    :ivar LPS: Liters per second
    :type LPS: int
    :ivar MLD: Million liters per day
    :type MLD: int
    """
    CFS = swmm_FlowUnitsProperty.swmm_CFS
    GPM = swmm_FlowUnitsProperty.swmm_GPM
    MGD = swmm_FlowUnitsProperty.swmm_MGD
    CMS = swmm_FlowUnitsProperty.swmm_CMS
    LPS = swmm_FlowUnitsProperty.swmm_LPS
    MLD = swmm_FlowUnitsProperty.swmm_MLD

class SWMMAPIErrors(Enum):
    """
    Enumeration of SWMM API errors.

    :ivar PROJECT_NOT_OPENED: Project not opened
    :type PROJECT_NOT_OPENED: int
    :ivar SIMULATION_NOT_STARTED: Simulation not started
    :type SIMULATION_NOT_STARTED: int
    :ivar SIMULATION_NOT_ENDED: Simulation not ended
    :type SIMULATION_NOT_ENDED: int    
    """
    PROJECT_NOT_OPENED = swmm_API_Errors.ERR_API_NOT_OPEN          # API not open
    SIMULATION_NOT_STARTED = swmm_API_Errors.ERR_API_NOT_STARTED       # API not started
    SIMULATION_NOT_ENDED = swmm_API_Errors.ERR_API_NOT_ENDED         # API not ended
    OBJECT_TYPE = swmm_API_Errors.ERR_API_OBJECT_TYPE       # Invalid object type
    OBJECT_INDEX = swmm_API_Errors.ERR_API_OBJECT_INDEX      # Invalid object index
    OBJECT_NAME = swmm_API_Errors.ERR_API_OBJECT_NAME       # Invalid object name
    PROPERTY_TYPE = swmm_API_Errors.ERR_API_PROPERTY_TYPE     # Invalid property type
    PROPERTY_VALUE = swmm_API_Errors.ERR_API_PROPERTY_VALUE    # Invalid property value
    TIME_PERIOD = swmm_API_Errors.ERR_API_TIME_PERIOD       # Invalid time period
    HOTSTART_FILE_OPEN = swmm_API_Errors.ERR_API_HOTSTART_FILE_OPEN # Error opening hotstart file
    HOTSTART_FILE_FORMAT = swmm_API_Errors.ERR_API_HOTSTART_FILE_FORMAT # Invalid hotstart file format
    SIMULATION_IS_RUNNING = swmm_API_Errors.ERR_API_IS_RUNNING # Simulation is running

cdef void c_wrapper_function(double x):
    """
    Wrapper function to call a Python function.

    :param x: Input value
    :type x: double
    """
    global py_progress_callback
    cdef tuple args = (x,)
    PyObject_CallObject(py_progress_callback, args)

cdef progress_callback wrap_python_function_as_callback(object py_func):
    """
    Wrap a Python function as a callback.

    :param py_func: Python function
    :type py_func: callable
    :return: Callback function
    :rtype: progress_callback
    """
    global py_progress_callback
    py_progress_callback = py_func
    return <progress_callback>c_wrapper_function

cdef object global_solver = None

cdef void progress_callback_wrapper(double progress):
    """
    Wrapper function to call the instance method.
    
    :param progress: Progress percentage
    :type progress: double

    """
    global solver_instance

    if solver_instance is not None:
        solver_instance.__progress_callback(progress)

def run_solver(
    inp_file: str, 
    rpt_file: str, 
    out_file: str, 
    swmm_progress_callback: Callable[[float], None] = None
    ) -> int:
    """
    Run a SWMM simulation with a progress callback.

    :param inp_file: Input file name
    :rtype inp_file: str
    :param rpt_file: Report file name
    :rtype rpt_file: str
    :param out_file: Output file name
    :rtype out_file: str
    :param swmm_progress_callback: Progress callback function
    :type swmm_progress_callback: callable
    :return: Error code (0 if successful)
    """
    cdef int error_code = 0
    cdef bytes c_inp_file_bytes = inp_file.encode('utf-8')
    cdef progress_callback c_swm_progress_callback

    if rpt_file is not None:
       rpt_file = inp_file.replace('.inp', '.rpt')

    if out_file is not None:
         out_file = inp_file.replace('.inp', '.out')

    cdef bytes c_rpt_file_bytes = rpt_file.encode('utf-8')
    cdef bytes c_out_file_bytes = out_file.encode('utf-8')

    cdef const char* c_inp_file = c_inp_file_bytes
    cdef const char* c_rpt_file = c_rpt_file_bytes
    cdef const char* c_out_file = c_out_file_bytes

    if swmm_progress_callback is not None:
        c_swm_progress_callback = <progress_callback>wrap_python_function_as_callback(swmm_progress_callback)
        error_code = swmm_run_with_callback(c_inp_file, c_rpt_file, c_out_file, c_swm_progress_callback)
    else:
        error_code = swmm_run(c_inp_file, c_rpt_file, c_out_file)

    if error_code != 0:
        raise SWMMSolverException(f'Run failed with message: {get_error_message(error_code)}')
    
    return error_code

cpdef cython_datetime decode_swmm_datetime(double swmm_datetime):
    """
    Decode a SWMM datetime into a datetime object.
    
    :param swmm_datetime: SWMM datetime float value
    :type swmm_datetime: float
    
    :return: datetime object
    :rtype: datetime
    """
    cdef int year, month, day, hour, minute, second, day_of_week
    swmm_decodeDate(swmm_datetime, &year, &month, &day, &hour, &minute, &second, &day_of_week)

    return datetime(year, month, day, hour, minute, second)

cpdef double encode_swmm_datetime(cython_datetime dt):
    """
    Encode a datetime object into a SWMM datetime float value.

    :param dt: datetime object
    :type dt: datetime
    :return: SWMM datetime float value
    :rtype: float
    """
    cdef int year = dt.year
    cdef int month = dt.month
    cdef int day = dt.day
    cdef int hour = dt.hour
    cdef int minute = dt.minute
    cdef int second = dt.second

    return swmm_encodeDate(year, month, day, hour, minute, second)

cpdef int version():
    """
    Get the SWMM version.
    
    :return: SWMM version
    :rtype: str
    """
    cdef int swmm_version = swmm_getVersion()

    return swmm_version

cpdef str get_error_message(int error_code):
    """
    Get the error message for a SWMM error code.
    
    :param error_code: Error code
    :type error_code: int
    :return: Error message
    :rtype: str
    """
    cdef char* c_error_message = <char*>malloc(1024*sizeof(char))
    
    swmm_getErrorFromCode(error_code, &c_error_message)

    error_message = c_error_message.decode('utf-8')

    free(c_error_message)

    return error_message

class SolverState(Enum):
    """
    An enumeration to represent the state of the solver.
    """
    CREATED = 0 
    OPEN = 1
    STARTED = 2
    FINISHED = 3
    ENDED = 4
    REPORTED = 5
    CLOSED = 6

class CallbackType(Enum):
    """
    An enumeration to represent the type of callback.
    """
    BEFORE_INITIALIZE = 0
    BEFORE_OPEN = 1
    AFTER_OPEN = 2
    BEFORE_START = 3
    AFTER_START = 4
    BEFORE_STEP = 5
    AFTER_STEP = 6
    BEFORE_END = 7
    AFTER_END = 8
    BEFORE_REPORT = 9
    AFTER_REPORT = 10
    BEFORE_CLOSE = 11
    AFTER_CLOSE = 12

class SWMMSolverException(Exception):
    """
    Exception class for SWMM output file processing errors.
    """
    def __init__(self, message: str) -> None:
        """
        Constructor to initialize the exception message.

        :param message: Error message.
        :type message: str
        """
        super().__init__(message)

cdef class Solver:
    """
    A class to represent a SWMM solver.
    """
    cdef str _inp_file
    cdef str _rpt_file
    cdef str _out_file
    cdef bint _save_results
    cdef int _stride_step
    cdef dict _callbacks 
    cdef int _progress_callbacks_per_second 
    cdef list _progress_callbacks 
    cdef clock_t _clock 
    cdef double _total_duration
    cdef object _solver_state
    cdef object _partial_step_function 

    def __cinit__(
        self, 
        str inp_file, 
        str rpt_file = None, 
        str out_file = None, 
        int stride_step = 300, 
        bint save_results=True
    ):
        """
        Constructor to create a new SWMM solver.

        :param inp_file: Input file name
        :param rpt_file: Report file name
        :param out_file: Output file name
        """
        global global_solver
        self._save_results = save_results
        self._inp_file = inp_file
        self._progress_callbacks_per_second = 2
        self._stride_step = stride_step
        self._clock = clock()
        global_solver = self

        if rpt_file is not None:
            self._rpt_file = rpt_file
        else:
            self._rpt_file = inp_file.replace('.inp', '.rpt')

        if out_file is not None:
            self._out_file = out_file
        else:
            self._out_file = inp_file.replace('.inp', '.out')
        
        self._callbacks = {
            CallbackType.BEFORE_INITIALIZE: [],
            CallbackType.BEFORE_OPEN: [],
            CallbackType.AFTER_OPEN: [],
            CallbackType.BEFORE_START: [],
            CallbackType.AFTER_START: [],
            CallbackType.BEFORE_STEP: [],
            CallbackType.AFTER_STEP: [],
            CallbackType.BEFORE_END: [],
            CallbackType.AFTER_END: [],
            CallbackType.BEFORE_REPORT: [],
            CallbackType.AFTER_REPORT: [],
            CallbackType.BEFORE_CLOSE: [],
            CallbackType.AFTER_CLOSE: []
        }
        
        self._progress_callbacks = []

        self._solver_state = SolverState.CREATED

    def __enter__(self):
        """
        Enter method for context manager.
        """
        self.__execute_callbacks(CallbackType.BEFORE_INITIALIZE)
        self.open()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """
        Exit method for context manager.
        """
        self.finalize()

    def __dealloc__(self):
        """
        Destructor to free the solver.
        """
        self.finalize()

    def __iter__(self):
        """
        Iterator method for the solver.
        """
        return self
    
    def __next__(self):
        """
        Next method for the solver.
        """
        if self._solver_state == SolverState.FINISHED:
            raise StopIteration
        else:
            return self.step()

    @property
    def start_datetime(self) -> datetime:
        """
        Get the start date of the simulation.
        
        :return: Start date
        :rtype: datetime
        """
        cdef double start_date = swmm_getValueExpanded(
            objType=SWMMObjects.SYSTEM.value, 
            property=SWMMSystemProperties.START_DATE.value,
            index=0,
            subIndex=0
        )
        
        return decode_swmm_datetime(start_date)

    @start_datetime.setter
    def start_datetime(self, sim_start_datetime: datetime) -> None:
        """
        Initialize the solver.
        
        :param sim_start_datetime: Start date of the simulation
        :return: Error code (0 if successful)
        """
        cdef double start_date = encode_swmm_datetime(dt=sim_start_datetime)
        cdef int error_code = swmm_setValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.START_DATE.value,
            index=0,
            subindex=0,
            value=start_date
        )

        self.__validate_error(error_code)

    @property
    def end_datetime(self) -> datetime:
        """
        Get the end date of the simulation.
        
        :return: End date
        :rtype: datetime
        """
        cdef double end_date = swmm_getValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.END_DATE.value,
            index=0,
            subIndex=0
        )
        
        return decode_swmm_datetime(end_date)

    @end_datetime.setter
    def end_datetime(self, sim_end_datetime: datetime) -> None:
        """
        Set the end date of the simulation.
        
        :param sim_end_datetime: End date of the simulation
        :return: Error code (0 if successful)
        """
        cdef double end_date = encode_swmm_datetime(dt=sim_end_datetime)
        cdef int error_code = swmm_setValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.END_DATE.value,
            index=0,
            subindex=0,
            value=end_date
        )

        self.__validate_error(error_code)
    
    @property
    def routing_step(self) -> float:
        """
        Get the routing time step of the simulation in seconds.
        
        :return: Routing time step
        :rtype: double
        """
        cdef double routing_step = swmm_getValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.ROUTING_STEP.value,
            index=0,
            subIndex=0
        )
        
        return routing_step

    @routing_step.setter
    def routing_step(self, value: float) -> None:
        """
        Set the routing time step of the simulation in seconds.
        
        :param value: Routing time step in seconds
        :type value: float
        """
        cdef int error_code = swmm_setValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.ROUTING_STEP.value,
            index=0,
            subindex=0,
            value=value
        )

        self.__validate_error(error_code)

    @property
    def reporting_step(self) -> float:
        """
        Get the reporting time step of the simulation in seconds.
        
        :return: Reporting time step
        :rtype: double
        """
        cdef double reporting_step = swmm_getValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.REPORT_STEP.value,
            index=0,
            subIndex=0
        )
        
        return reporting_step

    @reporting_step.setter
    def reporting_step(self, value: float) -> None:
        """
        Set the reporting time step of the simulation in seconds.
        
        :param value: Reporting time step in seconds
        :type value: float
        """
        cdef int error_code = swmm_setValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.REPORT_STEP.value,
            index=0,
            subindex=0,
            value=value
        )
        
        self.__validate_error(error_code)
   
    @property
    def report_start_datetime(self) -> datetime:
        """
        Get the report start date of the simulation.
        
        :return: Report start date
        :rtype: datetime
        """
        cdef double report_start_date = swmm_getValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.REPORT_START_DATE.value,
            index=0,
            subIndex=0
        )

        return decode_swmm_datetime(report_start_date)

    @report_start_datetime.setter
    def report_start_datetime(self, report_start_datetime: datetime) -> None:
        """
        Set the report start date of the simulation.
        
        :param report_start_datetime: Report start date
        :type report_start_datetime: datetime
        """
        cdef double report_start_date = encode_swmm_datetime(report_start_datetime)
        cdef int error_code = swmm_setValueExpanded(
            objType=SWMMObjects.SYSTEM.value, 
            property=SWMMSystemProperties.REPORT_START_DATE.value, 
            index=0, 
            subindex=0, 
            value=report_start_date
        )

        self.__validate_error(error_code)

    @property
    def current_datetime(self) -> datetime:
        """
        Get the current date of the simulation.
        
        :return: Current date
        :rtype: datetime
        """
        cdef double current_date = swmm_getValueExpanded(
            objType=SWMMObjects.SYSTEM.value,
            property=SWMMSystemProperties.CURRENT_DATE.value,
            index=0,
            subIndex=0
        )

        return decode_swmm_datetime(current_date)
    
    @property
    def progress_callbacks_per_second(self) -> int:
        """
        Get the number of progress callbacks per second.
        
        :return: Progress callbacks per second
        :rtype: int
        """
        return self._progress_callbacks_per_second
    
    @progress_callbacks_per_second.setter
    def progress_callbacks_per_second(self, value: int) -> None:
        """
        Set the number of progress callbacks per second.
        
        :param value: Progress callbacks per second
        :type value: int
        """
        self._progress_callbacks_per_second = max(1, value)

    def get_object_count(self, object_type: SWMMObjects) -> int:
        """
        Get the count of a SWMM object type.
        
        :param object_type: SWMM object type
        :type object_type: SWMMObjects
        :return: Object count
        :rtype: int
        """
        cdef int count = swmm_getCount(objType=object_type.value)

        self.__validate_error(count)

        return count
    
    def get_object_name(self, object_type: SWMMObjects, index: int) -> str:
        """
        Get the name of a SWMM object.
        
        :param object_type: SWMM object type
        :type object_type: SWMMObjects
        :param index: Object index
        :type index: int
        :return: Object name
        :rtype: str
        """
        cdef char* c_object_name = <char*>malloc(1024*sizeof(char))

        cdef int error_code = swmm_getName(
            objType=object_type.value,
            index=index,
            name=c_object_name,
            size=1024
        )

        self.__validate_error(error_code)

        object_name = c_object_name.decode('utf-8')

        free(c_object_name)

        return object_name
    
    def get_object_names(self, object_type: SWMMObjects) -> List[str]:
        """
        Get the names of all SWMM objects of a given type.
        
        :param object_type: SWMM object type
        :type object_type: SWMMObjects
        :return: Object names
        :rtype: List[str]
        """
        cdef char* c_object_name = <char*>malloc(1024*sizeof(char))
        cdef list object_names = []
        cdef int count = self.get_object_count(object_type=object_type)

        for i in range(count):

            error_code = swmm_getName(
                objType=object_type.value,
                index=i,
                name=c_object_name,
                size=1024
            )

            self.__validate_error(error_code)

            object_name = c_object_name.decode('utf-8')
            object_names.append(object_name)

        
        free(c_object_name)

        return object_names
    
    def get_object_index(self, object_type: SWMMObjects, object_name: str) -> int:
        """
        Get the index of a SWMM object.
        
        :param object_type: SWMM object type
        :type object_type: SWMMObjects
        :param object_name: Object name
        :type object_name: str
        :return: Object index
        :rtype: int
        """
        cdef int index = swmm_getIndex(
            objType=object_type.value,
            name=object_name.encode('utf-8')
        )

        return index

    def set_value(
        self, 
        object_type: SWMMObjects, 
        property_type: Union[
            SWMMRainGageProperties, 
            SWMMSubcatchmentProperties, 
            SWMMNodeProperties, 
            SWMMLinkProperties, 
            SWMMSystemProperties
        ], 
        index: Union[int, str], 
        value: float, 
        sub_index: int = -1
        ) -> None:
        """
        Set a SWMM system property value.
        
        :param object_type: SWMM object type (e.g., SWMMObjects.NODE)
        :type object_type: SWMMObjects 
        :param property_type: SWMM system property type (e.g., SWMMSystemProperties.START_DATE)
        :type property_type: Union[SWMMRainGageProperties, SWMMSubcatchmentProperties, SWMMNodeProperties, SWMMLinkProperties, SWMMSystemProperties]
        :param index: Object index (e.g., 0, 'J1')
        :type index: int or str
        :param value: Property value (e.g., 10.0)
        :type value: double
        :param sub_index: Sub-index (e.g., 0) for properties with sub-indexes. For example pollutant index for POLLUTANT properties.
        :type sub_index: int
        """
        cdef int element_index = -1

        if isinstance(index, str):
            element_index = self.get_object_index(object_type, index)
            self.__validate_error(element_index)
        else:
            element_index = index

        cdef int error_code = swmm_setValueExpanded(
            objType=<int>object_type.value, 
            property=<int>property_type.value,
            index=element_index, 
            subindex=sub_index, 
            value=value
        )

        self.__validate_error(error_code)

    def get_value(
        self, 
        object_type: SWMMObjects, 
        property_type: Union[
            SWMMRainGageProperties, 
            SWMMSubcatchmentProperties, 
            SWMMNodeProperties, 
            SWMMLinkProperties, 
            SWMMSystemProperties
        ],
        index: Union[int, str], 
        sub_index: int = -1
        ) -> float:
        """
        Get a SWMM system property value.
        
        :param object_type: SWMM object type (e.g., SWMMObjects.NODE)
        :type object_type: SWMMObjects
        :param property_type: SWMM system property type (e.g., SWMMSystemProperties.START_DATE)
        :type property_type: Union[SWMMRainGageProperties, SWMMSubcatchmentProperties, SWMMNodeProperties, SWMMLinkProperties, SWMMSystemProperties]
        :param index: Object index (e.g., 0, 'J1')
        :type index: int or str
        :param sub_index: Sub-index (e.g., 0) for properties with sub-indexes. For example pollutant index for POLLUTANT properties.
        :type sub_index: int
        :return: Property value
        :rtype: double
        """

        cdef int element_index = -1

        if isinstance(index, str):
            element_index = self.get_object_index(object_type, index)
            self.__validate_error(element_index)
        else:
            element_index = index

        cdef double value = swmm_getValueExpanded(
            objType=<int>object_type.value, 
            property=<int>property_type.value, 
            index=element_index, 
            subIndex=sub_index
        )

        self.__validate_error(<int>value)

        return value
    
    @property
    def stride_step(self) -> int:
        """
        Get the stride step of the simulation.
        
        :return: Stride step
        :rtype: int
        """
        return self._stride_step

    @stride_step.setter
    def stride_step(self, value: int):
        """
        Set the stride time step of the simulation.
        
        :param value: Stride step in seconds
        :type value: int
        """
        self._stride_step = value

    @property
    def solver_state(self) -> SolverState:
        """
        Get the state of the solver.
        
        :return: Solver state
        :rtype: SolverState
        """
        return self._solver_state

    def add_callback(self, callback_type: CallbackType, callback: Callable[[Solver], None]) -> None:
        """
        Add a callback to the solver.
        
        :param callback_type: Type of callback
        :type callback_type: CallbackType
        :param callback: Callback function
        :type callback: callable
        """
        self._callbacks[callback_type].append(callback)

    def add_progress_callback(self, callback: Callable[[float], None]) -> None:
        """
        Add a progress callback to the solver.
        
        :param callback: Progress callback function
        :type callback: callable
        """
        self._progress_callbacks.append(callback)

    cpdef void open(self):
        """
        Opens the SWMM solver by calling the open method in the SWMM API.
        """
        cdef int error_code = 0
        cdef bytes c_inp_file_bytes = self._inp_file.encode('utf-8')
        cdef bytes c_rpt_file_bytes = self._rpt_file.encode('utf-8')
        cdef bytes c_out_file_bytes = self._out_file.encode('utf-8')

        cdef const char* c_inp_file = c_inp_file_bytes
        cdef const char* c_rpt_file = c_rpt_file_bytes
        cdef const char* c_out_file = c_out_file_bytes
        
        if self._solver_state == SolverState.OPEN:
            pass
        elif self._solver_state == SolverState.CREATED or self._solver_state == SolverState.CLOSED:
            self.__execute_callbacks(CallbackType.BEFORE_OPEN)
            
            error_code = swmm_open(
                inp_file=c_inp_file,
                rpt_file=c_rpt_file,
                out_file=c_out_file
            )
            
            self.__validate_error(error_code)
            self._solver_state = SolverState.OPEN
            self.__execute_callbacks(CallbackType.AFTER_OPEN)
            self._clock = clock()
        else:
            raise SWMMSolverException(f'Open failed: Solver is not in a valid state: {self._solver_state}')
        
        self._total_duration = swmm_getValue(
            property=SWMMSystemProperties.END_DATE.value,
            index=0
        ) - swmm_getValue(
            property=SWMMSystemProperties.START_DATE.value,
            index=0
        )

    cpdef void start(self):
        """
        Starts the SWMM solver.
        """
        cdef int error_code = 0

        if self._solver_state == SolverState.STARTED:
            pass
        elif self._solver_state == SolverState.OPEN:
            self.__execute_callbacks(CallbackType.BEFORE_START)
            error_code = swmm_start(save_flag=self._save_results)
            self.__validate_error(error_code)
            self._solver_state = SolverState.STARTED
            self.__execute_callbacks(CallbackType.AFTER_START)
        else:
            raise SWMMSolverException(f'Start failed: Solver is not in a valid state: {self._solver_state}')

    cpdef void initialize(self):
        """
        Initializes the solver and starts the simulation. Calls the open and start methods in
        the SWMM API.
        """
        self.__execute_callbacks(CallbackType.BEFORE_INITIALIZE)
        self.open()
        self.start()
    
    cpdef tuple step(self, int steps = 0):
        """
        Step a SWMM simulation.
        
        :param steps: Number of steps to run. Overrides internal stride step if greater than 0.
        :type steps: int 
        :return: elapsed_time, current_date
        :rtype: Tuple[float, datetime]
        """
        cdef double elapsed_time = 0.0
        cdef double progress = 0.0
        
        error_code = swmm_stride(strideStep=steps if steps > 0 else self._stride_step, elapsedTime=&elapsed_time)

        if error_code < 0:
            self.__validate_error(error_code)
        
        progress = (
            swmm_getValue(
                property=SWMMSystemProperties.CURRENT_DATE.value,
                index=0
            ) - self._total_duration
        ) / self._total_duration
        
        self.__execute_progress_callbacks(progress)

        if elapsed_time <= 0.0:
            self._solver_state = SolverState.FINISHED

        return elapsed_time, decode_swmm_datetime(
            swmm_datetime=swmm_getValue(
                property=SWMMSystemProperties.CURRENT_DATE.value,
                index=0
            )
        )
        # else:
            # raise SWMMSolverException(f'Step failed: Solver is not in a valid state: {self._solver_state}')

    cpdef void end(self):
        """
        Ends the SWMM simulation.
        """
        cdef int error_code = 0

        if self._solver_state == SolverState.ENDED or \
           self._solver_state == SolverState.CREATED:
            pass
        elif self._solver_state == SolverState.OPEN or \
             self._solver_state == SolverState.STARTED or \
             self._solver_state == SolverState.FINISHED:
            
            self.__execute_callbacks(CallbackType.BEFORE_END)
            error_code = swmm_end()
            self.__validate_error(error_code)
            self._solver_state = SolverState.ENDED
            self.__execute_callbacks(CallbackType.AFTER_END)
        else:
            raise SWMMSolverException(f'End failed: Solver is not in a valid state: {self._solver_state}')
    
    cpdef void report(self):
        """
        Reports the results of the SWMM simulation.
        """
        cdef int error_code = 0

        if self._solver_state == SolverState.REPORTED or self._solver_state == SolverState.CREATED:
            pass
        elif self._solver_state == SolverState.ENDED:
            self.__execute_callbacks(CallbackType.BEFORE_REPORT)
            error_code = swmm_report()
            self.__validate_error(error_code)
            self._solver_state = SolverState.REPORTED
            self.__execute_callbacks(CallbackType.AFTER_REPORT)
        else:
            raise SWMMSolverException(f'Report failed: Solver is not in a valid state: {self._solver_state}')
    
    cpdef void close(self):
        """
        Close the solver.
        """
        cdef int error_code = 0

        if self._solver_state == SolverState.CREATED:
            pass
        elif self._solver_state == SolverState.REPORTED:
            self.__execute_callbacks(CallbackType.BEFORE_CLOSE)
            error_code = swmm_close()
            self.__validate_error(error_code)
            self._solver_state = SolverState.CLOSED
            self.__execute_callbacks(CallbackType.AFTER_CLOSE)
        else:
            raise SWMMSolverException(f'Close failed: Solver is not in a valid state: {self._solver_state}')

    cpdef void finalize(self):
        """
        Finalize the solver. Ends simulation, reports the results, and dispose objects.
        """
        cdef int error_code = 0

        if self._solver_state == SolverState.OPEN or \
           self._solver_state == SolverState.STARTED or \
           self._solver_state == SolverState.FINISHED:

            self.end()
            self.report()
            self.close()
        elif self._solver_state == SolverState.ENDED:
            self.report()
            self.close()
        elif self._solver_state == SolverState.REPORTED:
            self.close()
        elif self._solver_state == SolverState.CREATED or self._solver_state == SolverState.CLOSED:
            pass
        else:
            raise SWMMSolverException(f'Finalize failed: Solver is not in a valid state: {self._solver_state}')

    cpdef void execute(self):
        """
        Run the solver to completion.
        
        :return: Error code (0 if successful)
        """
        cdef int error_code = 0
        cdef progress_callback swmm_progress_callback = <progress_callback>progress_callback_wrapper
        cdef bytes c_inp_file_bytes = self._inp_file.encode('utf-8')
        cdef bytes c_rpt_file_bytes = self._rpt_file.encode('utf-8')
        cdef bytes c_out_file_bytes = self._out_file.encode('utf-8')

        cdef const char* c_inp_file = c_inp_file_bytes
        cdef const char* c_rpt_file = c_rpt_file_bytes
        cdef const char* c_out_file = c_out_file_bytes

        if (self._solver_state != SolverState.CREATED or self._solver_state != SolverState.CLOSED):
            if len(self._progress_callbacks) > 0:
                error_code = swmm_run_with_callback(
                    inp_file=c_inp_file, 
                    rpt_file=c_rpt_file, 
                    out_file=c_out_file,
                    progress=swmm_progress_callback
                )
            else:
                error_code = swmm_run(
                    inp_file=c_inp_file,
                    rpt_file=c_rpt_file,
                    out_file=c_out_file
                )
        else:
            raise SWMMSolverException(f'Solver is not in a valid state: {self._solver_state}')

    cpdef void use_hotstart(self, str hotstart_file):
        """
        Use a hotstart file.
        
        :param hotstart_file: Hotstart file name
        """
        cdef bytes c_hotstart_file = hotstart_file.encode('utf-8')
        cdef const char* cc_hotstart_file = c_hotstart_file
        cdef int error_code = swmm_useHotStart(hotStartFile=cc_hotstart_file)

        self.__validate_error(error_code)
    
    cpdef void save_hotstart(self, str hotstart_file):
        """
        Save a hotstart file.
        
        :param hotstart_file: Hotstart file name
        """
        cdef bytes c_hotstart_file = hotstart_file.encode('utf-8')
        cdef const char* cc_hotstart_file = c_hotstart_file
        cdef int error_code = swmm_saveHotStart(hotStartFile=cc_hotstart_file)

        self.__validate_error(error_code)

    def get_mass_balance_error(self) -> Tuple[float, float, float]:
        """
        Get the mass balance error.
        
        :return: Mass balance error
        :rtype: Tuple[float, float, float]
        """
        cdef int error_code = 0
        cdef float runoffErr, flowErr, qualErr

        swmm_getMassBalErr(
            runoffErr=&runoffErr, 
            flowErr=&flowErr,
            qualErr=&qualErr
        )

        self.__validate_error(error_code)

    def __execute_callbacks(self, callback_type: CallbackType) -> None:
        """
        Execute the callbacks for the given type.
        
        :param callback_type: Type of callback
        :type callback_type: CallbackType
        """
        for callback in self._callbacks[callback_type]:
            callback(self)

    cpdef void __execute_progress_callbacks(self, double percent_complete):
        """
        Execute the progress callbacks.
        
        :param percent_complete: Percent complete
        :type percent_complete: float
        """
        for callback in self._progress_callbacks:
            callback(percent_complete)

    cdef void __progress_callback(self, double percent_complete):
        """
        Progress callback for the solver.
        
        :param percent_complete: Percent complete
        :type percent_complete: float
        """
        cdef clock_t elapsed_time =   clock() - self._clock

        if elapsed_time > 1.0 / self._progress_callbacks_per_second:
            self.__execute_progress_callbacks(
                percent_complete=percent_complete
            )

            self._clock = clock()

    cdef void __validate_error(self, error_code: int) :
        """
        Validate the error code and raise an exception if it is not 0.
        
        :param error_code: Error code to validate
        :type error_code: int
        """
        cdef int internal_error_code = <int>swmm_getValue(
            property=SWMMObjects.SYSTEM.value,
            index=SWMMSystemProperties.ERROR_CODE.value
        )

        if error_code < 0:
            if internal_error_code > 0:
                raise SWMMSolverException(f'SWMM failed with message: {internal_error_code}, {self.__get_error()}')
            else:
                raise SWMMSolverException(f'SWMM failed with message: {error_code}, {get_error_message(error_code)}')

    cdef str __get_error(self):
        """
        Get the error code from the solver.
        
        :return: Error code
        :rtype: int
        """
        cdef char* c_error_message = <char*>malloc(1024*sizeof(char))
        swmm_getError(c_error_message, 1024)

        error_message = c_error_message.decode('utf-8')

        free(c_error_message)

        return error_message


