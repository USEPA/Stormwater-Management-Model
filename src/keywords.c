//-----------------------------------------------------------------------------
//   keywords.c
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/20/14  (Build 5.1.000)
//            04/14/14  (Build 5.1.004)
//            09/15/14  (Build 5.1.007)
//            03/19/15  (Build 5.1.008)
//            08/05/15  (Build 5.1.010)
//            08/01/16  (Build 5.1.011)
//   Author:  L. Rossman
//
//   Exportable keyword dictionary
//
//   NOTE: the keywords in each list must appear in same order used
//         by its complementary enumerated variable in enums.h and
//         must be terminated by NULL. The actual text of each keyword
//         is defined in text.h.
//
//   Build 5.1.007:
//   - Keywords for Ignore RDII option and groundwater flow equation
//     and climate adjustment input sections added.
//
//   Build 5.1.008:
//   - Keyword arrays placed in alphabetical order for better readability.
//   - Keywords added for Minimum Routing Step and Number of Threads options.
//
//   Build 5.1.010:
//   - New Modified Green Ampt keyword added to InfilModelWords.
//   - New Roadway weir keyword added to WeirTypeWords.
//
//   Build 5.1.011:
//   - New section keyword for [EVENTS] added.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>          // need this to define NULL
#include "text.h"

char* BuildupTypeWords[]   = { w_NONE, w_POW, w_EXP, w_SAT, w_EXT, NULL};
char* CurveTypeWords[]     = { w_STORAGE, w_DIVERSION, w_TIDAL, w_RATING,
                               w_CONTROLS, w_SHAPE,
                               w_PUMP1, w_PUMP2, w_PUMP3, w_PUMP4, NULL}; 
char* DividerTypeWords[]   = { w_CUTOFF, w_TABULAR, w_WEIR, w_OVERFLOW, NULL};
char* EvapTypeWords[]      = { w_CONSTANT, w_MONTHLY, w_TIMESERIES,
                               w_TEMPERATURE, w_FILE, w_RECOVERY,
                               w_DRYONLY, NULL};
char* FileTypeWords[]      = { w_RAINFALL, w_RUNOFF, w_HOTSTART, w_RDII,
                               w_INFLOWS, w_OUTFLOWS, NULL};
char* FileModeWords[]      = { w_NO, w_SCRATCH, w_USE, w_SAVE, NULL};
char* FlowUnitWords[]      = { w_CFS, w_GPM, w_MGD, w_CMS, w_LPS, w_MLD, NULL};
char* ForceMainEqnWords[]  = { w_H_W, w_D_W, NULL};
char* GageDataWords[]      = { w_TIMESERIES, w_FILE, NULL};
char* InfilModelWords[]    = { w_HORTON, w_MOD_HORTON, w_GREEN_AMPT,
                               w_MOD_GREEN_AMPT, w_CURVE_NUMEBR, NULL};        //(5.1.010)
char* InertDampingWords[]  = { w_NONE, w_PARTIAL, w_FULL, NULL};
char* LinkOffsetWords[]    = { w_DEPTH, w_ELEVATION, NULL};
char* LinkTypeWords[]      = { w_CONDUIT, w_PUMP, w_ORIFICE,
                               w_WEIR, w_OUTLET };
char* LoadUnitsWords[]     = { w_LBS, w_KG, w_LOGN };
char* NodeTypeWords[]      = { w_JUNCTION, w_OUTFALL,
                               w_STORAGE, w_DIVIDER };
char* NoneAllWords[]       = { w_NONE, w_ALL, NULL};
char* NormalFlowWords[]    = { w_SLOPE, w_FROUDE, w_BOTH, NULL};
char* NormalizerWords[]    = { w_PER_AREA, w_PER_CURB, NULL};
char* NoYesWords[]         = { w_NO, w_YES, NULL};
char* OffOnWords[]         = { w_OFF, w_ON, NULL};
char* OldRouteModelWords[] = { w_NONE, w_NF, w_KW, w_EKW, w_DW, NULL};
char* OptionWords[]        = { w_FLOW_UNITS,        w_INFIL_MODEL,
                               w_ROUTE_MODEL,       w_START_DATE,
                               w_START_TIME,        w_END_DATE,
                               w_END_TIME,          w_REPORT_START_DATE,
                               w_REPORT_START_TIME, w_SWEEP_START,
                               w_SWEEP_END,         w_START_DRY_DAYS,
                               w_WET_STEP,          w_DRY_STEP,
                               w_ROUTE_STEP,        w_REPORT_STEP,
                               w_ALLOW_PONDING,     w_INERT_DAMPING,
                               w_SLOPE_WEIGHTING,   w_VARIABLE_STEP,
                               w_NORMAL_FLOW_LTD,   w_LENGTHENING_STEP,
                               w_MIN_SURFAREA,      w_COMPATIBILITY,
                               w_SKIP_STEADY_STATE, w_TEMPDIR,
                               w_IGNORE_RAINFALL,   w_FORCE_MAIN_EQN,
                               w_LINK_OFFSETS,      w_MIN_SLOPE,
                               w_IGNORE_SNOWMELT,   w_IGNORE_GWATER,
                               w_IGNORE_ROUTING,    w_IGNORE_QUALITY,
                               w_MAX_TRIALS,        w_HEAD_TOL,
                               w_SYS_FLOW_TOL,      w_LAT_FLOW_TOL,
                               w_IGNORE_RDII,       w_MIN_ROUTE_STEP,          //(5.1.008)
                               w_NUM_THREADS,       NULL};                     //(5.1.008)
char* OrificeTypeWords[]   = { w_SIDE, w_BOTTOM, NULL};
char* OutfallTypeWords[]   = { w_FREE, w_NORMAL, w_FIXED, w_TIDAL,
                               w_TIMESERIES, NULL};
char* PatternTypeWords[]   = { w_MONTHLY, w_DAILY, w_HOURLY, w_WEEKEND, NULL};
char* PondingUnitsWords[]  = { w_PONDED_FEET, w_PONDED_METERS };
char* ProcessVarWords[]    = { w_HRT, w_DT, w_FLOW, w_DEPTH, w_AREA, NULL};
char* PumpTypeWords[]      = { w_TYPE1, w_TYPE2, w_TYPE3, w_TYPE4, w_IDEAL };
char* QualUnitsWords[]     = { w_MGperL, w_UGperL, w_COUNTperL, NULL};
char* RainTypeWords[]      = { w_INTENSITY, w_VOLUME, w_CUMULATIVE, NULL};
char* RainUnitsWords[]     = { w_INCHES, w_MMETER, NULL};
char* RelationWords[]      = { w_TABULAR, w_FUNCTIONAL, NULL};
char* ReportWords[]        = { w_INPUT, w_CONTINUITY, w_FLOWSTATS,
                               w_CONTROLS, w_SUBCATCH, w_NODE, w_LINK,
                               w_NODESTATS, NULL};
char* RouteModelWords[]    = { w_NONE, w_STEADY, w_KINWAVE, w_XKINWAVE,
                               w_DYNWAVE, NULL};
char* RuleKeyWords[]       = { w_RULE, w_IF, w_AND, w_OR, w_THEN, w_ELSE, 
                               w_PRIORITY, NULL};
char* SectWords[]          = { ws_TITLE,          ws_OPTION,
                               ws_FILE,           ws_RAINGAGE,
                               ws_TEMP,           ws_EVAP, 
                               ws_SUBCATCH,       ws_SUBAREA,
                               ws_INFIL,          ws_AQUIFER,
                               ws_GROUNDWATER,    ws_SNOWMELT,
                               ws_JUNCTION,       ws_OUTFALL,
                               ws_STORAGE,        ws_DIVIDER,
                               ws_CONDUIT,        ws_PUMP,
                               ws_ORIFICE,        ws_WEIR,
                               ws_OUTLET,         ws_XSECTION,
                               ws_TRANSECT,       ws_LOSS,
                               ws_CONTROL,        ws_POLLUTANT,
                               ws_LANDUSE,        ws_BUILDUP,
                               ws_WASHOFF,        ws_COVERAGE,
                               ws_INFLOW,         ws_DWF,
                               ws_PATTERN,        ws_RDII, 
                               ws_UNITHYD,        ws_LOADING,
                               ws_TREATMENT,      ws_CURVE,
                               ws_TIMESERIES,     ws_REPORT,
                               ws_COORDINATE,     ws_VERTICES,
                               ws_POLYGON,        ws_LABEL,
                               ws_SYMBOL,         ws_BACKDROP, 
                               ws_TAG,            ws_PROFILE,
                               ws_MAP,            ws_LID_CONTROL,
                               ws_LID_USAGE,      ws_GWF,                      //(5.1.007)
                               ws_ADJUST,         ws_EVENT,                    //(5.1.011)
                               NULL};                       
char* SnowmeltWords[]      = { w_PLOWABLE, w_IMPERV, w_PERV, w_REMOVAL, NULL};
char* TempKeyWords[]       = { w_TIMESERIES, w_FILE, w_WINDSPEED, w_SNOWMELT,
                               w_ADC, NULL};
char* TransectKeyWords[]   = { w_NC, w_X1, w_GR, NULL};
char* TreatTypeWords[]     = { w_REMOVAL, w_CONCEN, NULL};
char* UHTypeWords[]        = { w_SHORT, w_MEDIUM, w_LONG, NULL};
char* VolUnitsWords[]      = { w_MGAL, w_MLTRS };
char* VolUnitsWords2[]     = { w_GAL, w_LTR };
char* WashoffTypeWords[]   = { w_NONE, w_EXP, w_RC, w_EMC, NULL};
char* WeirTypeWords[]      = { w_TRANSVERSE, w_SIDEFLOW, w_VNOTCH,
                               w_TRAPEZOIDAL, w_ROADWAY, NULL};                //(5.1.010)
char* XsectTypeWords[]     = { w_DUMMY,           w_CIRCULAR,
                               w_FILLED_CIRCULAR, w_RECT_CLOSED,
                               w_RECT_OPEN,       w_TRAPEZOIDAL,
                               w_TRIANGULAR,      w_PARABOLIC,
                               w_POWERFUNC,       w_RECT_TRIANG,
                               w_RECT_ROUND,      w_MOD_BASKET,
                               w_HORIZELLIPSE,    w_VERTELLIPSE,
                               w_ARCH,            w_EGGSHAPED,
                               w_HORSESHOE,       w_GOTHIC,
                               w_CATENARY,        w_SEMIELLIPTICAL,
                               w_BASKETHANDLE,    w_SEMICIRCULAR,
                               w_IRREGULAR,       w_CUSTOM,
                               w_FORCE_MAIN,      NULL};
