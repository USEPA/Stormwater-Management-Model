//-----------------------------------------------------------------------------
//   error.h
//
//   Project: EPA SWMM5
//   Version: 5.0
//   Date:    04/20/11  (Build 5.0.022)
//   Author:  L. Rossman
//
//   Error codes
//
//   NOTE: The error code enumeration was re-ordered sequentially for
//         release 5.0.022 and all previous update comments were removed.
//-----------------------------------------------------------------------------

enum  ErrorType {

  //... Runtime Errors
      ERR_NONE,                 //     0
      ERR_MEMORY,               //101  1
      ERR_KINWAVE,              //103  2
      ERR_ODE_SOLVER,           //105  3
      ERR_TIMESTEP,             //107  4

  //... Subcatchment/Aquifer Errors
      ERR_SUBCATCH_OUTLET,      //108  5
      ERR_AQUIFER_PARAMS,       //109  6
      ERR_GROUND_ELEV,          //110  7

  //... Conduit/Pump Errors
      ERR_LENGTH,               //111  8
      ERR_ELEV_DROP,            //112  9
      ERR_ROUGHNESS,            //113  10
      ERR_BARRELS,              //114  11
      ERR_SLOPE,                //115  12
      ERR_NO_XSECT,             //117  13
      ERR_XSECT,                //119  14
      ERR_NO_CURVE,             //121  15

  //... Topology Errors
      ERR_LOOP,                 //131  16
      ERR_MULTI_OUTLET,         //133  17
      ERR_MULTI_DUMMY_LINK,     //134  18

  //... Node Errors
      ERR_DIVIDER,              //135  19
      ERR_DIVIDER_LINK,         //136  20
      ERR_WEIR_DIVIDER,         //137  21
      ERR_NODE_DEPTH,           //138  22
      ERR_REGULATOR,            //139  23
      ERR_OUTFALL,              //141  24
      ERR_REGULATOR_SHAPE,      //143  25
      ERR_NO_OUTLETS,           //145  26

  //... RDII Errors
      ERR_UNITHYD_TIMES,        //151  27
      ERR_UNITHYD_RATIOS,       //153  28
      ERR_RDII_AREA,            //155  29

  //... Rain Gage Errors
      ERR_RAIN_FILE_CONFLICT,   //156  30
      ERR_RAIN_GAGE_FORMAT,     //157  31
      ERR_RAIN_GAGE_TSERIES,    //158  32
      ERR_RAIN_GAGE_INTERVAL,   //159  33

  //... Treatment Function Error
      ERR_CYCLIC_TREATMENT,     //161  34

  //... Curve/Time Series Errors
      ERR_CURVE_SEQUENCE,       //171  35
      ERR_TIMESERIES_SEQUENCE,  //173  36

  //... Snowmelt Errors
      ERR_SNOWMELT_PARAMS,      //181  37
      ERR_SNOWPACK_PARAMS,      //182  38

  //... LID Errors
      ERR_LID_TYPE,             //183  39
      ERR_LID_LAYER,            //184  40
      ERR_LID_PARAMS,           //185  41
      ERR_SUBCATCH_LID,         //186  42
      ERR_LID_AREAS,            //187  43
      ERR_LID_CAPTURE_AREA,     //188  44

  //... Simulation Date/Time Errors
      ERR_START_DATE,           //191  45
      ERR_REPORT_DATE,          //193  46
      ERR_REPORT_STEP,          //195  47

  //... Input Parser Errors
      ERR_INPUT,                //200  48
      ERR_LINE_LENGTH,          //201  49
      ERR_ITEMS,                //203  50
      ERR_KEYWORD,              //205  51
      ERR_DUP_NAME,             //207  52
      ERR_NAME,                 //209  53
      ERR_NUMBER,               //211  54
      ERR_DATETIME,             //213  55
      ERR_RULE,                 //217  56
      ERR_TRANSECT_UNKNOWN,     //219  57
      ERR_TRANSECT_SEQUENCE,    //221  58
      ERR_TRANSECT_TOO_FEW,     //223  59
      ERR_TRANSECT_TOO_MANY,    //225  60
      ERR_TRANSECT_MANNING,     //227  61
      ERR_TRANSECT_OVERBANK,    //229  62
      ERR_TRANSECT_NO_DEPTH,    //231  63
      ERR_TREATMENT_EXPR,       //233  64

  //... File Name/Opening Errors
      ERR_FILE_NAME,            //301  65
      ERR_INP_FILE,             //303  66
      ERR_RPT_FILE,             //305  67
      ERR_OUT_FILE,             //307  68
      ERR_OUT_WRITE,            //309  69
      ERR_OUT_READ,             //311  70

  //... Rain File Errors
      ERR_RAIN_FILE_SCRATCH,    //313  71
      ERR_RAIN_FILE_OPEN,       //315  72
      ERR_RAIN_FILE_DATA,       //317  73
      ERR_RAIN_FILE_SEQUENCE,   //318  74 
      ERR_RAIN_FILE_FORMAT,     //319  75
      ERR_RAIN_FILE_GAGE,       //321  76

  //... Runoff File Errors
      ERR_RUNOFF_FILE_OPEN ,    //323  77
      ERR_RUNOFF_FILE_FORMAT,   //325  78
      ERR_RUNOFF_FILE_END,      //327  79
      ERR_RUNOFF_FILE_READ,     //329  80

  //... Hotstart File Errors
      ERR_HOTSTART_FILE_NAMES,  //330  81
      ERR_HOTSTART_FILE_OPEN,   //331  82
      ERR_HOTSTART_FILE_FORMAT, //333  83
      ERR_HOTSTART_FILE_READ,   //335  84

  //... Climate File Errors
      ERR_NO_CLIMATE_FILE,      //336  85
      ERR_CLIMATE_FILE_OPEN,    //337  86
      ERR_CLIMATE_FILE_READ,    //338  87
      ERR_CLIMATE_END_OF_FILE,  //339  88

  //... RDII File Errors
      ERR_RDII_FILE_SCRATCH,    //341  89
      ERR_RDII_FILE_OPEN,       //343  90
      ERR_RDII_FILE_FORMAT,     //345  91
      
  //... Routing File Errors
      ERR_ROUTING_FILE_OPEN,    //351  92
      ERR_ROUTING_FILE_FORMAT,  //353  93
      ERR_ROUTING_FILE_NOMATCH, //355  94
      ERR_ROUTING_FILE_NAMES,   //357  95

  //... Time Series File Errors
      ERR_TABLE_FILE_OPEN,      //361  96
      ERR_TABLE_FILE_READ,      //363  97

  //... Runtime Errors
      ERR_SYSTEM,               //401  98
      ERR_NOT_CLOSED,           //402  99
      ERR_NOT_OPEN,             //403  100
      ERR_FILE_SIZE,            //405  101

      MAXERRMSG};
      
char* error_getMsg(int i);
int   error_getCode(int i);
int   error_setInpError(int errcode, char* s);
