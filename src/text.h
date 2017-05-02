//-----------------------------------------------------------------------------
//   text.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/19/14  (Build 5.1.001)
//            04/02/14  (Build 5.1.003)
//            04/14/14  (Build 5.1.004)
//            04/23/14  (Build 5.1.005)
//            05/19/14  (Build 5.1.006)
//            09/15/14  (Build 5.1.007)
//            03/19/15  (Build 5.1.008)
//            04/30/15  (Build 5.1.009)
//            08/05/15  (Build 5.1.010)
//            08/01/16  (Build 5.1.011)
//            03/14/17  (Build 5.1.012)
//   Author:  L. Rossman
//
//   Text strings
//-----------------------------------------------------------------------------

#define FMT01 \
 "\n Correct syntax is:\n swmm5 <input file> <report file> <output file>\n"
#define FMT02 "\n... EPA-SWMM 5.1 (Build 5.1.012)\n"                           //(5.1.012)

#define FMT03 " There are errors.\n"
#define FMT04 " There are warnings.\n"
#define FMT05 "\n"
#define FMT06 "\n o  Retrieving project data"
#define FMT07 "\n o  Writing output report"
#define FMT08 \
  "\n  EPA STORM WATER MANAGEMENT MODEL - VERSION 5.1 (Build 5.1.012)"         //(5.1.012)
#define FMT09 \
  "\n  --------------------------------------------------------------"
#define FMT10 "\n"
#define FMT11  "\n    Cannot use duplicate file names."
#define FMT12  "\n    Cannot open input file "
#define FMT13  "\n    Cannot open report file "
#define FMT14  "\n    Cannot open output file "
#define FMT15  "\n    Cannot open temporary output file"
#define FMT16  "\n  ERROR %d detected. Execution halted."
#define FMT17  "at line %d of input file:"
#define FMT18  "at line %d of %s] section:"
#define FMT19  "\n  Maximum error count exceeded."
#define FMT20  "\n\n  Analysis begun on:  %s"
#define FMT20a "  Analysis ended on:  %s"
#define FMT21  "  Total elapsed time: "

// Warning messages
#define WARN01 "WARNING 01: wet weather time step reduced to recording interval for Rain Gage" 
#define WARN02 "WARNING 02: maximum depth increased for Node"
#define WARN03 "WARNING 03: negative offset ignored for Link"
#define WARN04 "WARNING 04: minimum elevation drop used for Conduit"
#define WARN05 "WARNING 05: minimum slope used for Conduit"
#define WARN06 "WARNING 06: dry weather time step increased to the wet weather time step"
#define WARN07 "WARNING 07: routing time step reduced to the wet weather time step"
#define WARN08 "WARNING 08: elevation drop exceeds length for Conduit"
#define WARN09 "WARNING 09: time series interval greater than recording interval for Rain Gage"
//#define WARN10 "WARNING 10: crest elevation is below downstream invert for regulator Link"
#define WARN10 \
"WARNING 10: crest elevation raised to downstream invert for regulator Link"   //(5.1.011)
#define WARN11 "WARNING 11: non-matching attributes in Control Rule"           //(5.1.009)

// Analysis Option Keywords
#define  w_FLOW_UNITS        "FLOW_UNITS"
#define  w_INFIL_MODEL       "INFILTRATION"
#define  w_ROUTE_MODEL       "FLOW_ROUTING"
#define  w_START_DATE        "START_DATE"
#define  w_START_TIME        "START_TIME"
#define  w_END_DATE          "END_DATE"
#define  w_END_TIME          "END_TIME"
#define  w_REPORT_START_DATE "REPORT_START_DATE"
#define  w_REPORT_START_TIME "REPORT_START_TIME"
#define  w_SWEEP_START       "SWEEP_START"
#define  w_SWEEP_END         "SWEEP_END"
#define  w_START_DRY_DAYS    "DRY_DAYS"
#define  w_WET_STEP          "WET_STEP"
#define  w_DRY_STEP          "DRY_STEP"
#define  w_ROUTE_STEP        "ROUTING_STEP"
#define  w_REPORT_STEP       "REPORT_STEP"
#define  w_ALLOW_PONDING     "ALLOW_PONDING"
#define  w_INERT_DAMPING     "INERTIAL_DAMPING"
#define  w_SLOPE_WEIGHTING   "SLOPE_WEIGHTING"
#define  w_VARIABLE_STEP     "VARIABLE_STEP"
#define  w_NORMAL_FLOW_LTD   "NORMAL_FLOW_LIMITED"
#define  w_LENGTHENING_STEP  "LENGTHENING_STEP"
#define  w_MIN_SURFAREA      "MIN_SURFAREA"
#define  w_COMPATIBILITY     "COMPATIBILITY"
#define  w_SKIP_STEADY_STATE "SKIP_STEADY_STATE"
#define  w_TEMPDIR           "TEMPDIR"
#define  w_IGNORE_RAINFALL   "IGNORE_RAINFALL"
#define  w_FORCE_MAIN_EQN    "FORCE_MAIN_EQUATION"
#define  w_LINK_OFFSETS      "LINK_OFFSETS"
#define  w_MIN_SLOPE         "MIN_SLOPE"
#define  w_IGNORE_SNOWMELT   "IGNORE_SNOWMELT"
#define  w_IGNORE_GWATER     "IGNORE_GROUNDWATER"
#define  w_IGNORE_ROUTING    "IGNORE_ROUTING"
#define  w_IGNORE_QUALITY    "IGNORE_QUALITY"
#define  w_MAX_TRIALS        "MAX_TRIALS"
#define  w_HEAD_TOL          "HEAD_TOLERANCE"
#define  w_SYS_FLOW_TOL      "SYS_FLOW_TOL"
#define  w_LAT_FLOW_TOL      "LAT_FLOW_TOL"
#define  w_IGNORE_RDII       "IGNORE_RDII"                                     //(5.1.004)
#define  w_MIN_ROUTE_STEP    "MINIMUM_STEP"                                    //(5.1.008)
#define  w_NUM_THREADS       "THREADS"                                         //(5.1.008)

// Flow Units
#define  w_CFS               "CFS"
#define  w_GPM               "GPM"
#define  w_MGD               "MGD"
#define  w_CMS               "CMS"
#define  w_LPS               "LPS"
#define  w_MLD               "MLD"

// Flow Routing Methods
#define  w_NF                "NF"
#define  w_KW                "KW"
#define  w_EKW               "EKW"
#define  w_DW                "DW"

#define  w_STEADY            "STEADY"
#define  w_KINWAVE           "KINWAVE"
#define  w_XKINWAVE          "XKINWAVE"
#define  w_DYNWAVE           "DYNWAVE"

// Infiltration Methods
#define  w_HORTON            "HORTON"
#define  w_MOD_HORTON        "MODIFIED_HORTON"
#define  w_GREEN_AMPT        "GREEN_AMPT"
#define  w_MOD_GREEN_AMPT    "MODIFIED_GREEN_AMPT"                             //(5.1.010)
#define  w_CURVE_NUMEBR      "CURVE_NUMBER"

// Normal Flow Criteria
#define  w_SLOPE             "SLOPE"
#define  w_FROUDE            "FROUDE"
#define  w_BOTH              "BOTH"

// Snowmelt Data Keywords
#define  w_WINDSPEED         "WINDSPEED"
#define  w_SNOWMELT          "SNOWMELT"
#define  w_ADC               "ADC"
#define  w_PLOWABLE          "PLOWABLE"

// Evaporation Data Options
#define  w_CONSTANT          "CONSTANT"
#define  w_TIMESERIES        "TIMESERIES"
#define  w_TEMPERATURE       "TEMPERATURE"
#define  w_FILE              "FILE"
#define  w_RECOVERY          "RECOVERY"
#define  w_DRYONLY           "DRY_ONLY"

// DWF Time Pattern Types
#define  w_MONTHLY           "MONTHLY"
#define  w_DAILY             "DAILY"
#define  w_HOURLY            "HOURLY"
#define  w_WEEKEND           "WEEKEND"

// Rainfall Record Types
#define  w_INTENSITY         "INTENSITY"
#define  w_VOLUME            "VOLUME"
#define  w_CUMULATIVE        "CUMULATIVE"

// Unit Hydrograph Types
#define  w_SHORT             "SHORT"
#define  w_MEDIUM            "MEDIUM"
#define  w_LONG              "LONG"

// Internal Runoff Routing Options
#define  w_OUTLET            "OUTLET"
#define  w_IMPERV            "IMPERV"
#define  w_PERV              "PERV"

// Outfall Node Types
#define  w_FREE              "FREE"
#define  w_FIXED             "FIXED"
#define  w_TIDAL             "TIDAL"
#define  w_CRITICAL          "CRITICAL"
#define  w_NORMAL            "NORMAL"

// Flow Divider Node Types
#define  w_FUNCTIONAL        "FUNCTIONAL"
#define  w_TABULAR           "TABULAR"
#define  w_CUTOFF            "CUTOFF"
#define  w_OVERFLOW          "OVERFLOW"

// Pump Curve Types
#define  w_TYPE1             "TYPE1"
#define  w_TYPE2             "TYPE2"
#define  w_TYPE3             "TYPE3"
#define  w_TYPE4             "TYPE4"
#define  w_IDEAL             "IDEAL"

// Pump Curve Variables
#define  w_VOLUME            "VOLUME"
#define  w_DEPTH             "DEPTH"
#define  w_HEAD              "HEAD"

// Orifice Types
#define  w_SIDE              "SIDE"
#define  w_BOTTOM            "BOTTOM"

// Weir Types
#define  w_TRANSVERSE        "TRANSVERSE"
#define  w_SIDEFLOW          "SIDEFLOW"
#define  w_VNOTCH            "V-NOTCH"
#define  w_ROADWAY           "ROADWAY"                                         //(5.1.010)

// Conduit Cross-Section Shapes
#define  w_DUMMY             "DUMMY"
#define  w_CIRCULAR          "CIRCULAR"
#define  w_FILLED_CIRCULAR   "FILLED_CIRCULAR"
#define  w_RECT_CLOSED       "RECT_CLOSED"
#define  w_RECT_OPEN         "RECT_OPEN"
#define  w_TRAPEZOIDAL       "TRAPEZOIDAL"
#define  w_TRIANGULAR        "TRIANGULAR"
#define  w_PARABOLIC         "PARABOLIC"
#define  w_POWERFUNC         "POWER"
#define  w_RECT_TRIANG       "RECT_TRIANGULAR"
#define  w_RECT_ROUND        "RECT_ROUND"
#define  w_MOD_BASKET        "MODBASKETHANDLE"
#define  w_HORIZELLIPSE      "HORIZ_ELLIPSE"
#define  w_VERTELLIPSE       "VERT_ELLIPSE"
#define  w_ARCH              "ARCH"
#define  w_EGGSHAPED         "EGG"
#define  w_HORSESHOE         "HORSESHOE"
#define  w_GOTHIC            "GOTHIC"
#define  w_CATENARY          "CATENARY"
#define  w_SEMIELLIPTICAL    "SEMIELLIPTICAL"
#define  w_BASKETHANDLE      "BASKETHANDLE"
#define  w_SEMICIRCULAR      "SEMICIRCULAR"
#define  w_IRREGULAR         "IRREGULAR"
#define  w_CUSTOM            "CUSTOM"
#define  w_FORCE_MAIN        "FORCE_MAIN"
#define  w_H_W               "H-W"
#define  w_D_W               "D-W" 

// Link Offset Options
#define  w_ELEVATION         "ELEVATION"

// Transect Data Input Codes
#define  w_NC                "NC"
#define  w_X1                "X1"
#define  w_GR                "GR"

// Rain Volume Units
#define  w_INCHES            "IN"
#define  w_MMETER            "MM"

// Flow Volume Units
#define  w_MGAL              "10^6 gal"
#define  w_MLTRS             "10^6 ltr"
#define  w_GAL               "gal" 
#define  w_LTR               "ltr"

// Ponded Depth Units
#define  w_PONDED_FEET       "Feet"
#define  w_PONDED_METERS     "Meters"

// Concentration Units
#define  w_MGperL            "MG/L"
#define  w_UGperL            "UG/L"
#define  w_COUNTperL         "#/L"

// Mass Units
#define  w_MG                "MG"
#define  w_UG                "UG"
#define  w_COUNT             "#"

// Load Units
#define  w_LBS               "lbs"
#define  w_KG                "kg"
#define  w_LOGN              "LogN"

// Pollutant Buildup Functions
#define  w_POW               "POW"
#define  w_EXP               "EXP"
#define  w_SAT               "SAT"
#define  w_EXT               "EXT"

// Normalizing Variables for Pollutant Buildup
#define  w_PER_AREA          "AREA"
#define  w_PER_CURB          "CURB"

// Pollutant Washoff Functions
// (EXP function defined above)
#define  w_RC                "RC"
#define  w_EMC               "EMC"

// Treatment Keywords
#define  w_REMOVAL           "REMOVAL"
#define  w_RATE              "RATE"
#define  w_HRT               "HRT"
#define  w_DT                "DT"
#define  w_AREA              "AREA"

// Curve Types
//define  w_STORAGE (defined below)
#define  w_DIVERSION         "DIVERSION"
#define  w_TIDAL             "TIDAL"
#define  w_RATING            "RATING"
#define  w_SHAPE             "SHAPE"
#define  w_PUMP1             "PUMP1"
#define  w_PUMP2             "PUMP2"
#define  w_PUMP3             "PUMP3"
#define  w_PUMP4             "PUMP4"

// Reporting Options
#define  w_INPUT             "INPUT"
#define  w_CONTINUITY        "CONTINUITY"
#define  w_FLOWSTATS         "FLOWSTATS"
#define  w_CONTROLS          "CONTROL"
#define  w_NODESTATS         "NODESTATS"

// Interface File Types
#define  w_RAINFALL          "RAINFALL"
#define  w_RUNOFF            "RUNOFF"
#define  w_HOTSTART          "HOTSTART"
#define  w_RDII              "RDII"
#define  w_ROUTING           "ROUTING"
#define  w_INFLOWS           "INFLOWS"
#define  w_OUTFLOWS          "OUTFLOWS"

// Miscellaneous Keywords
#define  w_OFF               "OFF"
#define  w_ON                "ON"
#define  w_NO                "NO"
#define  w_YES               "YES"
#define  w_NONE              "NONE"
#define  w_ALL               "ALL"
#define  w_SCRATCH           "SCRATCH"
#define  w_USE               "USE"
#define  w_SAVE              "SAVE"
#define  w_FULL              "FULL"
#define  w_PARTIAL           "PARTIAL"

// Major Object Types
#define  w_GAGE              "RAINGAGE"
#define  w_SUBCATCH          "SUBCATCH"
#define  w_NODE              "NODE"
#define  w_LINK              "LINK"
#define  w_POLLUT            "POLLUTANT"
#define  w_LANDUSE           "LANDUSE"
#define  w_TSERIES           "TIME SERIES"
#define  w_TABLE             "TABLE"
#define  w_UNITHYD           "HYDROGRAPH"

// Node Sub-Types
#define  w_JUNCTION          "JUNCTION"
#define  w_OUTFALL           "OUTFALL"
#define  w_STORAGE           "STORAGE"
#define  w_DIVIDER           "DIVIDER"

// Link Sub-Types
#define  w_CONDUIT           "CONDUIT"
#define  w_PUMP              "PUMP"
#define  w_ORIFICE           "ORIFICE"
#define  w_WEIR              "WEIR"

// Control Rule Keywords
#define  w_RULE              "RULE"
#define  w_IF                "IF"
#define  w_AND               "AND"
#define  w_OR                "OR"
#define  w_THEN              "THEN"
#define  w_ELSE              "ELSE"
#define  w_PRIORITY          "PRIORITY"

// External Inflow Types
#define  w_FLOW              "FLOW"
#define  w_CONCEN            "CONCEN"
#define  w_MASS              "MASS"

// Variable Units
#define  w_FEET              "FEET"
#define  w_METERS            "METERS"
#define  w_FPS               "FT/SEC"
#define  w_MPS               "M/SEC"
#define  w_PCNT              "PERCENT"
#define  w_ACRE              "acre"
#define  w_HECTARE           "hectare"

// Input File Sections
#define  ws_TITLE            "[TITLE"
#define  ws_OPTION           "[OPTION"
#define  ws_FILE             "[FILE"
#define  ws_RAINGAGE         "[RAINGAGE"
#define  ws_TEMP             "[TEMPERATURE"
#define  ws_EVAP             "[EVAP"
#define  ws_SUBCATCH         "[SUBCATCHMENT"
#define  ws_SUBAREA          "[SUBAREA"
#define  ws_INFIL            "[INFIL"
#define  ws_AQUIFER          "[AQUIFER"
#define  ws_GROUNDWATER      "[GROUNDWATER"
#define  ws_SNOWMELT         "[SNOWPACK"
#define  ws_JUNCTION         "[JUNC"
#define  ws_OUTFALL          "[OUTFALL"
#define  ws_STORAGE          "[STORAGE"
#define  ws_DIVIDER          "[DIVIDER"
#define  ws_CONDUIT          "[CONDUIT"
#define  ws_PUMP             "[PUMP"
#define  ws_ORIFICE          "[ORIFICE"
#define  ws_WEIR             "[WEIR"
#define  ws_OUTLET           "[OUTLET"
#define  ws_XSECTION         "[XSECT"
#define  ws_TRANSECT         "[TRANSECT"
#define  ws_LOSS             "[LOSS"
#define  ws_CONTROL          "[CONTROL"
#define  ws_POLLUTANT        "[POLLUT"
#define  ws_LANDUSE          "[LANDUSE"
#define  ws_BUILDUP          "[BUILDUP"
#define  ws_WASHOFF          "[WASHOFF"
#define  ws_COVERAGE         "[COVERAGE"
#define  ws_INFLOW           "[INFLOW"
#define  ws_DWF              "[DWF"
#define  ws_PATTERN          "[PATTERN"
#define  ws_RDII             "[RDII"
#define  ws_UNITHYD          "[HYDROGRAPH"
#define  ws_LOADING          "[LOADING"
#define  ws_TREATMENT        "[TREATMENT"
#define  ws_CURVE            "[CURVE"
#define  ws_TIMESERIES       "[TIMESERIES"
#define  ws_REPORT           "[REPORT"
#define  ws_MAP              "[MAP"
#define  ws_COORDINATE       "[COORDINATE"
#define  ws_VERTICES         "[VERTICES"
#define  ws_POLYGON          "[POLYGON"
#define  ws_SYMBOL           "[SYMBOL"
#define  ws_LABEL            "[LABEL"
#define  ws_BACKDROP         "[BACKDROP"
#define  ws_TAG              "[TAG"
#define  ws_PROFILE          "[PROFILE"
#define  ws_LID_CONTROL      "[LID_CONTROL"
#define  ws_LID_USAGE        "[LID_USAGE"
#define  ws_GW_FLOW          "[GW_FLOW"     //Deprecated                       //(5.1.007)
#define  ws_GWF              "[GWF"                                            //(5.1.007)
#define  ws_ADJUST           "[ADJUSTMENT"                                     //(5.1.007)
#define  ws_EVENT            "[EVENT"                                          //(5.1.011)
