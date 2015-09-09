//-----------------------------------------------------------------------------
//   error.h
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    03/20/14  (Build 5.1.001)
//   Author:  L. Rossman
//
//   Error codes
//
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
      ERR_PUMP_LIMITS,          //122  16

  //... Topology Errors
      ERR_LOOP,                 //131  17
      ERR_MULTI_OUTLET,         //133  18
      ERR_DUMMY_LINK,           //134  19

  //... Node Errors
      ERR_DIVIDER,              //135  20
      ERR_DIVIDER_LINK,         //136  21
      ERR_WEIR_DIVIDER,         //137  22
      ERR_NODE_DEPTH,           //138  23
      ERR_REGULATOR,            //139  24
      ERR_OUTFALL,              //141  25
      ERR_REGULATOR_SHAPE,      //143  26
      ERR_NO_OUTLETS,           //145  27

  //... RDII Errors
      ERR_UNITHYD_TIMES,        //151  28
      ERR_UNITHYD_RATIOS,       //153  29
      ERR_RDII_AREA,            //155  30

  //... Rain Gage Errors
      ERR_RAIN_FILE_CONFLICT,   //156  31
      ERR_RAIN_GAGE_FORMAT,     //157  32
      ERR_RAIN_GAGE_TSERIES,    //158  33
      ERR_RAIN_GAGE_INTERVAL,   //159  34

  //... Treatment Function Error
      ERR_CYCLIC_TREATMENT,     //161  35

  //... Curve/Time Series Errors
      ERR_CURVE_SEQUENCE,       //171  36
      ERR_TIMESERIES_SEQUENCE,  //173  37

  //... Snowmelt Errors
      ERR_SNOWMELT_PARAMS,      //181  38
      ERR_SNOWPACK_PARAMS,      //182  39

  //... LID Errors
      ERR_LID_TYPE,             //183  40
      ERR_LID_LAYER,            //184  41
      ERR_LID_PARAMS,           //185  42
      ERR_SUBCATCH_LID,         //186  43
      ERR_LID_AREAS,            //187  44
      ERR_LID_CAPTURE_AREA,     //188  45

  //... Simulation Date/Time Errors
      ERR_START_DATE,           //191  46
      ERR_REPORT_DATE,          //193  47
      ERR_REPORT_STEP,          //195  48

  //... Input Parser Errors
      ERR_INPUT,                //200  49
      ERR_LINE_LENGTH,          //201  50
      ERR_ITEMS,                //203  51
      ERR_KEYWORD,              //205  52
      ERR_DUP_NAME,             //207  53
      ERR_NAME,                 //209  54
      ERR_NUMBER,               //211  55
      ERR_DATETIME,             //213  56
      ERR_RULE,                 //217  57
      ERR_TRANSECT_UNKNOWN,     //219  58
      ERR_TRANSECT_SEQUENCE,    //221  59
      ERR_TRANSECT_TOO_FEW,     //223  60
      ERR_TRANSECT_TOO_MANY,    //225  61
      ERR_TRANSECT_MANNING,     //227  62
      ERR_TRANSECT_OVERBANK,    //229  63
      ERR_TRANSECT_NO_DEPTH,    //231  64
      ERR_TREATMENT_EXPR,       //233  65

  //... File Name/Opening Errors
      ERR_FILE_NAME,            //301  66
      ERR_INP_FILE,             //303  67
      ERR_RPT_FILE,             //305  68
      ERR_OUT_FILE,             //307  69
      ERR_OUT_WRITE,            //309  70
      ERR_OUT_READ,             //311  71

  //... Rain File Errors
      ERR_RAIN_FILE_SCRATCH,    //313  72
      ERR_RAIN_FILE_OPEN,       //315  73
      ERR_RAIN_FILE_DATA,       //317  74
      ERR_RAIN_FILE_SEQUENCE,   //318  75 
      ERR_RAIN_FILE_FORMAT,     //319  76
      ERR_RAIN_IFACE_FORMAT,    //320  77
      ERR_RAIN_FILE_GAGE,       //321  78

  //... Runoff File Errors
      ERR_RUNOFF_FILE_OPEN ,    //323  79
      ERR_RUNOFF_FILE_FORMAT,   //325  80
      ERR_RUNOFF_FILE_END,      //327  81
      ERR_RUNOFF_FILE_READ,     //329  82

  //... Hotstart File Errors
      ERR_HOTSTART_FILE_NAMES,  //330  83
      ERR_HOTSTART_FILE_OPEN,   //331  84
      ERR_HOTSTART_FILE_FORMAT, //333  85
      ERR_HOTSTART_FILE_READ,   //335  86

  //... Climate File Errors
      ERR_NO_CLIMATE_FILE,      //336  87
      ERR_CLIMATE_FILE_OPEN,    //337  88
      ERR_CLIMATE_FILE_READ,    //338  89
      ERR_CLIMATE_END_OF_FILE,  //339  90

  //... RDII File Errors
      ERR_RDII_FILE_SCRATCH,    //341  91
      ERR_RDII_FILE_OPEN,       //343  92
      ERR_RDII_FILE_FORMAT,     //345  93
      
  //... Routing File Errors
      ERR_ROUTING_FILE_OPEN,    //351  94
      ERR_ROUTING_FILE_FORMAT,  //353  95
      ERR_ROUTING_FILE_NOMATCH, //355  96
      ERR_ROUTING_FILE_NAMES,   //357  97

  //... Time Series File Errors
      ERR_TABLE_FILE_OPEN,      //361  98
      ERR_TABLE_FILE_READ,      //363  99

  //... Runtime Errors
      ERR_SYSTEM,               //401  100
      ERR_NOT_CLOSED,           //402  101
      ERR_NOT_OPEN,             //403  102
      ERR_FILE_SIZE,            //405  103

      MAXERRMSG};
      
char* error_getMsg(int i);
int   error_getCode(int i);
int   error_setInpError(int errcode, char* s);
