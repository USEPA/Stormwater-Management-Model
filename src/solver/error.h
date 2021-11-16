//-----------------------------------------------------------------------------
//   error.h
//
//   Project: EPA SWMM5
//   Version: 5.2
//   Date:    11/01/21  (Build 5.2.0)
//   Author:  L. Rossman
//
//   Error codes
//
//-----------------------------------------------------------------------------

#ifndef ERROR_H
#define ERROR_H

enum  ErrorType {

// ... Runtime Errors
      ERR_NONE                 = 0,
      ERR_MEMORY               = 101,
      ERR_KINWAVE              = 103,
      ERR_ODE_SOLVER           = 105,
      ERR_TIMESTEP             = 107,

// ... Subcatchment/Aquifer Errors
      ERR_SUBCATCH_OUTLET      = 108,
      ERR_AQUIFER_PARAMS       = 109,
      ERR_GROUND_ELEV          = 110,

// ... Conduit/Pump Errors
      ERR_LENGTH               = 111,
      ERR_ELEV_DROP            = 112,
      ERR_ROUGHNESS            = 113,
      ERR_BARRELS              = 114,
      ERR_SLOPE                = 115,
      ERR_NO_XSECT             = 117,
      ERR_XSECT                = 119,
      ERR_NO_CURVE             = 121,
      ERR_PUMP_LIMITS          = 122,

// ... Topology Errors
      ERR_LOOP                 = 131,
      ERR_MULTI_OUTLET         = 133,
      ERR_DUMMY_LINK           = 134,

// ... Node Errors
      ERR_DIVIDER              = 135,
      ERR_DIVIDER_LINK         = 136,
      ERR_WEIR_DIVIDER         = 137,
      ERR_NODE_DEPTH           = 138,
      ERR_REGULATOR            = 139,
      ERR_STORAGE_VOLUME       = 140, 
      ERR_OUTFALL              = 141,
      ERR_REGULATOR_SHAPE      = 143,
      ERR_NO_OUTLETS           = 145,

// ... RDII Errors
      ERR_UNITHYD_TIMES        = 151,
      ERR_UNITHYD_RATIOS       = 153,
      ERR_RDII_AREA            = 155,

// ... Rain Gage Errors
      ERR_RAIN_FILE_CONFLICT   = 156,
      ERR_RAIN_GAGE_FORMAT     = 157,
      ERR_RAIN_GAGE_TSERIES    = 158,
      ERR_RAIN_GAGE_INTERVAL   = 159,

// ... Treatment Function Error
      ERR_CYCLIC_TREATMENT     = 161,

// ... Curve/Time Series Errors
      ERR_CURVE_SEQUENCE       = 171,
      ERR_TIMESERIES_SEQUENCE  = 173,

// ... Snowmelt Errors
      ERR_SNOWMELT_PARAMS      = 181,
      ERR_SNOWPACK_PARAMS      = 182,

// ... LID Errors
      ERR_LID_TYPE             = 183,
      ERR_LID_LAYER            = 184,
      ERR_LID_PARAMS           = 185,
      ERR_LID_AREAS            = 187,
      ERR_LID_CAPTURE_AREA     = 188,

// ... Simulation Date/Time Errors
      ERR_START_DATE           = 191,
      ERR_REPORT_DATE          = 193,
      ERR_REPORT_STEP          = 195,

// ... Input Parser Errors
      ERR_INPUT                = 200,
      ERR_LINE_LENGTH          = 201,
      ERR_ITEMS                = 203,
      ERR_KEYWORD              = 205,
      ERR_DUP_NAME             = 207,
      ERR_NAME                 = 209,
      ERR_NUMBER               = 211,
      ERR_DATETIME             = 213,
      ERR_RULE                 = 217,
      ERR_TRANSECT_UNKNOWN     = 219,
      ERR_TRANSECT_SEQUENCE    = 221,
      ERR_TRANSECT_TOO_FEW     = 223,
      ERR_TRANSECT_TOO_MANY    = 225,
      ERR_TRANSECT_MANNING     = 227,
      ERR_TRANSECT_OVERBANK    = 229,
      ERR_TRANSECT_NO_DEPTH    = 231,
      ERR_MATH_EXPR            = 233,
      ERR_INFIL_PARAMS         = 235,

// ... File Name/Opening Errors
      ERR_FILE_NAME            = 301,
      ERR_INP_FILE             = 303,
      ERR_RPT_FILE             = 305,
      ERR_OUT_FILE             = 307,
      ERR_OUT_SIZE             = 308,
      ERR_OUT_WRITE            = 309,
      ERR_OUT_READ             = 311,

// ... Rain File Errors
      ERR_RAIN_FILE_SCRATCH    = 313,
      ERR_RAIN_FILE_OPEN       = 315,
      ERR_RAIN_FILE_DATA       = 317,
      ERR_RAIN_FILE_SEQUENCE   = 318, 
      ERR_RAIN_FILE_FORMAT     = 319,
      ERR_RAIN_IFACE_FORMAT    = 320,
      ERR_RAIN_FILE_GAGE       = 321,

// ... Runoff File Errors
      ERR_RUNOFF_FILE_OPEN     = 323,
      ERR_RUNOFF_FILE_FORMAT   = 325,
      ERR_RUNOFF_FILE_END      = 327,
      ERR_RUNOFF_FILE_READ     = 329,

// ... Hotstart File Errors
      ERR_HOTSTART_FILE_OPEN   = 331,
      ERR_HOTSTART_FILE_FORMAT = 333,
      ERR_HOTSTART_FILE_READ   = 335,

// ... Climate File Errors
      ERR_NO_CLIMATE_FILE      = 336,
      ERR_CLIMATE_FILE_OPEN    = 337,
      ERR_CLIMATE_FILE_READ    = 338,
      ERR_CLIMATE_END_OF_FILE  = 339,

// ... RDII File Errors
      ERR_RDII_FILE_SCRATCH    = 341,
      ERR_RDII_FILE_OPEN       = 343,
      ERR_RDII_FILE_FORMAT     = 345,
      
// ... Routing File Errors
      ERR_ROUTING_FILE_OPEN    = 351,
      ERR_ROUTING_FILE_FORMAT  = 353,
      ERR_ROUTING_FILE_NOMATCH = 355,
      ERR_ROUTING_FILE_NAMES   = 357,

// ... Time Series File Errors
      ERR_TABLE_FILE_OPEN      = 361,
      ERR_TABLE_FILE_READ      = 363,

// ... Runtime Errors
      ERR_SYSTEM               = 500,

// ... API Errors
      ERR_API_NOT_OPEN         = 501,
      ERR_API_NOT_STARTED      = 502,
      ERR_API_NOT_ENDED        = 503,
      ERR_API_OBJECT_TYPE      = 504,
	  ERR_API_OBJECT_INDEX     = 505,
      ERR_API_OBJECT_NAME      = 506,
	  ERR_API_PROPERTY_TYPE    = 507,
      ERR_API_PROPERTY_VALUE   = 508,
      ERR_API_TIME_PERIOD      = 509,

// ... Additional Errors
      MAXERRMSG = 1000
};
      
char* error_getMsg(int i, char* msg);
int   error_setInpError(int errcode, char* s);

#endif //ERROR_H
