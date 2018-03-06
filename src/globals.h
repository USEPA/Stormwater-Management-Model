//-----------------------------------------------------------------------------
//   globals.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/19/14  (Build 5.1.000)
//            04/14/14  (Build 5.1.004)
//            09/15/14  (Build 5.1.007)
//            03/19/15  (Build 5.1.008)
//            08/01/16  (Build 5.1.011)
//            03/14/17  (Build 5.1.012)
//   Author:  L. Rossman
//
//   Global Variables
//
//   Build 5.1.004:
//   - Ignore RDII option added.
//
//   Build 5.1.007:
//   - Monthly climate variable adjustments added.
//
//   Build 5.1.008:
//   - Number of parallel threads for dynamic wave routing added.
//   - Minimum dynamic wave routing variable time step added.
//
//   Build 5.1.011:
//   - Changed WarningCode to Warnings (# warnings issued)
//   - Added error message text as a variable. 
//   - Added elapsed simulation time (in decimal days) variable.
//   - Added variables associated with detailed routing events.
//
//   Build 5.1.012:
//   - InSteadyState variable made local to routing_execute in routing.c.
//-----------------------------------------------------------------------------

EXTERN TFile
                  Finp,                     // Input file
                  Fout,                     // Output file
                  Frpt,                     // Report file
                  Fclimate,                 // Climate file
                  Frain,                    // Rainfall file
                  Frunoff,                  // Runoff file
                  Frdii,                    // RDII inflow file
                  Fhotstart1,               // Hot start input file
                  Fhotstart2,               // Hot start output file
                  Finflows,                 // Inflows routing file
                  Foutflows;                // Outflows routing file

EXTERN long
                  Nperiods,                 // Number of reporting periods
                  StepCount,                // Number of routing steps used
                  NonConvergeCount;         // Number of non-converging steps

EXTERN char
                  Msg[MAXMSG+1],            // Text of output message
                  ErrorMsg[MAXMSG+1],       // Text of error message           //(5.1.011)
                  Title[MAXTITLE][MAXMSG+1],// Project title
                  TempDir[MAXFNAME+1];      // Temporary file directory

EXTERN TRptFlags
                  RptFlags;                 // Reporting options

EXTERN int
                  Nobjects[MAX_OBJ_TYPES],  // Number of each object type
                  Nnodes[MAX_NODE_TYPES],   // Number of each node sub-type
                  Nlinks[MAX_LINK_TYPES],   // Number of each link sub-type
                  UnitSystem,               // Unit system
                  FlowUnits,                // Flow units
                  InfilModel,               // Infiltration method
                  RouteModel,               // Flow routing method
                  ForceMainEqn,             // Flow equation for force mains
                  LinkOffsets,              // Link offset convention
                  AllowPonding,             // Allow water to pond at nodes
                  InertDamping,             // Degree of inertial damping
                  NormalFlowLtd,            // Normal flow limited
                  SlopeWeighting,           // Use slope weighting
                  Compatibility,            // SWMM 5/3/4 compatibility
                  SkipSteadyState,          // Skip over steady state periods
                  IgnoreRainfall,           // Ignore rainfall/runoff
                  IgnoreRDII,               // Ignore RDII                     //(5.1.004)
                  IgnoreSnowmelt,           // Ignore snowmelt
                  IgnoreGwater,             // Ignore groundwater
                  IgnoreRouting,            // Ignore flow routing
                  IgnoreQuality,            // Ignore water quality
                  ErrorCode,                // Error code number
                  Warnings,                 // Number of warning messages      //(5.1.011)
                  WetStep,                  // Runoff wet time step (sec)
                  DryStep,                  // Runoff dry time step (sec)
                  ReportStep,               // Reporting time step (sec)
                  SweepStart,               // Day of year when sweeping starts
                  SweepEnd,                 // Day of year when sweeping ends
                  MaxTrials,                // Max. trials for DW routing
                  NumThreads,               // Number of parallel threads used //(5.1.008)
                  NumEvents;                // Number of detailed events       //(5.1.011)
                //InSteadyState;            // System flows remain constant    //(5.1.012)

EXTERN double
                  RouteStep,                // Routing time step (sec)
                  MinRouteStep,             // Minimum variable time step (sec) //(5.1.008)
                  LengtheningStep,          // Time step for lengthening (sec)
                  StartDryDays,             // Antecedent dry days
                  CourantFactor,            // Courant time step factor
                  MinSurfArea,              // Minimum nodal surface area
                  MinSlope,                 // Minimum conduit slope
                  RunoffError,              // Runoff continuity error
                  GwaterError,              // Groundwater continuity error
                  FlowError,                // Flow routing error
                  QualError,                // Quality routing error
                  HeadTol,                  // DW routing head tolerance (ft)
                  SysFlowTol,               // Tolerance for steady system flow
                  LatFlowTol;               // Tolerance for steady nodal inflow       

EXTERN DateTime
                  StartDate,                // Starting date
                  StartTime,                // Starting time
                  StartDateTime,            // Starting Date+Time
                  EndDate,                  // Ending date
                  EndTime,                  // Ending time
                  EndDateTime,              // Ending Date+Time
                  ReportStartDate,          // Report start date
                  ReportStartTime,          // Report start time
                  ReportStart;              // Report start Date+Time

EXTERN double
                  ReportTime,               // Current reporting time (msec)
                  OldRunoffTime,            // Previous runoff time (msec)
                  NewRunoffTime,            // Current runoff time (msec)
                  OldRoutingTime,           // Previous routing time (msec)
                  NewRoutingTime,           // Current routing time (msec)
                  TotalDuration,            // Simulation duration (msec)
                  ElapsedTime;              // Current elapsed time (days)     //(5.1.011)

EXTERN TTemp      Temp;                     // Temperature data
EXTERN TEvap      Evap;                     // Evaporation data
EXTERN TWind      Wind;                     // Wind speed data
EXTERN TSnow      Snow;                     // Snow melt data
EXTERN TAdjust    Adjust;                   // Climate adjustments             //(5.1.007)

EXTERN TSnowmelt* Snowmelt;                 // Array of snow melt objects
EXTERN TGage*     Gage;                     // Array of rain gages
EXTERN TSubcatch* Subcatch;                 // Array of subcatchments
EXTERN TAquifer*  Aquifer;                  // Array of groundwater aquifers
EXTERN TUnitHyd*  UnitHyd;                  // Array of unit hydrographs
EXTERN TNode*     Node;                     // Array of nodes
EXTERN TOutfall*  Outfall;                  // Array of outfall nodes
EXTERN TDivider*  Divider;                  // Array of divider nodes
EXTERN TStorage*  Storage;                  // Array of storage nodes
EXTERN TLink*     Link;                     // Array of links
EXTERN TConduit*  Conduit;                  // Array of conduit links
EXTERN TPump*     Pump;                     // Array of pump links
EXTERN TOrifice*  Orifice;                  // Array of orifice links
EXTERN TWeir*     Weir;                     // Array of weir links
EXTERN TOutlet*   Outlet;                   // Array of outlet device links
EXTERN TPollut*   Pollut;                   // Array of pollutants
EXTERN TLanduse*  Landuse;                  // Array of landuses
EXTERN TPattern*  Pattern;                  // Array of time patterns
EXTERN TTable*    Curve;                    // Array of curve tables
EXTERN TTable*    Tseries;                  // Array of time series tables
EXTERN TTransect* Transect;                 // Array of transect data
EXTERN TShape*    Shape;                    // Array of custom conduit shapes
EXTERN TEvent*    Event;                    // Array of routing events         //(5.1.011)
